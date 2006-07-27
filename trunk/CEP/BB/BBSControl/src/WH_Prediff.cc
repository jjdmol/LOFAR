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
#include <BBSControl/WH_Prediff.h>
#include <BBSControl/DH_WOPrediff.h>
#include <BBSControl/DH_Solution.h>
#include <BBSControl/DH_Prediff.h>
#include <BBS/Prediffer.h>
#include <BBS/MNS/MeqDomain.h>
#include <BBS/BBSTestLogger.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

namespace LOFAR
{
  namespace BBS
  {
    const uint woPrediffChanId = 0;
    const uint solutionChanId  = 1;
    const uint prediffChanId   = 2;

    WH_Prediff::WH_Prediff(const string& name, const string& id, 
			   const ParameterSet& pset)
      : WorkHolder   (2, 3, name, "WH_Prediff"),
	itsID        (id),
	itsArgs      (pset),
	itsFirstCall (true)
    {
      LOG_TRACE_FLOW("WH_Prediff constructor");
      getDataManager().addInDataHolder
	(woPrediffChanId, new DH_WOPrediff(name+"_in0"));
      getDataManager().addOutDataHolder
	(woPrediffChanId, new DH_WOPrediff(name+"_out0")); // dummy
      getDataManager().addInDataHolder
	(solutionChanId, new DH_Solution(name+"_in1"));
      getDataManager().addOutDataHolder
	(solutionChanId, new DH_Solution(name+"_out1")); // dummy
      getDataManager().addOutDataHolder
	(prediffChanId, new DH_Prediff(name+"_out2"));

      // switch input and output channel trigger off
      getDataManager().setAutoTriggerIn(woPrediffChanId, false);
      getDataManager().setAutoTriggerIn(solutionChanId, false);
      getDataManager().setAutoTriggerOut(woPrediffChanId, false);
      getDataManager().setAutoTriggerOut(solutionChanId, false);
      getDataManager().setAutoTriggerOut(prediffChanId, false);
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
      return new WH_Prediff (name, itsID, itsArgs);
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
      DH_WOPrediff* wo;
      ASSERT(wo = dynamic_cast<DH_WOPrediff*>
	     (getDataManager().getInHolder(woPrediffChanId)));

      if (wo->getDoNothing() == false)
      {
	DH_Prediff* dhRes;
	ASSERT(dhRes = dynamic_cast<DH_Prediff*>
	       (getDataManager().getOutHolder(prediffChanId)));

	//  dhRes->clearData();

	ParameterSet args;
	vector<int> ant;
	vector<string> pNames;
	vector<string> exPNames;
	vector<int> peelSrcs;
	vector<int> corrs;
	wo->getVarData(args, ant, pNames, exPNames, peelSrcs, corrs);
	int contrID = wo->getStrategyControllerID();
	bool isNew = false;
	Prediffer* pred = getPrediffer(contrID, args, ant, isNew);
    
	// Execute workorder
	if (wo->getWritePredData())
	{
	  pred->setWorkDomain(wo->getStartChannel(), wo->getEndChannel(), 
			      wo->getStartTime(), wo->getTimeLength());
	  pred->writePredictedData(wo->getWriteInDataCol() 
				   ? "DATA" : "CORRECTED_DATA");
	}
	else
	{
	  if (wo->getNewBaselines())
	  {
	    pred->select(ant, ant, wo->getUseAutoCorrelations(), corrs);
      
	    vector<int> emptyS(0);
	    if (peelSrcs.size() > 0)
	    {
	      pred->setPeelGroups(peelSrcs, emptyS);
	    }
	    pred->clearSolvableParms();
	    pred->setSolvableParms(pNames, exPNames);
	
	    pred->setWorkDomain(wo->getStartChannel(), wo->getEndChannel(), 
				wo->getStartTime(), wo->getTimeLength());
	  }
	  else if (wo->getNewDomain())
	  {
	    vector<int> emptyS(0);
	    if (peelSrcs.size() > 0)
	    {
	      pred->setPeelGroups(peelSrcs, emptyS);
	    }
	    pred->clearSolvableParms();
	    pred->setSolvableParms(pNames, exPNames);
      
	    pred->setWorkDomain(wo->getStartChannel(), wo->getEndChannel(), 
				wo->getStartTime(), wo->getTimeLength());
	  }
	  else if (wo->getNewPeelSources())
	  {
	    vector<int> emptyS(0);
	    if (peelSrcs.size() > 0)
	    {
	      pred->setPeelGroups(peelSrcs, emptyS);
	    }
	    pred->clearSolvableParms();
	    pred->setSolvableParms(pNames, exPNames);
	  }

	  // Loop over iterations
	  for (int curIter=0; curIter < wo->getMaxIterations(); curIter++)
	  {
	    bool converged = false;
	    // Parameter update
	    // If next iteration within the workorder, read solution
	    if (curIter != 0)
	    {
	      BBSTest::ScopedTimer updatePTimer("P:update-parms");
	      ParmDataInfo solVec;
	      converged = readSolution(wo->getSolutionID(), curIter-1, solVec);
	      if (readSolution(wo->getSolutionID(), curIter-1, solVec))
	      {
		break; // Skip prediffing and end iterating
	      }
	      else
	      {
		pred->updateSolvableParms(solVec);
	      }
	    }
	    else if (wo->getUpdateParms())  // Read solution
	    {
	      BBSTest::ScopedTimer updatePTimer("P:update-parms");
	      int solID = wo->getSolutionID();
	      if ((solID != -1))
	      {
		// Read solution and update parameters
		ParmDataInfo solVec;
		readSolution(solID, solVec);
		pred->updateSolvableParms(solVec);
	      }
	      else
	      {
		// Reread parameter values from table
		pred->updateSolvableParms();
	      }
	    }

	    // Calculate, put in output dataholder buffer and send to solver
	    {
	      BBSTest::ScopedTimer predifTimer("P:prediffer");
	      dhRes = dynamic_cast<DH_Prediff*>
		(getDataManager().getOutHolder(prediffChanId));
	      vector<casa::LSQFit> fitters;
	      pred->fillFitters (fitters, "DATA");
	      dhRes->setFitters (fitters);
	      MeqDomain domain = pred->getWorkDomain();
	      dhRes->setDomain (domain.startX(), domain.endX(),
				domain.startY(), domain.endY());
	      // send result to solver
	      getDataManager().readyWithOutHolder(prediffChanId);
	    }

	  } // End of loop over iterations


	  if (wo->getSubtractSources())
	  {
	    // >>>For now: always write in new file 
	    pred->subtractData("DATA", "CORRECTED_DATA");
	  }
      
	} // end else

      } // end if (wo->getDoNothing==false)


      // If Prediffer object is no longer needed: clean up  
      if (wo->getCleanUp())
      {
	int contrID = wo->getStrategyControllerID();
	deletePrediffer(contrID);
      }
      
      // Update workorder status
      wo->setStatus(DH_WOPrediff::Executed);
      Connection* conn = getDataManager().getInConnection(woPrediffChanId);
      ASSERTSTR(conn!=0, "No connection set!");
      BBSTest::ScopedTimer st("P:prediffer_updating_db");
      wo->updateDB(*conn);
    }

