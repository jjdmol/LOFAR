//#  WH_Prediff.cc: predicts visibilities and determines the difference to
//#                 measured data
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBS3/WH_Prediff.h>
#include <Common/LofarLogger.h>
#include <BBS3/DH_WOPrediff.h>
#include <BBS3/DH_Prediff.h>
#include <BBS3/Prediffer.h>

namespace LOFAR
{

WH_Prediff::WH_Prediff(const string& name, int id)
  : WorkHolder   (1, 2, name, "WH_Prediff"),
    itsID        (id),
    itsPrediffer (0)
{
  LOG_TRACE_FLOW("WH_Prediff constructor");
  getDataManager().addInDataHolder(0, new DH_WOPrediff(name+"_in"));
  getDataManager().addOutDataHolder(0, new DH_WOPrediff(name+"_out0")); // dummy
  getDataManager().addOutDataHolder(1, new DH_Prediff(name+"_out1"));

  // switch input and output channel trigger off
  getDataManager().setAutoTriggerIn(0, false);
  getDataManager().setAutoTriggerOut(0, false);
  getDataManager().setAutoTriggerOut(1, false);
}

WH_Prediff::~WH_Prediff()
{
  LOG_TRACE_FLOW("WH_Prediff destructor");
  delete itsPrediffer;
}

WH_Prediff* WH_Prediff::make (const string& name)
{
  return new WH_Prediff (name, itsID);
}

void WH_Prediff::preprocess()
{
  LOG_TRACE_RTTI("WH_Prediff preprocess()");
}

void WH_Prediff::process()
{
  LOG_TRACE_RTTI("WH_Prediff process()");

  // Read workorder -> KSarguments and antennas
  // Query the database for a work order
  DH_WOPrediff* wo =  dynamic_cast<DH_WOPrediff*>(getDataManager().getInHolder(0));
  DH_PL* woPtr = dynamic_cast<DH_PL*>(wo);
 
  // Wait for workorder
  bool firstTime = true;
  while ((woPtr->queryDB("status=0 and (kstype='" + getName()
                         + "') order by woid asc")) <= 0)
  {
    if (firstTime)
    {
      cout << "No workorder found by " << getName() << ". Waiting for work order..." << endl;
      firstTime = false;
    }
  }

  // Update workorder status
  wo->setStatus(DH_WOPrediff::Assigned);
  woPtr->updateDB();

  DH_Prediff* dhRes = dynamic_cast<DH_Prediff*>(getDataManager().getOutHolder(0));
  //  dhRes->clearData();

  // Execute workorder
  if (wo->getInitialize())  // Initialize, nextInterval and getEquations (+send ParmData)
  {
    KeyValueMap args;
    vector<int> ant;
    vector<string> pNames;
    vector<int> peelSrcs;
    wo->getVarData(args, ant, pNames, peelSrcs);
    createPrediffer(args, ant);
    itsPrediffer->select(ant, ant, wo->getFirstChannel(), wo->getLastChannel());
    itsPrediffer->setTimeInterval(wo->getTimeInterval());
    itsPrediffer->clearSolvableParms();
    vector<string> emptyP(0);
    itsPrediffer->setSolvableParms(pNames, emptyP, true);
    vector<int> emptyS(0);
    itsPrediffer->peel(peelSrcs, emptyS);
    itsPrediffer->resetIterator();
    itsPrediffer->nextInterval();
    itsPrediffer->updateSolvableParms(); //????
    //    itsPrediffer->getEquations();        // returns a std::list<MeqResult>

    //    dhRes->setParmData(itsPrediffer->getSolvableParmData());
  }
  else if (wo->getNextInterval())  // nextInterval and getEquations (+send ParmData)
  {
    ASSERTSTR(itsPrediffer!=0, "The prediffer has not been created and initialized.");
    itsPrediffer->nextInterval();
    itsPrediffer->updateSolvableParms(); //????
    //    itsPrediffer->getEquations();

    //    itsPrediffer->getSolvableParmData();  // Returns a vector<ParmData>& -> set in dhRes
  }
  else                   // getEquations
  {
    ASSERTSTR(itsPrediffer!=0, "The prediffer has not been created and initialized.");
    //    itsPrediffer->updateSolvableParms(); //????
    //    itsPrediffer->getEquations();
  }

  // write result
}

void WH_Prediff::dump()
{
  LOG_TRACE_RTTI("WH_Prediff process()");
}

void WH_Prediff::createPrediffer(const KeyValueMap& args, const vector<int>& antNrs)
{
  // Create a Prediffer object
  string msName = args.getString("MSName", "empty") + ".MS";
  string meqModel = args.getString("meqTableName", "meqmodel");
  string skyModel = args.getString("skyTableName", "skymodel");
  string dbType = args.getString("DBType", "postgres");
  string dbName = args.getString("DBName", "test");
  string dbHost = args.getString("DBHost", "dop50");
  string dbPwd = args.getString("DBPwd", "");

  int ddid = args.getInt("ddid", 0);
  string modelType = args.getString("modelType", "LOFAR.RI");
  bool calcUVW = args.getBool("calcUVW", false);
  bool lockMappedMem = args.getBool("lockMappedMem", false);

  itsPrediffer = new Prediffer(msName, meqModel, skyModel, dbType, 
			       dbName, dbHost, dbPwd, ddid,
			       antNrs, modelType, calcUVW,
			       lockMappedMem);
}



} // namespace LOFAR
