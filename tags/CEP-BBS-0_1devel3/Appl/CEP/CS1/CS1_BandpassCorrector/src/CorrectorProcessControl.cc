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

#include <CS1_BandpassCorrector/CorrectorProcessControl.h>
#include "MS_File.h"
#include "BandpassCorrector.h"

#define CORRECTOR_VERSION "0.10"

namespace LOFAR
{
  namespace CS1
  {
    //===============>>> CorrectorProcessControl::CorrectorProcessControl  <<<===============
    CorrectorProcessControl::CorrectorProcessControl()
    : ProcessControl()
    {
      myMS       = NULL;
      itsCorrector = NULL;
    }

    //===============>>> CorrectorProcessControl::~CorrectorProcessControl  <<<==============
    CorrectorProcessControl::~CorrectorProcessControl()
    {
      if (myMS)
      { delete myMS;
      }
      if (itsCorrector)
      { delete itsCorrector;
      }
    }

    //===============>>> CorrectorProcessControl::define  <<<==============================
    tribool CorrectorProcessControl::define()
    {
      LOFAR::ACC::APS::ParameterSet* ParamSet = LOFAR::ACC::APS::globalParameterSet();
      itsMS       = ParamSet->getString("ms");
      itsFixed    = ParamSet->getInt32("fixed");
      itsWindow   = ParamSet->getInt32("window");
      return true;
    }

    //===============>>> CorrectorProcessControl::run  <<<=================================
    tribool CorrectorProcessControl::run()
    {
      try{
      std::cout << "Runnning Bandpass corrector please wait..." << std::endl;
      itsCorrector->CorrectBandpass(itsFixed);
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> CorrectorProcessControl::init  <<<================================
    tribool CorrectorProcessControl::init()
    {
      try {
      using std::cout;
      using std::cerr;
      using std::endl;

      cout  << string(CORRECTOR_VERSION) + string(" Bandpass corrector by Adriaan Renting for LOFAR data\n") +
              string("This is experimental software, please report errors or requests to renting@astron.nl\n") +
              string("Documentation can be found at: www.astron.nl/~renting\n");
      cout << itsMS << endl;
      if (itsMS == "")
      {
        cerr  << "Usage: CS1_BandpassCorrector ms=test.MS window=13" << endl
              << "Where ms       [no default]    is the Measurementset" << endl
              << "      window   [default 13]    is the window size for the used median" << endl
              << "    **More information needed here**" << endl;
        return false;
      }
      myMS         = new MS_File(itsMS);
      itsCorrector = new BandpassCorrector (myMS, itsWindow);
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> CorrectorProcessControl::pause  <<<===============================
    tribool CorrectorProcessControl::pause(const std::string&)
    { return false;
    }

    //===============>>> CorrectorProcessControl::quit  <<<================================
    tribool CorrectorProcessControl::quit()
    {
      if (myMS)
      {
        delete myMS;
        myMS = NULL;
      }
      if (itsCorrector)
      {
        delete itsCorrector;
        itsCorrector = NULL;
      }
      return true;
    }

    //===============>>> CorrectorProcessControl::recover  <<<=============================
    tribool CorrectorProcessControl::recover(const std::string&)
    { return false;
    }

    //===============>>> CorrectorProcessControl::reinit  <<<==============================
    tribool CorrectorProcessControl::reinit(const  std::string&)
    { return false;
    }

    //===============>>> CorrectorProcessControl::askInfo  <<<=============================
    std::string CorrectorProcessControl::askInfo(const std::string&)
    { return std::string("");
    }

    //===============>>> CorrectorProcessControl::snapshot  <<<============================
    tribool CorrectorProcessControl::snapshot(const std::string&)
    { return false;
    }
  } //namespace CS1
}; //namespace LOFAR
