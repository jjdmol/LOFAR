/***************************************************************************
 *   Copyright (C) 2006-8 by ASTRON/LOFAR, Adriaan Renting                 *
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
#include <casa/Inputs/Input.h>
#include <ms/MeasurementSets.h>

#include <CS1_IDPPP/PipelineProcessControl.h>
#include "MsInfo.h"
#include "MsFile.h"
#include "RunDetails.h"
#include "Pipeline.h"

#include "BandpassCorrector.h"
#include "Flagger.h"
#include "ComplexMedianFlagger.h"
#include "ComplexMedianFlagger2.h"
#include "FrequencyFlagger.h"
#include "MADFlagger.h"
#include "DataSquasher.h"

#define PIPELINE_VERSION "0.22"

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
      LOFAR::ACC::APS::ParameterSet* ParamSet = LOFAR::ACC::APS::globalParameterSet();
      myDetails  = new RunDetails();
      myDetails->Fixed        = ParamSet->getInt32("fixed", 0);         // BandpassCorrector
      myDetails->FreqWindow   = ParamSet->getInt32("freqwindow", 1);    // FrequencyFlagger, MADFlagger
      myDetails->TimeWindow   = ParamSet->getInt32("timewindow", 1);    // ComplexMedianFlagger, MADFlagger
      myDetails->Treshold     = ParamSet->getDouble("treshold", 1.0);   // FrequencyFlagger, MADFlagger
      myDetails->Algorithm    = ParamSet->getInt32("algorithm", 0);     // FrequencyFlagger
      myDetails->MinThreshold = ParamSet->getDouble("min", 1.0);        // ComplexMedianFlagger
      myDetails->MaxThreshold = ParamSet->getDouble("max", 1.0);        // ComplexMedianFlagger
      myDetails->Existing     = ParamSet->getBool("existing", false);   // all flaggers
      myDetails->NChan        = ParamSet->getInt32("nchan");            // DataSquasher
      myDetails->Start        = ParamSet->getInt32("start");            // DataSquasher
      myDetails->Step         = ParamSet->getInt32("step");             // DataSquasher
      myDetails->Skip         = ParamSet->getBool("skipflags", false);  // DataSquasher
      myDetails->Columns      = ParamSet->getBool("allcolumns", false); // DataSquasher
      itsInMS                 = ParamSet->getString("msin");
      itsOutMS                = ParamSet->getString("msout");
      itsBandpass             = ParamSet->getInt32("bandpass", 0);
      itsFlagger              = ParamSet->getInt32("flagger", 0);
      itsSquasher             = ParamSet->getInt32("squasher", 0);
      myDetails->PrintInfo();
      return true;
    }

    //===============>>> PipelineProcessControl::run  <<<=================================
    tribool PipelineProcessControl::run()
    {
      try{
      std::cout << "Runnning pipeline please wait..." << std::endl;
        myFile->Init(*myInfo, *myDetails);
        MsInfo* outInfo = new MsInfo(itsOutMS);
        outInfo->Update();
        outInfo->PrintInfo();
        myPipeline->Run(outInfo, myDetails->Columns);
        delete outInfo;
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
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

        cout  << string(PIPELINE_VERSION) + string(" Integrated Data Post Processing Pipeline by Adriaan Renting and others for LOFAR CS1 data\n") +
                string("This is experimental software, please report errors or requests to renting@astron.nl\n") +
                string("Documentation can be found at: www.lofar.org/operations\n");

        myFile     = new MsFile(itsInMS, itsOutMS);
        myInfo     = new MsInfo(itsInMS);
        myInfo->Update();
        myInfo->PrintInfo();
        switch (itsBandpass)
        {
          case 1:  myBandpass = new BandpassCorrector(); break;
        }
        switch (itsFlagger)
        {
          case 1:  myFlagger = new ComplexMedianFlagger(); break;
          case 2:  myFlagger = new ComplexMedianFlagger2(); break;
          case 3:  myFlagger = new FrequencyFlagger(); break;
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
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
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
      myPipeline = 0;
      delete myFile;
      myFile = 0;
      delete myInfo;
      myInfo = 0;
      delete myBandpass;
      myBandpass = 0;
      delete myFlagger;
      myFlagger = 0;
      delete mySquasher;
      mySquasher = 0;
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
    { return std::string("");
    }

    //===============>>> PipelineProcessControl::snapshot  <<<============================
    tribool PipelineProcessControl::snapshot(const std::string&)
    { return false;
    }
  } //namespace CS1
}; //namespace LOFAR
