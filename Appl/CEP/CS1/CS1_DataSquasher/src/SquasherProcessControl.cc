/***************************************************************************
 *   Copyright (C) 2007 by Adriaan Renting, ASTRON                         *
 *   renting@astron.nl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>
#include <cstdlib>
#include <string>
#include <tables/Tables.h>
#include <ms/MeasurementSets.h>
#include <casa/Exceptions.h>

#include <CS1_DataSquasher/SquasherProcessControl.h>
#include "DataSquasher.h"

#define SQUASHER_VERSION "0.31"
//0.20 Added handling MODEL and CORRECTED DATA
//0.30 Added handling threshold and weights

namespace LOFAR
{
  namespace CS1
  {
    using namespace casa;
    //===============>>> SquasherProcessControl::SquasherProcessControl  <<<===============
    SquasherProcessControl::SquasherProcessControl()
    : ProcessControl()
    {
      itsSquasher = NULL;
    }

    //===============>>> SquasherProcessControl::~SquasherProcessControl  <<<==============
    SquasherProcessControl::~SquasherProcessControl()
    {
      if (itsSquasher)
      { delete itsSquasher;
      }
    }

    //===============>>> SquasherProcessControl::define  <<<==============================
    tribool SquasherProcessControl::define()
    {
      LOFAR::ACC::APS::ParameterSet* ParamSet = LOFAR::ACC::APS::globalParameterSet();
      itsInMS      = ParamSet->getString("inms");
      itsOutMS     = ParamSet->getString("outms");
      itsStart     = ParamSet->getInt32("start");
      itsStep      = ParamSet->getInt32("step");
      itsNChan     = ParamSet->getInt32("nchan");
      itsThreshold = ParamSet->getFloat("threshold");
      itsSkip      = ParamSet->getBool("useflags"); //used to be called "skip flags", hence the name
      itsColumns   = ParamSet->getBool("allcolumns");
      return true;
    }

    //===============>>> SquasherProcessControl::run  <<<=================================
    tribool SquasherProcessControl::run()
    {
      try{
        std::cout << "Creating " << itsOutMS << ", please wait..." << std::endl;
        Table temptable = tableCommand(string("SELECT UVW,FLAG_CATEGORY,WEIGHT,SIGMA,ANTENNA1,ANTENNA2,ARRAY_ID,DATA_DESC_ID,") +
          string("EXPOSURE,FEED1,FEED2,FIELD_ID,FLAG_ROW,INTERVAL,OBSERVATION_ID,PROCESSOR_ID,SCAN_NUMBER,") +
          string("STATE_ID,TIME,TIME_CENTROID,WEIGHT_SPECTRUM,FLAG FROM ") + itsInMS);
        temptable.deepCopy(itsOutMS, Table::NewNoReplace);

        MeasurementSet inMS  = MeasurementSet(itsInMS);
        MeasurementSet outMS = MeasurementSet(itsOutMS, Table::Update);

        //some magic to create a new DATA column
        TableDesc tdesc = inMS.tableDesc();
        ColumnDesc desc = tdesc.rwColumnDesc("DATA");
        IPosition pos = desc.shape();
        Vector<Int> temp = pos.asVector();
        std::cout << "Old shape: " << temp(0) << ":" <<  temp(1) << std::endl;
        temp(1) = itsNChan/itsStep;
        std::cout << "New shape: " << temp(0) << ":" <<  temp(1) << std::endl;
        IPosition pos2(temp);
        desc.setOptions(0);
        desc.setShape(pos2);
        desc.setOptions(4);
        outMS.addColumn(desc);

        //do the actual data squashing
        Cube<Bool> NewFlags(0, 0, 0);
        itsSquasher->Squash(inMS, outMS, "DATA",
                            itsStart, itsStep, itsNChan, itsThreshold,
                            itsSkip, NewFlags);

        //if present handle the CORRECTED_DATA column
        if (tdesc.isColumn("CORRECTED_DATA"))
        {
          if (itsColumns)
          {
            cout << "Processing CORRECTED_DATA" << endl;
            desc = tdesc.rwColumnDesc("CORRECTED_DATA");
            desc.setOptions(0);
            desc.setShape(pos2);
            desc.setOptions(4);
            outMS.addColumn(desc);

            //do the actual data squashing
            itsSquasher->Squash(inMS, outMS, "CORRECTED_DATA",
                                itsStart, itsStep, itsNChan, itsThreshold,
                                itsSkip, NewFlags);
          }
          else
          { outMS.removeColumn("CORRECTED_DATA");
          }
        }

        //if present handle the MODEL_DATA column
        if (tdesc.isColumn("MODEL_DATA"))
        {
          if (itsColumns)
          {
            cout << "Processing MODEL_DATA" << endl;
            desc = tdesc.rwColumnDesc("MODEL_DATA");
            desc.setOptions(0);
            desc.setShape(pos2);
            desc.setOptions(4);
            desc.rwKeywordSet().removeField("CHANNEL_SELECTION"); //messes with the Imager if it's there.
            Matrix<Int> selection;
            selection.resize(2, itsSquasher->itsNumBands); //dirty hack with direct reference to itsSquasher
            selection.row(0) = 0; //start
            selection.row(1) = itsNChan/itsStep;
            desc.rwKeywordSet().define("CHANNEL_SELECTION",selection); // #spw x [startChan, NumberChan] for the VisBuf in the Imager
            // see code/msvis/implement/MSVis/VisSet.cc
            outMS.addColumn(desc);
            outMS.addColumn(ArrayColumnDesc<Float>("IMAGING_WEIGHT","imaging weight", 1));

            //do the actual data squashing
            itsSquasher->Squash(inMS, outMS, "MODEL_DATA",
                                itsStart, itsStep, itsNChan, itsThreshold,
                                itsSkip, NewFlags);
          }
          else
          { outMS.removeColumn("MODEL_DATA");
            outMS.removeColumn("IMAGING_WEIGHT");
          }
        }

        //fix the FLAGS column
        desc = tdesc.rwColumnDesc("FLAG");
        desc.setOptions(0);
        desc.setShape(pos2);
        desc.setOptions(4);
        outMS.removeColumn("FLAG");
        outMS.addColumn(desc);
        ArrayColumn<Bool> flags(outMS, "FLAG");
        flags.putColumn(NewFlags);

        //Fix the SpectralWindow values
        MSSpectralWindow SPW = outMS.spectralWindow();
        ScalarColumn<Int> channum(SPW, "NUM_CHAN");
        channum.fillColumn(itsNChan/itsStep);
        ArrayColumn<Float> weight(outMS, "WEIGHT");
        Vector<Float> temp2(itsSquasher->itsNumPolarizations, 1.0); //dirty hack with direct reference to itsSquasher
        weight.fillColumn(temp2);
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> SquasherProcessControl::init  <<<================================
    tribool SquasherProcessControl::init()
    {
      try {
      using std::cout;
      using std::cerr;
      using std::endl;

      cout  << string(SQUASHER_VERSION) + string(" data squasher by Adriaan Renting for LOFAR CS1\n") +
              string("This is experimental software, please report errors or requests to renting@astron.nl\n") +
              string("Documentation can be found at: www.lofar.org/wiki/index.php/CS1/Software#Post_Processing_tools\n");
      cout << string("Squashing ") << itsInMS << string(" into ") << itsOutMS << endl;
      if (itsInMS == "" || itsOutMS == "")
      {
        cerr  << " Error missing input" << endl;
        return false;
      }
      itsSquasher = new DataSquasher();
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> SquasherProcessControl::pause  <<<===============================
    tribool SquasherProcessControl::pause(const std::string&)
    { return false;
    }

    //===============>>> SquasherProcessControl::quit  <<<================================
    tribool SquasherProcessControl::quit()
    {
      if (itsSquasher)
      {
        delete itsSquasher;
        itsSquasher = NULL;
      }
      return true;
    }

    //===============>>> SquasherProcessControl::recover  <<<=============================
    tribool SquasherProcessControl::recover(const std::string&)
    { return false;
    }

    //===============>>> SquasherProcessControl::reinit  <<<==============================
    tribool SquasherProcessControl::reinit(const  std::string&)
    { return false;
    }

    //===============>>> SquasherProcessControl::askInfo  <<<=============================
    std::string SquasherProcessControl::askInfo(const std::string&)
    { return std::string("");
    }

    //===============>>> SquasherProcessControl::snapshot  <<<============================
    tribool SquasherProcessControl::snapshot(const std::string&)
    { return false;
    }
  } //namespace CS1
}; //namespace LOFAR
