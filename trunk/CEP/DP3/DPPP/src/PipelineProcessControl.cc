//# Copyright (C) 2006-8
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
//#
//# @author Adriaan Renting

#include <lofar_config.h>
#include <iostream>
#include <casa/Inputs/Input.h>
#include <ms/MeasurementSets.h>
#include <casa/OS/Path.h>

#include <MS/VdsMaker.h>

#include <DPPP/PipelineProcessControl.h>
#include <DPPP/MsInfo.h>
#include <DPPP/MsFile.h>
#include <DPPP/RunDetails.h>
#include <DPPP/Pipeline.h>

#include <DPPP/BandpassCorrector.h>
#include <DPPP/Flagger.h>
#include <DPPP/ComplexMedianFlagger.h>
#include <DPPP/ComplexMedianFlagger2.h>
#include <DPPP/FrequencyFlagger.h>
#include <DPPP/MADFlagger.h>
#include <DPPP/DataSquasher.h>

#define PIPELINE_VERSION "0.43"
// 0.23 Added AbsoluteThreshold for MADFlagger
// 0.24 Added writing VDS file
// 0.30 Changed Time handling and interval calculation
// 0.31 Some other bug fixes
// 0.32 More bugfixes
// 0.33 Support for lofar storage manager
// 0.34 Fixed bugs in WEIGHT_SPECTRUM stuff
// 0.40 changed order of flaggers to be as in documentation!
// 0.41 allow flagger to use other columns
// 0.42 small change to handle odd number of input columns when writing CHAN_FREQ
// 0.43 check data regularity

namespace LOFAR
{
  namespace CS1
  {
    //===============>>> PipelineProcessControl::PipelineProcessControl  <<<===============
    PipelineProcessControl::PipelineProcessControl()
    : ProcessControl(),
      myPipeline(NULL),
      myFile(NULL),
      myInfo(NULL),
      myBandpass(NULL),
      myFlagger(NULL),
      mySquasher(NULL),
      myDetails(NULL)
    {
    }

    //===============>>> PipelineProcessControl::~PipelineProcessControl  <<<==============
    PipelineProcessControl::~PipelineProcessControl()
    {
    }

    //===============>>> PipelineProcessControl::define  <<<==============================
    tribool PipelineProcessControl::define()
    {
      ParameterSet* ParamSet = globalParameterSet();
      myDetails  = new RunDetails();
      myDetails->Fixed        = ParamSet->getUint32("fixed", 0);           // BandpassCorrector
      myDetails->FreqWindow   = ParamSet->getUint32("freqwindow", 1);      // FrequencyFlagger, MADFlagger
      myDetails->TimeWindow   = ParamSet->getUint32("timewindow", 1);      // ComplexMedianFlagger, MADFlagger
      myDetails->Threshold    = ParamSet->getDouble("threshold", 1.0);     // FrequencyFlagger, MADFlagger
      myDetails->Algorithm    = ParamSet->getUint32("algorithm", 0);       // FrequencyFlagger
      myDetails->MinThreshold = ParamSet->getDouble("min", 0.0);           // ComplexMedianFlagger
      myDetails->MaxThreshold = ParamSet->getDouble("max", 0.0);           // ComplexMedianFlagger, MADFlagger
      myDetails->Existing     = ParamSet->getBool("existing", false);      // all flaggers
      myDetails->NChan        = ParamSet->getUint32("nchan", 0);           // DataSquasher
      myDetails->Start        = ParamSet->getUint32("start", 0);           // DataSquasher
      myDetails->Step         = ParamSet->getUint32("step", 1);            // DataSquasher
      myDetails->Skip         = ParamSet->getBool("skipflags", false);     // DataSquasher
      myDetails->Columns      = ParamSet->getBool("allcolumns", false);    // DataSquasher
      myDetails->TimeStep     = ParamSet->getUint32("timestep", 1);        // DataSquasher
      myDetails->TileNChan    = ParamSet->getUint32("tilenfreq", 8);       // default 8 chan
      myDetails->TileSize     = ParamSet->getUint32("tilesize", 1024);     // default 1024 KB
      myDetails->DataColumn   = ParamSet->getString("datacolumn", "DATA"); // column to flag, all flaggers
      itsInMS                 = ParamSet->getString("msin");
      itsOutMS                = ParamSet->getString("msout");
      itsClusterDesc          = ParamSet->getString("clusterdesc");
      itsVdsDir               = ParamSet->getString("vdsdir", string());
      itsBandpass             = ParamSet->getUint32("bandpass", 0);
      itsFlagger              = ParamSet->getUint32("flagger", 0);
      itsSquasher             = ParamSet->getUint32("squasher", 0);
      if (myDetails->CheckValues())
      { return false;
      }
      if (itsFlagger == 0
          && (myDetails->FreqWindow != 1 || myDetails->TimeWindow != 1))
      { myDetails->FreqWindow = 1;
        myDetails->TimeWindow = 1;
        std::cout << "No flagger, time and frequency windows reset to 1x1" << std::endl;
      }
      if (itsSquasher == 0
          && (myDetails->Step != 1 || myDetails->Start != 0 || myDetails->TimeStep != 1))
      { myDetails->Step     = 1;
        myDetails->Start    = 0;
        myDetails->TimeStep = 1;
        std::cout << "No squasher, step, start and timestep sizes reset" << std::endl;
      }
      myDetails->PrintInfo();
      return true;
    }

