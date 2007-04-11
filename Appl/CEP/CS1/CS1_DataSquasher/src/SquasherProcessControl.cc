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

#define SQUASHER_VERSION "0.10"

namespace LOFAR
{
  namespace CS1
  {
    using namespace casa;
    //===============>>> SquasherProcessControl::SquasherProcessControl  <<<===============
    SquasherProcessControl::SquasherProcessControl()
    : ProcessControl()
    {
      inMS        = NULL;
      itsSquasher = NULL;
    }

    //===============>>> SquasherProcessControl::~SquasherProcessControl  <<<==============
    SquasherProcessControl::~SquasherProcessControl()
    {
      if (inMS)
      { delete inMS;
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
      itsStart = ParamSet->getInt32("start");
      itsStep  = ParamSet->getInt32("step");
      itsNChan = ParamSet->getInt32("nchan");
      return true;
    }

    //===============>>> SquasherProcessControl::run  <<<=================================
    tribool SquasherProcessControl::run()
    {
      try{
        std::cout << "Creating " << itsOutMS << ", please wait..." << std::endl;
        inMS->deepCopy(itsOutMS, Table::NewNoReplace);
        MeasurementSet outMS = MeasurementSet(itsOutMS, Table::Update);
        TableDesc tdesc = outMS.tableDesc();
        ColumnDesc desc = tdesc.rwColumnDesc("DATA");
        IPosition pos = desc.shape();
        Vector<Int> temp = pos.asVector();
        std::cout << "Old shape: " << temp(0) << ":" <<  temp(1) << std::endl;
        temp(1) = itsNChan/itsStep;
        std::cout << "New shape: " << temp(0) << ":" <<  temp(1) << std::endl;
        outMS.renameColumn("OLDDATA", "DATA");
        IPosition pos2(temp);
        desc.setOptions(0);
        desc.setShape(pos2);
        desc.setOptions(4);
        outMS.addColumn(desc);

        itsSquasher->Squash(outMS,
                            "OLDDATA",
                            "DATA",
                            itsStart,
                            itsStep,
                            itsNChan);
        //outMS.removeColumn("OLDDATA");
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
              string("Documentation can be found at: www.lofar.org/wiki\n");
      cout << string("Squashing ") << itsInMS << string(" into ") << itsOutMS << endl;
      if (itsInMS == "" || itsOutMS == "")
      {
        cerr  << " Error missing input" << endl;
        return false;
      }
      inMS        = new MeasurementSet(itsInMS);
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
      if (inMS)
      {
        delete inMS;
        inMS = NULL;
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
