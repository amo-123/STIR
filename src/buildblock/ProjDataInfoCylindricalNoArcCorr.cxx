//
// $Id$
//
/*!

  \file
  \ingroup projdata

  \brief Implementation of non-inline functions of class 
  ProjDataInfoCylindricalNoArcCorr

  \author Kris Thielemans

  $Date$
  $Revision$
*/
/*
    Copyright (C) 2000- $Date$, IRSL
    See STIR/LICENSE.txt for details
*/

#include "stir/ProjDataInfoCylindricalNoArcCorr.h"
#include "stir/Bin.h"
#include "stir/CartesianCoordinate3D.h"
#include "stir/round.h"

#ifdef BOOST_NO_STRINGSTREAM
#include <strstream.h>
#else
#include <sstream>
#endif

#ifndef STIR_NO_NAMESPACES
using std::endl;
using std::ends;
#endif

START_NAMESPACE_STIR
ProjDataInfoCylindricalNoArcCorr:: 
ProjDataInfoCylindricalNoArcCorr()
{}

ProjDataInfoCylindricalNoArcCorr:: 
ProjDataInfoCylindricalNoArcCorr(const shared_ptr<Scanner> scanner_ptr,
                                 const float ring_radius_v, const float angular_increment_v,
				 const  VectorWithOffset<int>& num_axial_pos_per_segment,
                                 const  VectorWithOffset<int>& min_ring_diff_v, 
                                 const  VectorWithOffset<int>& max_ring_diff_v,
                                 const int num_views,const int num_tangential_poss)
: ProjDataInfoCylindrical(scanner_ptr,
                          num_axial_pos_per_segment,
                          min_ring_diff_v, max_ring_diff_v,
                          num_views, num_tangential_poss),
  ring_radius(ring_radius_v),
  angular_increment(angular_increment_v)
{
  view_tangpos_to_det1det2_initialised = false;
  det1det2_to_view_tangpos_initialised = false;
}

ProjDataInfoCylindricalNoArcCorr:: 
ProjDataInfoCylindricalNoArcCorr(const shared_ptr<Scanner> scanner_ptr,
                                 const  VectorWithOffset<int>& num_axial_pos_per_segment,
                                 const  VectorWithOffset<int>& min_ring_diff_v, 
                                 const  VectorWithOffset<int>& max_ring_diff_v,
                                 const int num_views,const int num_tangential_poss)
: ProjDataInfoCylindrical(scanner_ptr,
                          num_axial_pos_per_segment,
                          min_ring_diff_v, max_ring_diff_v,
                          num_views, num_tangential_poss)
{
  assert(scanner_ptr.use_count()!=0);
  ring_radius = scanner_ptr->get_ring_radius();
  angular_increment = _PI/scanner_ptr->get_num_detectors_per_ring();
  view_tangpos_to_det1det2_initialised = false;
  det1det2_to_view_tangpos_initialised = false;
}




ProjDataInfo*
ProjDataInfoCylindricalNoArcCorr::clone() const
{
  return static_cast<ProjDataInfo*>(new ProjDataInfoCylindricalNoArcCorr(*this));
}

string
ProjDataInfoCylindricalNoArcCorr::parameter_info()  const
{

#ifdef BOOST_NO_STRINGSTREAM
  // dangerous for out-of-range, but 'old-style' ostrstream seems to need this
  char str[50000];
  ostrstream s(str, 50000);
#else
  std::ostringstream s;
#endif  
  s << "ProjDataInfoCylindricalNoArcCorr := \n";
  s << ProjDataInfoCylindrical::parameter_info();
  s << "End :=\n";
  return s.str();
}

/*
   TODO make compile time assert

   Warning:
   this code makes use of an implementation dependent feature:
   bit shifting negative ints to the right.
    -1 >> 1 should be -1
    -2 >> 1 should be -1
   This is ok on SUNs (gcc, but probably SUNs cc as well), Parsytec (gcc),
   Pentium (gcc, VC++) and probably every other system which uses
   the 2-complement convention.
*/

