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
    itsID        (id)
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
  // Clean up map with Prediffer objects
  PrediffMap::iterator iter;
  for (iter=itsPrediffs.begin(); iter!=itsPrediffs.end(); iter++)
  {
    delete (*iter).second;
  }
  itsPrediffs.clear();

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

  cout << "!!!!!! Prediffer read workorder: " << endl;
  wo->dump();
  cout << "!!!!!!" << endl;

  // Update workorder status
  wo->setStatus(DH_WOPrediff::Assigned);
  woPtr->updateDB();

  DH_Prediff* dhRes = dynamic_cast<DH_Prediff*>(getDataManager().getOutHolder(1));
  //  dhRes->clearData();

  KeyValueMap args;
  vector<int> ant;
  vector<string> pNames;
  vector<int> peelSrcs;
  wo->getVarData(args, ant, pNames, peelSrcs);
  int contrID = wo->getStrategyControllerID();
  Prediffer* pred = getPrediffer(contrID, args, ant);

  // Execute workorder
  if (wo->getNewBaselines())
  {
    pred->select(ant, ant);
    vector<uint32> dataShape;
    dataShape = pred->setDomain(wo->getStartFreq(), wo->getFreqLength(), 
				wo->getStartTime(), wo->getTimeLength());
    dhRes->setDataSize(dataShape);              // Set data field of output DH_Prediff to correct size
    vector<int> emptyS(0);
    pred->setPeelSources(peelSrcs, emptyS);
    pred->clearSolvableParms();
    vector<string> emptyP(0);
    pred->setSolvableParms(pNames, emptyP, true);
    pred->updateSolvableParms();
    // Calculate and put in output dataholder buffer
    pred->getEquations(dhRes->getDataPtr(), dataShape);
    if (wo->getSubtractSources())
    {
      pred->subtractPeelSources(true);   // >>>For now: always write in new file 
    }
    dhRes->setParmData(pred->getSolvableParmData());    
  }
  else if (wo->getNewDomain())
  {
    vector<uint32> dataShape;
    dataShape = pred->setDomain(wo->getStartFreq(), wo->getFreqLength(), 
				wo->getStartTime(), wo->getTimeLength());
    dhRes->setDataSize(dataShape);              // Set data field of output DH_Prediff to correct size
    vector<int> emptyS(0);
    pred->setPeelSources(peelSrcs, emptyS);
    pred->clearSolvableParms();
    vector<string> emptyP(0);
    pred->setSolvableParms(pNames, emptyP, true);
    pred->updateSolvableParms();
    // Calculate and put in output dataholder buffer
    pred->getEquations(dhRes->getDataPtr(), dataShape);
    dhRes->setParmData(pred->getSolvableParmData());
  }
  else if (wo->getNewPeelSources())
  {
    vector<int> emptyS(0);
    pred->setPeelSources(peelSrcs, emptyS);
    pred->clearSolvableParms();
    vector<string> emptyP(0);
    pred->setSolvableParms(pNames, emptyP, true);
    pred->updateSolvableParms();
    // Calculate and put in output dataholder buffer
    pred->getEquations(dhRes->getDataPtr(), dhRes->getDataSize());
 }
  else
  {
    pred->updateSolvableParms();
    // Calculate and put in output dataholder buffer
    pred->getEquations(dhRes->getDataPtr(), dhRes->getDataSize());
  }
  if (wo->getSubtractSources())
  {
    pred->subtractPeelSources(true);   // >>>For now: always write in new file 
  }

  // send result to solver
  getDataManager().readyWithOutHolder(1);

  // Update workorder status
  wo->setStatus(DH_WOPrediff::Executed);
  woPtr->updateDB();

  if (wo->getCleanUp())   // If Prediffer (cache) is no longer needed: clean up  
  {
    itsPrediffs.erase(contrID);
  }


}

void WH_Prediff::dump()
{
  LOG_TRACE_RTTI("WH_Prediff process()");
}

Prediffer* WH_Prediff::getPrediffer(int id, const KeyValueMap& args, 
				 const vector<int>& antNrs)
{
  PrediffMap::iterator iter;
  iter = itsPrediffs.find(id);
  if (iter != itsPrediffs.end())
  {
    return (*iter).second;
  }
  else
  {
    // Create a Prediffer object
    string msName = args.getString("MSName", "empty") + ".MS";
    string meqModel = args.getString("meqTableName", "meqmodel");
    string skyModel = args.getString("skyTableName", "skymodel");
    string dbType = args.getString("DBType", "postgres");
    string dbName = args.getString("DBName", "test");
    string dbHost = args.getString("DBHost", "dop50");
    string dbPwd = args.getString("DBPwd", "");

    string modelType = args.getString("modelType", "LOFAR.RI");
    bool calcUVW = args.getBool("calcUVW", false);
    bool lockMappedMem = args.getBool("lockMappedMem", false);
    
    Prediffer* pred = new Prediffer(msName, meqModel, skyModel, dbType, 
				    dbName, dbHost, dbPwd, antNrs, 
				    modelType, calcUVW,lockMappedMem);
    // add to map
    itsPrediffs.insert(PrediffMap::value_type(id, pred));
    return pred;
  }
}



} // namespace LOFAR