    void WH_Prediff::dump() const
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
	ParameterSet myargs = args;
	myargs.add ("DBIsMaster", "F");
	BBSTest::ScopedUSRTimer dbStartupTimer("C:DBStartup");
	dbStartupTimer.end();
  
	string modelType = args.getString("modelType");
	bool calcUVW = args.getBool("calcUVW");
	string msName = args.getString("subsetMSPath") + "/" + args.getString("MSName")+ "_p" + itsID;

	vector<vector<int> > srcgrp;
	getSrcGrp (args, srcgrp);
	Prediffer* pred = new Prediffer(msName,
					makePDM("meqTableName", myargs),
					makePDM("skyTableName", myargs),
					antNrs, modelType, srcgrp, calcUVW);
	// add to map
	itsPrediffs.insert(PrediffMap::value_type(id, pred));
	isNew = true;
	return pred;
      }
    }

    void WH_Prediff::deletePrediffer(int id)
    {
      PrediffMap::iterator iter;
      iter = itsPrediffs.find(id);
      if (iter != itsPrediffs.end())
      {
	delete iter->second;
	itsPrediffs.erase(iter);
      }
    }

    void WH_Prediff::readWorkOrder()
    {
      // Query the database for a work order
      DH_WOPrediff* wo = dynamic_cast<DH_WOPrediff*>
	(getDataManager().getInHolder(woPrediffChanId));
      Connection* conn = getDataManager().getInConnection(woPrediffChanId);
      ASSERTSTR(conn!=0, "No connection set!");
 
      // Wait for workorder
      bool firstTime = true;
      ostringstream q;
      q << "SELECT * FROM bbs3woprediffer WHERE STATUS=0 AND(KSTYPE='" 
	<< getName() << "') order by WOID ASC";

      while ((wo->queryDB(q.str(), *conn)) <= 0)
      {
	if (firstTime)
	{
	  cout << "No workorder found by " << getName() 
	       << ". Waiting for work order..." << endl;
	  firstTime = false;
	}
      }

      //  cout << "!!!!!! Prediffer read workorder: " << endl;
      //wo->dump();
      //  cout << "!!!!!!" << endl;

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
	// each string in this vector holds a vector of source numbers
	// (integers) in the format srcs=[0,1,2]
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

    void WH_Prediff::readSolution(int woid, ParmDataInfo& solVec)
    {
      LOG_TRACE_FLOW("WH_Prediff reading solution");

      DH_Solution* sol = dynamic_cast<DH_Solution*>
	(getDataManager().getInHolder(solutionChanId));
      Connection* conn = getDataManager().getInConnection(solutionChanId);
      ASSERTSTR(conn!=0, "No connection set!");

      // Wait for solution
      char str[64];
      sprintf(str, "SELECT * FROM bbs3solutions WHERE WOID=%i "
	      "ORDER BY iteration DESC", woid);
      string query(str);

      bool firstTime = true;
      while ((sol->queryDB(query, *conn)) <= 0)
      {
	if (firstTime)
	{
	  cout << "No solution with WOID = " << woid 
	       << " found by WH_Prediff. Waiting for solution..." << endl;
	  firstTime = false;
	}
      }
  
      sol->getSolution(solVec);
    }

    bool WH_Prediff::readSolution(int woid, int iteration, ParmDataInfo& solVec)
    {
      LOG_TRACE_FLOW("WH_Prediff reading solution");

      DH_Solution* sol = dynamic_cast<DH_Solution*>
	(getDataManager().getInHolder(solutionChanId));
      Connection* conn = getDataManager().getInConnection(solutionChanId);
      ASSERTSTR(conn!=0, "No connection set!");

      // Wait for solution
      char str[64];
      sprintf(str, "SELECT * FROM bbs3solutions WHERE WOID=%i "
	      "AND iteration=%i", woid, iteration);
      string query(str);

      bool firstTime = true;
      while ((sol->queryDB(query, *conn)) <= 0)
      {
	if (firstTime)
	{
	  cout << "No solution with WOID = " << woid << " and ITERATION= " 
	       << iteration << " found by WH_Prediff. Waiting for solution..."
	       << endl;
	  firstTime = false;
	}
      }
  
      sol->getSolution(solVec);

      return (sol->hasConverged());
    }

    ParmDB::ParmDBMeta WH_Prediff::makePDM (const string& nameKey,
					    const ParameterSet& ps)
    {
      string type = ps.getString("DBType");
      string tableName = ps.getString(nameKey);
      ParmDB::ParmDBMeta pdm(type, tableName);

      if (toLower(type) != "aips") {
	// extract all information for other types
	string dbName = ps.getString("DBName");
	string userName = ps.getString("DBUserName");
	string dbPwd = ps.getString("DBPwd");
	string hostName = ps.getString("DBMasterHost");
	pdm.setSQLMeta (dbName, userName, dbPwd, hostName);
      }
      return pdm;
    }

  } // namespace BBS

} // namespace LOFAR