/*
  Go from sinograms to detectors.
  Because sinograms are not arc-corrected, tang_pos_num corresponds
  to an angle as well. Before interleaving we have that
  \verbatim
  det_angle_1 = LOR_angle + bin_angle
  det_angle_2 = LOR_angle + (Pi - bin_angle)
  \endverbatim
  (Hint: understand this first at LOR_angle=0, then realise that
  other LOR_angles follow just by rotation)

  Code gets slightly intricate because:
  - angles have to be defined modulo 2 Pi (so num_detectors)
  - interleaving
*/
void 
ProjDataInfoCylindricalNoArcCorr::
initialise_view_tangpos_to_det1det2() const
{
  const int num_detectors =
    get_scanner_ptr()->get_num_detectors_per_ring();

  assert(num_detectors%2 == 0);
  assert(get_min_view_num() == 0);
  assert(get_max_view_num() == num_detectors/2 - 1);
  // check views range from 0 to Pi
  assert(fabs(get_phi(Bin(0,0,0,0))) < 1.E-4);
  assert(fabs(get_phi(Bin(0,num_detectors/2,0,0)) - _PI) < 1.E-4);
  const int min_tang_pos_num = -(num_detectors/2)+1;
  const int max_tang_pos_num = -(num_detectors/2)+num_detectors;
  
  view_tangpos_to_det1det2.grow(0,num_detectors/2-1);
  for (int v_num=0; v_num<=num_detectors/2-1; ++v_num)
  {
    view_tangpos_to_det1det2[v_num].grow(min_tang_pos_num, max_tang_pos_num);

    for (int tp_num=min_tang_pos_num; tp_num<=max_tang_pos_num; ++tp_num)
    {
      /*
         adapted from CTI code
         Note for implementation: avoid using % with negative numbers
         so add num_detectors before doing modulo num_detectors)
        */
      view_tangpos_to_det1det2[v_num][tp_num].det1_num = 
        (v_num + (tp_num >> 1) + num_detectors) % num_detectors;
      view_tangpos_to_det1det2[v_num][tp_num].det2_num = 
        (v_num - ( (tp_num + 1) >> 1 ) + num_detectors/2) % num_detectors;
    }
  }
  view_tangpos_to_det1det2_initialised = true;
}

void 
ProjDataInfoCylindricalNoArcCorr::
initialise_det1det2_to_view_tangpos() const
{
  const int num_detectors =
    get_scanner_ptr()->get_num_detectors_per_ring();

  assert(num_detectors%2 == 0);
  assert(get_min_view_num() == 0);
  // check views range from 0 to Pi
  assert(fabs(get_phi(Bin(0,0,0,0))) < 1.E-4);
  assert(fabs(get_phi(Bin(0,get_max_view_num()+1,0,0)) - _PI) < 1.E-4);
  //const int min_tang_pos_num = -(num_detectors/2);
  //const int max_tang_pos_num = -(num_detectors/2)+num_detectors;
  const int max_num_views = num_detectors/2;

  det1det2_to_view_tangpos.grow(0,num_detectors-1);
  for (int det1_num=0; det1_num<num_detectors; ++det1_num)
  {
    det1det2_to_view_tangpos[det1_num].grow(0, num_detectors-1);

    for (int det2_num=0; det2_num<num_detectors; ++det2_num)
    {            
      if (det1_num == det2_num)
	  continue;
      /*
       This somewhat obscure formula was obtained by inverting the code for
       get_det_num_pair_for_view_tangential_pos_num()
       This can be simplified (especially all the branching later on), but
       as we execute this code only occasionally, it's probably not worth it.
      */
      int swap_detectors;
      /*
      Note for implementation: avoid using % with negative numbers
      so add num_detectors before doing modulo num_detectors
      */
      int tang_pos_num = (det1_num - det2_num +  3*num_detectors/2) % num_detectors;
      int view_num = (det1_num - (tang_pos_num >> 1) +  num_detectors) % num_detectors;
      
      /* Now adjust ranges for view_num, tang_pos_num.
      The next lines go only wrong in the singular (and irrelevant) case
      det_num1 == det_num2 (when tang_pos_num == num_detectors - tang_pos_num)
      
        We use the combinations of the following 'symmetries' of
        (tang_pos_num, view_num) == (tang_pos_num+2*num_views, view_num + num_views)
        == (-tang_pos_num, view_num + num_views)
        Using the latter interchanges det_num1 and det_num2, and this leaves
        the LOR the same in the 2D case. However, in 3D this interchanges the rings
        as well. So, we keep track of this in swap_detectors, and return its final
        value.
      */
      if (view_num <  max_num_views)
      {
        if (tang_pos_num >=  max_num_views)
        {
          tang_pos_num = num_detectors - tang_pos_num;
          swap_detectors = 1;
        }
        else
        {
          swap_detectors = 0;
        }
      }
      else
      {
        view_num -= max_num_views;
        if (tang_pos_num >=  max_num_views)
        {
          tang_pos_num -= num_detectors;
          swap_detectors = 0;
        }
        else
        {
          tang_pos_num *= -1;
          swap_detectors = 1;
        }
      }
      
      det1det2_to_view_tangpos[det1_num][det2_num].view_num = view_num;
      det1det2_to_view_tangpos[det1_num][det2_num].tang_pos_num = tang_pos_num;
      det1det2_to_view_tangpos[det1_num][det2_num].swap_detectors = swap_detectors==0;     
    }
  }
  det1det2_to_view_tangpos_initialised = true;
}

