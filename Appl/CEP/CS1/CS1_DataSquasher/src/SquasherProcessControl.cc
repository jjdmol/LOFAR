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

#include <lofar_config.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <tables/Tables.h>
#include <ms/MeasurementSets.h>
#include <casa/Exceptions.h>

#include <CS1_DataSquasher/SquasherProcessControl.h>
#include "DataSquasher.h"

#define SQUASHER_VERSION "0.50"
//0.20 Added handling MODEL and CORRECTED DATA
//0.30 Added handling threshold and weights
//0.31 Added handing MODEL_DATA and CORRECTED_DATA keywords for imager
//0.40 Changed creation mechnism of the destination MS
//0.41 Cleaned up the code for readability
//0.42 Fixed the incorect weighting of partially flagged bands
//0.43 Some code cleanup, and maybe small error fix
//0.50 Added handling of WEIGHT_SPECTRUM

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
      //maybe most of this code should move to the DataSquasher module
      try{
        std::cout << "Creating " << itsOutMS << ", please wait..." << std::endl;
        Table temptable = tableCommand(string("SELECT UVW,FLAG_CATEGORY,WEIGHT,SIGMA,ANTENNA1,ANTENNA2,ARRAY_ID,DATA_DESC_ID,") +
                                       string("EXPOSURE,FEED1,FEED2,FIELD_ID,FLAG_ROW,INTERVAL,OBSERVATION_ID,PROCESSOR_ID,") +
                                       string("SCAN_NUMBER,STATE_ID,TIME,TIME_CENTROID,FLAG FROM ") + itsInMS);
        // NOT copying WEIGHT_SPECTRUM as it only contains dummy data anyway

        // Need FLAG to make it a valid MS
        temptable.deepCopy(itsOutMS, Table::NewNoReplace);
        MeasurementSet inMS  = MeasurementSet(itsInMS);
        MeasurementSet outMS = MeasurementSet(itsOutMS, Table::Update);

        //some magic to create a new DATA column
        TableDesc tdesc = inMS.tableDesc();
        ColumnDesc desc = tdesc.rwColumnDesc("DATA");
        IPosition ipos = desc.shape();
        Vector<Int> temp_pos = ipos.asVector();
        std::cout << "Old shape: " << temp_pos(0) << ":" <<  temp_pos(1) << std::endl;
        int old_nchan = temp_pos(1);
        int new_nchan = itsNChan/itsStep;
        temp_pos(1) = new_nchan;
        std::cout << "New shape: " << temp_pos(0) << ":" <<  temp_pos(1) << std::endl;
        IPosition data_ipos(temp_pos);

        if (tdesc.isColumn("WEIGHT_SPECTRUM"))
        { tdesc.removeColumn("WEIGHT_SPECTRUM");
        }
        tdesc.addColumn(ArrayColumnDesc<Float>("WEIGHT_SPECTRUM", "Added by datasquasher",
                                               data_ipos, ColumnDesc::FixedShape));

        itsSquasher->TableResize(tdesc, data_ipos, "DATA", outMS);
        itsSquasher->TableResize(tdesc, data_ipos, "WEIGHT_SPECTRUM", outMS);

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
            itsSquasher->TableResize(tdesc, data_ipos, "CORRECTED_DATA", outMS);

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
            desc.setShape(data_ipos);
            desc.setOptions(4);
            desc.rwKeywordSet().removeField("CHANNEL_SELECTION"); //messes with the Imager if it's there but has wrong values
            Matrix<Int> selection;
            selection.resize(2, itsSquasher->itsNumBands); //dirty hack with direct reference to itsSquasher
            selection.row(0) = 0; //start in Imager, will therefore only work if imaging whole SPW
            selection.row(1) = new_nchan;
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
        itsSquasher->TableResize(tdesc, data_ipos, "FLAG", outMS);
        ArrayColumn<Bool> flags(outMS, "FLAG");
        flags.putColumn(NewFlags);

        //Fix the SpectralWindow values
        IPosition spw_ipos(1,new_nchan);
        MSSpectralWindow inSPW = inMS.spectralWindow();
        //ugly workaround MSSpectral window does no allow deleting and then recreating columns
        Table outSPW = Table(itsOutMS + "/SPECTRAL_WINDOW", Table::Update);
        ScalarColumn<Int> channum(outSPW, "NUM_CHAN");
        channum.fillColumn(new_nchan);

        TableDesc SPWtdesc = inSPW.tableDesc();
        itsSquasher->TableResize(SPWtdesc, spw_ipos, "CHAN_FREQ", outSPW);

        itsSquasher->TableResize(SPWtdesc, spw_ipos, "CHAN_WIDTH", outSPW);

        itsSquasher->TableResize(SPWtdesc, spw_ipos, "EFFECTIVE_BW", outSPW);

        itsSquasher->TableResize(SPWtdesc, spw_ipos, "RESOLUTION", outSPW);

        ROArrayColumn<Double> inFREQ(inSPW, "CHAN_FREQ");
        ROArrayColumn<Double> inWIDTH(inSPW, "CHAN_WIDTH");
        ROArrayColumn<Double> inBW(inSPW, "EFFECTIVE_BW");
        ROArrayColumn<Double> inRESOLUTION(inSPW, "RESOLUTION");

        ArrayColumn<Double> outFREQ(outSPW, "CHAN_FREQ");
        ArrayColumn<Double> outWIDTH(outSPW, "CHAN_WIDTH");
        ArrayColumn<Double> outBW(outSPW, "EFFECTIVE_BW");
        ArrayColumn<Double> outRESOLUTION(outSPW, "RESOLUTION");

        Vector<Double> old_temp(old_nchan, 0.0);
        Vector<Double> new_temp(new_nchan, 0.0);

        for (unsigned int i = 0; i < inSPW.nrow(); i++)
        {
          for (int j = 0; j < new_nchan; j++)
          { inFREQ.get(i, old_temp);
            if (itsStep % 2) //odd number of channels in step
            { new_temp(j) = old_temp(itsStart + j*itsStep + (itsStep + 1)/2);
            }
            else //even number of channels in step
            { new_temp(j) = 0.5 * (old_temp(itsStart + j*itsStep + itsStep/2 -1)
                                   + old_temp(itsStart + j*itsStep + itsStep/2));
            }
            outFREQ.put(i, new_temp);
          }
          for (int j = 0; j < new_nchan; j++)
          { inWIDTH.get(i, old_temp);
            new_temp(j) = old_temp(0) * itsStep;
            outWIDTH.put(i, new_temp);
          }
          for (int j = 0; j < new_nchan; j++)
          { inBW.get(i, old_temp);
            new_temp(j) = old_temp(0) * itsStep;
            outBW.put(i, new_temp);
          }
          for (int j = 0; j < new_nchan; j++)
          { inRESOLUTION.get(i, old_temp);
            new_temp(j) = old_temp(0) * itsStep;
            outRESOLUTION.put(i, new_temp);
          }
        }

        //Fix WEIGHT
/*        ArrayColumn<Float> weight(outMS, "WEIGHT");
        Vector<Float> temp_weight(itsSquasher->itsNumPolarizations, 1.0); //dirty hack with direct reference to itsSquasher
        weight.fillColumn(temp_weight);*/
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
              string("Documentation can be found at: www.lofar.org/operations/doku.php?id=engineering:software:postprocessing_software\n");

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

    //===============>>> SquasherProcessControl::release  <<<=============================
    tribool SquasherProcessControl::release()
    { return false;
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
