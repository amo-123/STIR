//
// %W%: %E%
//

/*!
  \file
  \ingroup utilities 
  \brief simple adding of images

  \author Kris Thielemans

  \date %E%
  \version %I%

  \warning This does not check if image types are the same, nor if origins and
  voxel sizes etc. match.
*/

#include "DiscretisedDensity.h"
#include "interfile.h"

#ifndef TOMO_NO_NAMESPACES
using std::cerr;
using std::cout;
using std::endl;
#endif



//********************** main



USING_NAMESPACE_TOMO


int main(int argc, char *argv[])
{


  if(argc<4)
  {
    cerr<< "Usage: " << argv[0] << " out_image image1 image2 [image3...]\n";
    exit(EXIT_FAILURE);
  }

  --argc;
  ++argv;
  const char * const out_image_filename = *argv;

  --argc;
  ++argv;
  cout << "Reading image " << *argv << endl;
  shared_ptr< DiscretisedDensity<3,float> >  image_ptr = 
    DiscretisedDensity<3,float>::read_from_file(*argv);


  while(--argc > 0)
  {
    ++argv;
    cout << "Reading image " << *argv << endl;
    shared_ptr< DiscretisedDensity<3,float> >  image_to_add_ptr = 
    DiscretisedDensity<3,float>::read_from_file(*argv);

    *image_ptr += *image_to_add_ptr;
  }

  cout << "Writing add_image " << out_image_filename << endl;
  write_basic_interfile(out_image_filename, *image_ptr);

  return EXIT_SUCCESS;

} //end main