    //===============>>> PipelineProcessControl::run  <<<=================================
    tribool PipelineProcessControl::run()
    {
      try{
        std::cout << "Runnning pipeline please wait..." << std::endl;
        myFile->Init(*myInfo, *myDetails, itsSquasher);
        myFile->PrintInfo();
        MsInfo* outInfo = new MsInfo(myFile->getOutMS(), myFile->getOutMS());
        outInfo->PrintInfo();
        myPipeline->Run(outInfo, myDetails->Columns);
        delete outInfo;
        myFile->flush();
        if (!itsClusterDesc.empty())
        {
          string vdsName = itsOutMS + ".vds";
          if (! itsVdsDir.empty())
          {
            if (itsVdsDir[itsVdsDir.size() - 1] != '/')
            {
              itsVdsDir.append ("/");
            }
            vdsName = itsVdsDir + string(casa::Path(vdsName).baseName());
          }
          // Create VDS file without detailed time info.
          LOFAR::VdsMaker::create (itsOutMS, vdsName, itsClusterDesc, "", false);
        }
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "AIPS++/Casa(core) error detected: " << err.getMesg() << std::endl;
        return false;
      }
      catch(std::exception& err)
      {
        std::cerr << "std::exception detected: " << err.what() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> PipelineProcessControl::init  <<<================================
    tribool PipelineProcessControl::init()
    {
      try {
        using std::cout;
        using std::cerr;
        using std::endl;

        cout  << string(PIPELINE_VERSION) + string(" Integrated Data Post Processing Pipeline by Adriaan Renting and others for LOFAR data\n") +
                string("This is experimental software, please report errors or requests to renting@astron.nl\n") +
                string("Documentation can be found at: www.lofar.org/operations\n");

        myFile     = new MsFile(itsInMS, itsOutMS);
        myInfo     = new MsInfo(myFile->getInMS(), myFile->getOrderedInTable());
        myInfo->PrintInfo();
        switch (itsBandpass)
        {
          case 1:  myBandpass = new BandpassCorrector(); break;
        }
        switch (itsFlagger)
        {
          case 1:  myFlagger = new FrequencyFlagger(); break;
          case 2:  myFlagger = new ComplexMedianFlagger(); break;
          case 3:  myFlagger = new ComplexMedianFlagger2(); break;
          case 4:  myFlagger = new MADFlagger(); break;
        }
        switch (itsSquasher)
        {
          case 1:  mySquasher = new DataSquasher(); break;
        }
        myPipeline = new Pipeline(myInfo, myFile, myDetails,
                                  myBandpass, myFlagger, mySquasher);
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "AIPS++/Casa(core) error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> PipelineProcessControl::pause  <<<===============================
    tribool PipelineProcessControl::pause(const std::string&)
    {
      return indeterminate;
    }

    //===============>>> PipelineProcessControl::quit  <<<================================
    tribool PipelineProcessControl::quit()
    {
      return true;
    }

    //===============>>> PipelineProcessControl::release  <<<=============================
    tribool PipelineProcessControl::release()
    {
      delete myPipeline;
      myPipeline = NULL;
      delete myFile;
      myFile = NULL;
      delete myInfo;
      myInfo = NULL;
      delete myBandpass;
      myBandpass = NULL;
      delete myFlagger;
      myFlagger = NULL;
      delete mySquasher;
      mySquasher = NULL;
      delete myDetails;
      myDetails = NULL;
      return true;
    }

    //===============>>> PipelineProcessControl::recover  <<<=============================
    tribool PipelineProcessControl::recover(const std::string&)
    { return false;
    }

    //===============>>> PipelineProcessControl::reinit  <<<==============================
    tribool PipelineProcessControl::reinit(const  std::string&)
    { return false;
    }

    //===============>>> PipelineProcessControl::askInfo  <<<=============================
    std::string PipelineProcessControl::askInfo(const std::string&)
    { return std::string();
    }

    //===============>>> PipelineProcessControl::snapshot  <<<============================
    tribool PipelineProcessControl::snapshot(const std::string&)
    { return false;
    }
  } //namespace CS1
}; //namespace LOFAR
