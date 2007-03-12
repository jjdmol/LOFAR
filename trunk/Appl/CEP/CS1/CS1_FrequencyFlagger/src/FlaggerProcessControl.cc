/***************************************************************************
 *   Copyright (C) 2006 by ASTRON, Adriaan Renting                         *
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
#include <iostream>
#include <casa/Inputs/Input.h>
#include <ms/MeasurementSets.h>

#include <CS1_FrequencyFlagger/FlaggerProcessControl.h>
#include "MS_File.h"
#include "FrequencyFlagger.h"

#define FLAGGER_VERSION "0.10"

namespace LOFAR
{
  namespace CS1
  {
    //===============>>> FlaggerProcessControl::FlaggerProcessControl  <<<===============
    FlaggerProcessControl::FlaggerProcessControl()
    : ProcessControl()
    {
      myMS       = NULL;
      itsFlagger = NULL;
    }

    //===============>>> FlaggerProcessControl::~FlaggerProcessControl  <<<==============
    FlaggerProcessControl::~FlaggerProcessControl()
    {
      if (myMS)
      { delete myMS;
      }
      if (itsFlagger)
      { delete itsFlagger;
      }
    }

    //===============>>> FlaggerProcessControl::define  <<<==============================
    tribool FlaggerProcessControl::define()
    {
      LOFAR::ACC::APS::ParameterSet* ParamSet = LOFAR::ACC::APS::globalParameterSet();
      itsMS        = ParamSet->getString("ms");
      itsExisting  = ParamSet->getBool("existing");
      itsThreshold = ParamSet->getDouble("threshold");
      return true;
    }

    //===============>>> FlaggerProcessControl::run  <<<=================================
    tribool FlaggerProcessControl::run()
    {
      try{
      std::cout << "Runnning flagger please wait..." << std::endl;
      itsFlagger->FlagDataOrBaselines(itsExisting);
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> FlaggerProcessControl::init  <<<================================
    tribool FlaggerProcessControl::init()
    {
      try {
      using std::cout;
      using std::cerr;
      using std::endl;

      cout  << string(FLAGGER_VERSION) + string(" autoflagging by Adriaan Renting for LOFAR CS1 data\n") +
              string("This is experimental software, please report errors or requests to renting@astron.nl\n") +
              string("Documentation can be found at: www.astron.nl/~renting\n");
      cout << itsMS << endl;
      if (itsMS == "")
      {
        cerr  << "Usage: WSRT_flagger ms=test.MS crosspol=false window=13 min=5 max=5.5 flagrms=true flagdata=true existing=true" << endl
              << "Where ms       [no default]    is the Measurementset" << endl
              << "      existing [default true]  determines if existing flags are kept (no new flag cat. is created)" << endl << endl
              << "Data at point window/2 will be flagged on:" << endl
              << "      vis[window/2] - (median(Re(vis[])) + median(Im(vis[]))) " << endl
              << "    > (min + (max-min)*baselinelength/maxbaselinelength)*sigma" << endl;
        return false;
      }
      myMS       = new MS_File(itsMS);
      itsFlagger = new FrequencyFlagger (myMS, itsThreshold);
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> FlaggerProcessControl::pause  <<<===============================
    tribool FlaggerProcessControl::pause(const std::string&)
    { return false;
    }

    //===============>>> FlaggerProcessControl::quit  <<<================================
    tribool FlaggerProcessControl::quit()
    {
      if (myMS)
      {
        delete myMS;
        myMS = NULL;
      }
      if (itsFlagger)
      {
        delete itsFlagger;
        itsFlagger = NULL;
      }
      return true;
    }

    //===============>>> FlaggerProcessControl::recover  <<<=============================
    tribool FlaggerProcessControl::recover(const std::string&)
    { return false;
    }

    //===============>>> FlaggerProcessControl::reinit  <<<==============================
    tribool FlaggerProcessControl::reinit(const  std::string&)
    { return false;
    }

    //===============>>> FlaggerProcessControl::askInfo  <<<=============================
    std::string FlaggerProcessControl::askInfo(const std::string&)
    { return std::string("");
    }

    //===============>>> FlaggerProcessControl::snapshot  <<<============================
    tribool FlaggerProcessControl::snapshot(const std::string&)
    { return false;
    }
  } //namespace CS1
}; //namespace LOFAR
