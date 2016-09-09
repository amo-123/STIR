/*!
  \file
  \ingroup IO
  \brief Declaration of class stir::InputStreamWithRecords

  \author Nikos Efthimiou
*/
/*
 *  Copyright (C) 2015, 2016 University of Leeds
    Copyright (C) 2016, UCL
    This file is part of STIR.

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This file is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    See STIR/LICENSE.txt for details
*/

#ifndef __stir_IO_InputStreamFromROOTFile_H__
#define __stir_IO_InputStreamFromROOTFile_H__

#include "stir/shared_ptr.h"
#include "stir/Succeeded.h"
#include "stir/listmode/CListRecordROOT.h"

#include <TROOT.h>
#include <TSystem.h>
#include <TChain.h>
#include <TH2D.h>
#include <TDirectory.h>
#include <TList.h>
#include <TChainElement.h>
#include <TTree.h>
#include <TFile.h>
#include <TVersionCheck.h>

START_NAMESPACE_STIR

//! A helper class to read data from a ROOT file Generated by GATE simulation toolkit
/*! \ingroup IO
        \author Nikos Efthimiou

        \details This class takes as input a root file, and returns the data stored in a meaningfull
        way. The validation of the ROOT input was done with version 5.34.
*/

class InputStreamFromROOTFile
{
public:
    typedef std::vector<long long int>::size_type SavedPosition;

    //! Default constructor
    InputStreamFromROOTFile(std::string filename,
                            std::string chain_name,
                            int crystal_repeater_x, int crystal_repeater_y, int crystal_repeater_z,
                            int submodule_repeater_x, int submodule_repeater_y, int submodule_repeater_z,
                            int module_repeater_x, int module_repeater_y, int module_repeater_z,
                            int rsector_repeater,
                            bool exclude_scattered, bool exclude_randoms,
                            float low_energy_window, float up_energy_window,
                            int offset_dets);


    virtual ~InputStreamFromROOTFile() {}

    //!
    //! \brief get_next_record
    //! \param record Reference to the Record
    //! \return
    //!  \details Returns the next record in the ROOT file.
    //!  The code is adapted from Sadek A. Nehmeh and CR Schmidtlein,
    //! downloaded from <a href="http://www.opengatecollaboration.org/STIR">GATE website</a>
    virtual
    Succeeded get_next_record(CListRecordROOT& record) ;
    //! Go to the first event.
    inline Succeeded reset();
    //! Save current position in a vector
    inline
    SavedPosition save_get_position();
    //! Set current position
    inline
    Succeeded set_get_position(const SavedPosition&);
    //! Get the vector with the saved positions
    inline
    std::vector<long long int> get_saved_get_positions() const;
    //! Set a vector with saved positions
    inline
    void set_saved_get_positions(const std::vector<long long int>& );
    //! Returns the total number of events
    inline long long int
    get_total_number_of_events();

private:

    virtual Succeeded initialize_root_read_out();
    //! Input data file name
    std::string filename;
    //! The starting position.
    long long int starting_stream_position;
    //! The total number of entries
    long long int nentries;
    //! Current get position
    long long int current_position;
    //! A vector with saved position indices.
    std::vector<long long int> saved_get_positions;

    // ROOT chain
    TChain *stream_ptr;

    // Variables to store root information
    std::string chain_name;
    Int_t           eventID1, eventID2;
    Int_t           crystalID1, crystalID2;
    Int_t           submoduleID1, submoduleID2;
    Int_t           moduleID1, moduleID2;
    Int_t           rsectorID1, rsectorID2;
//    Int_t           rotationAngle;
//    Float_t         globalPosX1, globalPosX2, sourcePosX1, sourcePosX2, sourcePosY1, sourcePosY2;
//    Float_t         globalPosY1, globalPosY2, globalPosZ1, globalPosZ2, sourcePosZ1, sourcePosZ2;
    Double_t        time1, time2;
    Float_t         energy1, energy2;
//    Int_t           sourceid1, sourceid2;
    Int_t           comptonphantom1, comptonphantom2;

    int crystal_repeater_x;
    int crystal_repeater_y;
    int crystal_repeater_z;
    int submodule_repeater_x;
    int submodule_repeater_y;
    int submodule_repeater_z;
    int module_repeater_x;
    int module_repeater_y;
    int module_repeater_z;
    int rsector_repeater;

    bool exclude_scattered;
    bool exclude_randoms;

    float low_energy_window;
    float up_energy_window;
    //! In GATE, inside a block, the indeces start from the lower
    //! unit counting upwards. Therefore in order to align the
    //! crystals, between STIR and GATE we have to move half block more.
    int half_block;
    int offset_dets;
};

END_NAMESPACE_STIR

#include "stir/IO/InputStreamFromROOTFile.inl"

#endif