Succeeded
ProjDataInfoCylindricalNoArcCorr::
find_scanner_coordinates_given_cartesian_coordinates(int& det1, int& det2, int& ring1, int& ring2,
					             const CartesianCoordinate3D<float>& c1,
						     const CartesianCoordinate3D<float>& c2) const
{
  const int num_detectors=get_scanner_ptr()->get_num_detectors_per_ring();
  const float ring_spacing=get_scanner_ptr()->get_ring_spacing();
  const float ring_radius=get_scanner_ptr()->get_ring_radius();

  const CartesianCoordinate3D<float> d = c2 - c1;
  /* parametrisation of LOR is 
     c = l*d+c1
     l has to be such that c.x^2 + c.y^2 = R^2
     i.e.
     (l*d.x+c1.x)^2+(l*d.y+c1.y)^2==R^2
     l^2*(d.x^2+d.y^2) + 2*l*(d.x*c1.x + d.y*c1.y) + c1.x^2+c2.y^2-R^2==0
     write as a*l^2+2*b*l+e==0
     l = (-b +- sqrt(b^2-a*e))/a
     argument of sqrt simplifies to
     R^2*(d.x^2+d.y^2)-(d.x*c1.y-d.y*c1.x)^2
  */
  const float dxy2 = (square(d.x())+square(d.y()));
  const float argsqrt=
    (square(ring_radius)*dxy2-square(d.x()*c1.y()-d.y()*c1.x()));
  if (argsqrt<=0)
    return Succeeded::no; // LOR is outside detector radius
  const float root = sqrt(argsqrt);

  const float l1 = (- (d.x()*c1.x() + d.y()*c1.y())+root)/dxy2;
  const float l2 = (- (d.x()*c1.x() + d.y()*c1.y())-root)/dxy2;
  const CartesianCoordinate3D<float> coord_det1 = d*l1 + c1;
  const CartesianCoordinate3D<float> coord_det2 = d*l2 + c1;
  assert(fabs(square(coord_det1.x())+square(coord_det1.y())-square(ring_radius))<square(ring_radius)*10.E-5);
  assert(fabs(square(coord_det2.x())+square(coord_det2.y())-square(ring_radius))<square(ring_radius)*10.E-5);

  det1 = stir::round(((2.*_PI)+atan2(-coord_det1.x(),coord_det1.y()))/(2.*_PI/num_detectors))% num_detectors;
  det2 = stir::round(((2.*_PI)+atan2(-coord_det2.x(),coord_det2.y()))/(2.*_PI/num_detectors))% num_detectors;
  ring1 = round(coord_det1.z()/ring_spacing);
  ring2 = round(coord_det2.z()/ring_spacing);


#ifndef NDEBUG
  {

    CartesianCoordinate3D<float> check1, check2;
    find_cartesian_coordinates_given_scanner_coordinates (check1, check2,
							  ring1,ring2, 
							  det1, det2);
    assert(norm(coord_det1-check1)<ring_spacing);
    assert(norm(coord_det2-check2)<ring_spacing);
  }
#endif
  assert(det1 >=0 && det1<get_scanner_ptr()->get_num_detectors_per_ring());
  assert(det2 >=0 && det2<get_scanner_ptr()->get_num_detectors_per_ring());

  return 
    (ring1 >=0 && ring1<get_scanner_ptr()->get_num_rings() &&
     ring2 >=0 && ring2<get_scanner_ptr()->get_num_rings()) 
     ? Succeeded::yes : Succeeded::no;
}


