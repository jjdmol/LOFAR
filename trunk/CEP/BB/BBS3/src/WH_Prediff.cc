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
#include <BBS3/DH_Solution.h>
#include <BBS3/DH_Prediff.h>
#include <BBS3/Prediffer.h>
#include <BBS3/MNS/MeqDomain.h>

namespace LOFAR
{

WH_Prediff::WH_Prediff(const string& name, int id)
  : WorkHolder   (2, 3, name, "WH_Prediff"),
    itsID        (id),
    itsFirstCall (true)
{
  LOG_TRACE_FLOW("WH_Prediff constructor");
  getDataManager().addInDataHolder(0, new DH_WOPrediff(name+"_in0"));
  getDataManager().addOutDataHolder(0, new DH_WOPrediff(name+"_out0")); // dummy
  getDataManager().addInDataHolder(1, new DH_Solution(name+"_in1"));
  getDataManager().addOutDataHolder(1, new DH_Solution(name+"_out1")); // dummy
  getDataManager().addOutDataHolder(2, new DH_Prediff(name+"_out2"));

  // switch input and output channel trigger off
  getDataManager().setAutoTriggerIn(0, false);
  getDataManager().setAutoTriggerIn(1, false);
  getDataManager().setAutoTriggerOut(0, false);
  getDataManager().setAutoTriggerOut(1, false);
  getDataManager().setAutoTriggerOut(2, false);
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

  // Read next workorder
  readWorkOrder();

  DH_WOPrediff* wo =  dynamic_cast<DH_WOPrediff*>(getDataManager().getInHolder(0));

  DH_Prediff* dhRes = dynamic_cast<DH_Prediff*>(getDataManager().getOutHolder(2));
  //  dhRes->clearData();

  ParameterSet args;
  vector<int> ant;
  vector<string> pNames;
  vector<int> peelSrcs;
  wo->getVarData(args, ant, pNames, peelSrcs);
  int contrID = wo->getStrategyControllerID();
  bool isNew = false;
  Prediffer* pred = getPrediffer(contrID, args, ant, isNew);

  // Execute workorder
  if (wo->getNewBaselines())
  {
    vector<int> corr;
    pred->select(ant, ant, wo->getUseAutoCorrelations(), corr);

    vector<int> emptyS(0);
    if (peelSrcs.size() > 0)
    {
      pred->setPeelGroups(peelSrcs, emptyS);
    }
    pred->clearSolvableParms();
    vector<string> emptyP(0);
    pred->setSolvableParms(pNames, emptyP, true);

    int size = pred->setDomain(wo->getStartFreq(), wo->getFreqLength(), 
			       wo->getStartTime(), wo->getTimeLength());
   
    dhRes->setBufferSize(size);              // Set data field of output DH_Prediff to correct size

    dhRes->setParmData(pred->getSolvableParmData());   
  }
  else if (wo->getNewDomain())
  {
    vector<int> emptyS(0);
    if (peelSrcs.size() > 0)
    {
      pred->setPeelGroups(peelSrcs, emptyS);
    }
    pred->clearSolvableParms();
    vector<string> emptyP(0);
    pred->setSolvableParms(pNames, emptyP, true);

    int size = pred->setDomain(wo->getStartFreq(), wo->getFreqLength(), 
			       wo->getStartTime(), wo->getTimeLength());
    dhRes->setBufferSize(size);      // Set data field of output DH_Prediff to correct size

    dhRes->setParmData(pred->getSolvableParmData());
  }
  else if (wo->getNewPeelSources())
  {
    vector<int> emptyS(0);
    if (peelSrcs.size() > 0)
    {
      pred->setPeelGroups(peelSrcs, emptyS);
    }
    pred->clearSolvableParms();
    vector<string> emptyP(0);
    pred->setSolvableParms(pNames, emptyP, true);
  }

  // Parameter update
  if (wo->getUpdateParms())
  {
    int solID = wo->getSolutionID();
    if (solID != -1)             // Read solution and update parameters
    {
      vector<ParmData> solVec;
      readSolution(solID, solVec);
      pred->updateSolvableParms(solVec);
    }
    else
    {                            // Reread parameter values from table
      pred->updateSolvableParms();
    }
  }

  // Calculate, put in output dataholder buffer and send to solver
  {
    dhRes = dynamic_cast<DH_Prediff*>(getDataManager().getOutHolder(2));
    casa::LSQFit fitter;
    pred->fillFitter (fitter);
    Prediffer::marshall (fitter, dhRes->getDataBuffer(),
			 dhRes->getBufferSize());
    MeqDomain domain = pred->getDomain();
    dhRes->setDomain(domain.startX(), domain.endX(), domain.startY(), 
		     domain.endY());
    // send result to solver
    getDataManager().readyWithOutHolder(2);
  }

  if (wo->getSubtractSources())
  {
    pred->subtractPeelSources(true);   // >>>For now: always write in new file 
  }

  // Update workorder status
  wo->setStatus(DH_WOPrediff::Executed);
  Connection* conn = getDataManager().getInConnection(0);
  ASSERTSTR(conn!=0, "No connection set!");
  wo->updateDB(*conn);

  if (wo->getCleanUp())   // If Prediffer (cache) is no longer needed: clean up  
  {
    itsPrediffs.erase(contrID);
  }


}

void WH_Prediff::dump()
{
  LOG_TRACE_RTTI("WH_Prediff process()");
}

Prediffer* WH_Prediff::getPrediffer(int id, const ParameterSet& args, 
				    const vector<int>& antNrs, bool& isNew)
{
  PrediffMap::iterator iter;
  iter = itsPrediffs.find(id);
  if (iter != itsPrediffs.end())
  {
    isNew = false;
    return (*iter).second;
  }
  else
  {
    // Create a Prediffer object
    string msName = args.getString("MSName");
    string meqModel = args.getString("meqTableName");
    string skyModel = args.getString("skyTableName");
    string dbType = args.getString("DBType");
    string dbName = args.getString("DBName");
    string dbHost = args.getString("DBHost");
    string dbPwd = args.getString("DBPwd");
    int dbMasterPort = args.getInt32("DBMasterPort");
    int dbMyPort = dbMasterPort + id;

    string modelType = args.getString("modelType");
    bool calcUVW = args.getBool("calcUVW");
    vector<vector<int> > srcgrp;
    getSrcGrp (args, srcgrp);
    Prediffer* pred = new Prediffer(msName, meqModel, skyModel, dbType, 
				    dbName, dbHost, dbPwd, dbMasterPort, dbMyPort,
				    antNrs, modelType, srcgrp, calcUVW);
    // add to map
    itsPrediffs.insert(PrediffMap::value_type(id, pred));
    isNew = true;
    return pred;
  }
}

void WH_Prediff::readWorkOrder()
{
  // Query the database for a work order
  DH_WOPrediff* wo =  dynamic_cast<DH_WOPrediff*>(getDataManager().getInHolder(0));
  Connection* conn = getDataManager().getInConnection(0);
  ASSERTSTR(conn!=0, "No connection set!");
 
  // Wait for workorder
  bool firstTime = true;
  while ((wo->queryDB("status=0 and (kstype='" + getName() + "') order by woid asc", *conn)) <= 0)
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
  wo->updateDB(*conn);
}

void WH_Prediff::getSrcGrp (const ParameterSet& args,
			    vector<vector<int> >& srcgrp) const
{
  srcgrp.resize (0);
  if (args.isDefined("sourceGroups")) {
    vector<string> groupVectors = args.getStringVector("sourceGroups");
    // each string in this vector holds a vector of source numbers (integers) in the format srcs=[0,1,2]
    vector<string>::iterator git = groupVectors.begin();
    for (; git != groupVectors.end(); git++) {
      ParameterSet srcVectorSet;
      // read the parameterSet from the string
      srcVectorSet.adoptBuffer(*git);
      // get the sources in this group
      srcgrp.push_back(srcVectorSet.getInt32Vector("srcs"));
    }
  }
}

void WH_Prediff::readSolution(int id, vector<ParmData>& solVec)
{
  LOG_TRACE_FLOW("WH_Prediff reading solution");

  DH_Solution* sol = dynamic_cast<DH_Solution*>(getDataManager().getInHolder(1));
  Connection* conn = getDataManager().getInConnection(1);
  ASSERTSTR(conn!=0, "No connection set!");

  // Wait for solution
  char str[32];
  sprintf(str, "WOID=%i", id);
  string query(str);

  ASSERTSTR(sol->queryDB(query, *conn) > 0, "No solution with WOID = "  
	    << id << " found by WH_Prediff");
  
  sol->getSolution(solVec);
}

} // namespace LOFAR
