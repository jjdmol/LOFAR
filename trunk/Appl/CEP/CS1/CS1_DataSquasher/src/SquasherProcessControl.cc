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
#include <iostream>
#include <ms/MeasurementSets.h>

#include <CS1_DataSquasher/SquasherProcessControl.h>
#include "DataSquasher.h"

#define SQUASHER_VERSION "0.10"

namespace LOFAR
{
  namespace CS1
  {
    //===============>>> SquasherProcessControl::SquasherProcessControl  <<<===============
    SquasherProcessControl::SquasherProcessControl()
    : ProcessControl()
    {
      inMS        = NULL;
      outMS       = NULL;
      itsSquasher = NULL;
    }

    //===============>>> SquasherProcessControl::~SquasherProcessControl  <<<==============
    SquasherProcessControl::~SquasherProcessControl()
    {
      if (inMS)
      { delete inMS;
      }
      if (outMS)
      { delete outMS;
      }
      if (itsSquasher)
      { delete itsSquasher;
      }
    }

    //===============>>> SquasherProcessControl::define  <<<==============================
    tribool SquasherProcessControl::define()
    {
      LOFAR::ACC::APS::ParameterSet* ParamSet = LOFAR::ACC::APS::globalParameterSet();
      itsInMS  = ParamSet->getString("inms");
      itsOutMS = ParamSet->getString("outms");
      itsStart = ParamSet->getBool("start");
      itsStep  = ParamSet->getBool("step");
      itsNChan = ParamSet->getBool("nchan");
      return true;
    }

    //===============>>> SquasherProcessControl::run  <<<=================================
    tribool SquasherProcessControl::run()
    {
      try{
        std::cout << "Runnning Data Squasher please wait..." << std::endl;
        MS_Reader.MS.deepCopy(itsOutMS);
        renameColumn("DATA", "OLDDATA");
        addColumn("DATA");

        itsSquasher->Squash("OLDDATA",
                            "DATA",
                            itsStart,
                            itsStep,
                            itsNchan);
        removeColumn("OLDDATA");
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
              string("Documentation can be found at: www.astron.nl/~renting\n");
      cout << itsMS << endl;
      if (itsInMS == "")
      {
        cerr  << " Error missing input" << endl;
        return false;
      }
      myMS       = new MS_File(itsMS);
      itsSquasher = new DataSquasher (myMS,
                                      itsWindow,
                                      itsCrosspol,
                                      itsMin,
                                      itsMax);
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
      if (myMS)
      {
        delete myMS;
        myMS = NULL;
      }
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