void 
ProjDataInfoCylindricalNoArcCorr::
find_cartesian_coordinates_of_detection(
					CartesianCoordinate3D<float>& coord_1,
					CartesianCoordinate3D<float>& coord_2,
					const Bin& bin) const
{
 // find detectors
  int det_num_a;
  int det_num_b;
  int ring_a;
  int ring_b;
  get_det_pair_for_bin(det_num_a, ring_a,
                       det_num_b, ring_b, bin);
  
  // find corresponding cartesian coordinates
  find_cartesian_coordinates_given_scanner_coordinates(coord_1,coord_2,
    ring_a,ring_b,det_num_a,det_num_b);
}


void
ProjDataInfoCylindricalNoArcCorr::
find_cartesian_coordinates_given_scanner_coordinates (CartesianCoordinate3D<float>& coord_1,
				 CartesianCoordinate3D<float>& coord_2,
				 const int Ring_A,const int Ring_B, 
				 const int det1, const int det2) const
{
//  assert(Ring_A >=0 && Ring_A<get_scanner_ptr()->get_num_rings());
  //assert(Ring_B >=0 && Ring_B<get_scanner_ptr()->get_num_rings());
  //assert(det1 >=0 && det1<get_scanner_ptr()->get_num_detectors_per_ring());
  //assert(det2 >=0 && det2<get_scanner_ptr()->get_num_detectors_per_ring());

  int num_detectors = get_scanner_ptr()->get_num_detectors_per_ring();

  float df1 = (2.*_PI/num_detectors)*(det1);
  float df2 = (2.*_PI/num_detectors)*(det2);
  float x1 = get_scanner_ptr()->get_ring_radius()*cos(df1);
  float y1 = get_scanner_ptr()->get_ring_radius()*sin(df1);
  float x2 = get_scanner_ptr()->get_ring_radius()*cos(df2);
  float y2 = get_scanner_ptr()->get_ring_radius()*sin(df2);
  float z1 = Ring_A*get_scanner_ptr()->get_ring_spacing();
  float z2 = Ring_B*get_scanner_ptr()->get_ring_spacing();
  // make sure the return values are in STIR coordinates
  coord_1.z() = z1;
  coord_1.y() = x1;
  coord_1.x() = -y1;

  coord_2.z() = z2;
  coord_2.y() = x2;
  coord_2.x() = -y2; 
}


void 
ProjDataInfoCylindricalNoArcCorr::
find_bin_given_cartesian_coordinates_of_detection(Bin& bin,
						  const CartesianCoordinate3D<float>& coord_1,
						  const CartesianCoordinate3D<float>& coord_2) const
{
  int det_num_a_trans;
  int det_num_b_trans;
  int ring_a_trans;
  int ring_b_trans;
  
  // given two CartesianCoordinates find the intersection     
  if (find_scanner_coordinates_given_cartesian_coordinates(det_num_a_trans,det_num_b_trans,
							   ring_a_trans, ring_b_trans,
							   coord_1,
							   coord_2) ==
      Succeeded::no)
  {
    bin.set_bin_value(-1);
    return;
  }

  if (ring_a_trans<0 ||
      ring_a_trans>=get_scanner_ptr()->get_num_rings() ||
      ring_b_trans<0 ||
      ring_b_trans>=get_scanner_ptr()->get_num_rings() ||
      get_bin_for_det_pair(bin,
			   det_num_a_trans, ring_a_trans,
			   det_num_b_trans, ring_b_trans) ==
      Succeeded::no ||
      bin.tangential_pos_num() < get_min_tangential_pos_num() ||
      bin.tangential_pos_num() > get_max_tangential_pos_num())
    bin.set_bin_value(-1);
}

END_NAMESPACE_STIR

