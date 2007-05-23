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
#include <tables/Tables/TableParse.h>
#include <ms/MeasurementSets.h>
#include <casa/Exceptions.h>

#include <CS1_SPWCombine/CombinerProcessControl.h>
#include "SPWCombine.h"

#define COMBINER_VERSION "0.10"

namespace LOFAR
{
  namespace CS1
  {
    using namespace casa;
    //===============>>> CombinerProcessControl::CombinerProcessControl  <<<===============
    CombinerProcessControl::CombinerProcessControl()
    : ProcessControl()
    {
      itsCombiner = NULL;
    }

    //===============>>> CombinerProcessControl::~CombinerProcessControl  <<<==============
    CombinerProcessControl::~CombinerProcessControl()
    {
    }

    //===============>>> CombinerProcessControl::define  <<<==============================
    tribool CombinerProcessControl::define()
    {
      LOFAR::ACC::APS::ParameterSet* ParamSet = LOFAR::ACC::APS::globalParameterSet();
      itsInMS  = ParamSet->getStringVector("inms");
      itsOutMS = ParamSet->getString("outms");
      return true;
    }

    //===============>>> CombinerProcessControl::run  <<<=================================
    tribool CombinerProcessControl::run()
    {
      try{
        std::cout << "Creating " << itsOutMS << ", please wait..." << std::endl;
        Table TempTable = tableCommand(string("SELECT FROM ") + itsInMS[0] + string(" WHERE DATA_DESC_ID = 0"));
        TempTable.deepCopy(itsOutMS, Table::NewNoReplace, true);
        tableCommand(string("DELETE FROM ") + itsOutMS + string("/DATA_DESCRIPTION WHERE rownumber() > 1"));
        tableCommand(string("DELETE FROM ") + itsOutMS + string("/SPECTRAL_WINDOW WHERE rownumber() > 1"));
        MeasurementSet outMS = MeasurementSet(itsOutMS, Table::Update);
        int nchan = 0;
        for (int i = 0; i < itsInMS.size(); i++)
        {
          itsCombiner->GetMSInfo(*(inMS[i]));
          for (int j = 0; j < itsCombiner->itsNumBands; j++)
          { nchan += itsCombiner->itsNumChannels;
          }
        }
        TableDesc tdesc = outMS.tableDesc();
        ColumnDesc desc = tdesc.rwColumnDesc("DATA");
        IPosition pos = desc.shape();
        Vector<Int> temp = pos.asVector();
        temp(1) = nchan;
        std::cout << "New number of channels: " << nchan << std::endl;
        IPosition pos2(temp);
        desc.setOptions(0);
        desc.setShape(pos2);
        desc.setOptions(4);
        outMS.removeColumn("DATA");
        outMS.addColumn(desc);

        //fix the FLAGS column
        desc = tdesc.rwColumnDesc("FLAG");
        desc.setOptions(0);
        desc.setShape(pos2);
        desc.setOptions(4);
        outMS.removeColumn("FLAG");
        outMS.addColumn(desc);

        //Fix the SpectralWindow values
        MSSpectralWindow SPW = outMS.spectralWindow();
        ScalarColumn<Int> channum(SPW, "NUM_CHAN");
        channum.fillColumn(nchan);

        itsCombiner->Combine(inMS, outMS, "DATA");
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> CombinerProcessControl::init  <<<================================
    tribool CombinerProcessControl::init()
    {
      try {
      using std::cout;
      using std::cerr;
      using std::endl;

      cout  << string(COMBINER_VERSION) + string(" spw combine by Adriaan Renting for LOFAR CS1\n") +
              string("This is experimental software, please report errors or requests to renting@astron.nl\n") +
              string("Documentation can be found at: www.lofar.org/wiki\n");
      cout << string("Combining ");
      for (int i = 0; i < itsInMS.size(); i++)
      {
        cout << itsInMS[i] << ", ";
      }
      cout << string(" into ") << itsOutMS << endl;
      if (itsInMS[0] == "" || itsOutMS == "")
      {
        cerr  << " Error missing input" << endl;
        return false;
      }
      inMS.resize(itsInMS.size());
      for (int i = 0; i < itsInMS.size(); i++)
      {
        inMS[i] = new MeasurementSet(itsInMS[i]);
      }
      itsCombiner = new SPWCombine();
      }
      catch(casa::AipsError& err)
      {
        std::cerr << "Aips++ error detected: " << err.getMesg() << std::endl;
        return false;
      }
      return true;
    }

    //===============>>> CombinerProcessControl::pause  <<<===============================
    tribool CombinerProcessControl::pause(const std::string&)
    { return false;
    }

    //===============>>> CombinerProcessControl::quit  <<<================================
    tribool CombinerProcessControl::quit()
    {
      return true;
    }

    //===============>>> CombinerProcessControl::recover  <<<=============================
    tribool CombinerProcessControl::recover(const std::string&)
    { return false;
    }

    //===============>>> CombinerProcessControl::reinit  <<<==============================
    tribool CombinerProcessControl::reinit(const  std::string&)
    { return false;
    }

    //===============>>> CombinerProcessControl::askInfo  <<<=============================
    std::string CombinerProcessControl::askInfo(const std::string&)
    { return std::string("");
    }

    //===============>>> CombinerProcessControl::snapshot  <<<============================
    tribool CombinerProcessControl::snapshot(const std::string&)
    { return false;
    }
  } //namespace CS1
}; //namespace LOFAR
