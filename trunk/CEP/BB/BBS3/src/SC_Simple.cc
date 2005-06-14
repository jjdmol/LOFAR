//#  SC_Simple.cc:  A simple calibration strategy
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>

#include <BBS3/SC_Simple.h>
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>
#include <TransportPL/DH_PL.h>
#include <BBS3/DH_Solution.h>
#include <BBS3/DH_WOPrediff.h>
#include <BBS3/DH_WOSolve.h>

namespace LOFAR
{

SC_Simple::SC_Simple(int id, Connection* inSolConn, Connection* outWOPDConn, 
		     Connection* outWOSolveConn, int nrPrediffers,
		     const KeyValueMap& args)
  : StrategyController(id, inSolConn, outWOPDConn, outWOSolveConn, 
		       nrPrediffers, args.getInt("DBMasterPort", 13157)),
    itsFirstCall      (true),
    itsPrevWOID       (0),
    itsArgs           (args),
    itsCurIter        (-1),
    itsCurStartTime   (0),
    itsControlParmUpd (false),
    itsStartTime      (0),
    itsTimeLength     (0),
    itsStartFreq      (0),
    itsFreqLength     (0)
{
  itsNrIterations = itsArgs.getInt("nrIterations", 0);
  KeyValueMap msParams = (const_cast<KeyValueMap&>(itsArgs))["MSDBparams"].getValueMap();
  string dbType = msParams.getString("DBType", "notfound");
  string meqTableName = msParams.getString("meqTableName", "notfound");
  string skyTableName = msParams.getString("skyTableName", "notfound");
  string dbName = msParams.getString("DBName", "notfound");
  string dbPwd = msParams.getString("DBPwd", "");
  string dbHost = msParams.getString("DBHost", "");

  itsControlParmUpd = itsArgs.getBool ("controlParmUpdate", false);
  itsStartTime = itsArgs.getFloat ("startTime", 0);
  itsTimeLength = itsArgs.getFloat ("timeLength", 0);
  itsStartFreq = itsArgs.getFloat ("startFreq", 0);
  itsFreqLength = itsArgs.getFloat ("freqLength", 0);
}

SC_Simple::~SC_Simple()
{}

bool SC_Simple::execute()
{
  DH_WOPrediff* WOPD = getPrediffWorkOrder();
  DH_WOSolve* WOSolve = getSolveWorkOrder();
  itsCurIter++;
  bool nextInter = false;
  if (itsFirstCall)
  {
    itsFirstCall = false;
    nextInter = true;
    WOPD->setNewBaselines(true);
    WOPD->setNewPeelSources(true);
    WOPD->setSubtractSources(false);
    WOPD->setUpdateParms(false);
    WOSolve->setNewDomain(true);
    itsCurStartTime = itsArgs.getFloat ("startTime", 0);
  }
  else
  {
    // Read solution of previously issued workorders
    readSolution();

    WOPD->setNewBaselines(false);
    WOPD->setNewPeelSources(false);
    WOPD->setSubtractSources(false);
    WOPD->setUpdateParms(true);
    WOSolve->setNewDomain(false);

    if (itsControlParmUpd)   // If Controller handles parameter writing
    {
      // Controller writes new parameter values directly to the tables
      vector<ParmData> pData;
      getSolution()->getSolution(pData);
      double fStart = getSolution()->getStartFreq();
      double fEnd = getSolution()->getEndFreq();
      double tStart = getSolution()->getStartTime();
      double tEnd = getSolution()->getEndTime();
      getParmWriter().write(pData, fStart, fEnd, tStart, tEnd);
    }
    else
    {
      // Send the (reference to) parameter values to Prediffers.
      WOPD->setSolutionID(itsPrevWOID);
    }

    // If solution for this interval is good enough, go to next. TBA
    // For now: if number of iterations reached: go to next interval.
    if (fmod(itsCurIter, itsNrIterations) == 0)
    {
      nextInter = true;
      itsCurStartTime += itsArgs.getFloat ("timeLength", 10);
    }
  }

  // Set prediffer workorder data
  WOPD->setStatus(DH_WOPrediff::New);
  WOPD->setKSType("Prediff1");
  WOPD->setStartFreq (itsArgs.getFloat ("startFreq", 0));
  WOPD->setFreqLength (itsArgs.getFloat ("freqLength", 0));
  WOPD->setStartTime (itsCurStartTime);
  float timeLength = itsArgs.getFloat ("timeLength", 10);
  WOPD->setTimeLength (timeLength);
  WOPD->setModelType (itsArgs.getString ("modelType", "notfound"));
  WOPD->setUseAutoCorrelations(itsArgs.getBool ("useAutoCorr", true));
  WOPD->setCalcUVW (itsArgs.getBool ("calcUVW", false));
  KeyValueMap msParams = (const_cast<KeyValueMap&>(itsArgs))["MSDBparams"].getValueMap();
  vector<int> ant = (const_cast<KeyValueMap&>(itsArgs))["antennas"].getVecInt();
  vector<string> pNames = (const_cast<KeyValueMap&>(itsArgs))["solvableParams"].getVecString();
  vector<int> srcs = (const_cast<KeyValueMap&>(itsArgs))["sources"].getVecInt();
  WOPD->setVarData (msParams, ant, pNames, srcs);

  WOPD->setNewWorkOrderID();
  WOPD->setStrategyControllerID(getID());
  WOPD->setNewDomain(nextInter);

  
  // Set solver workorder data  
  WOSolve->setStatus(DH_WOSolve::New);
  WOSolve->setKSType("Solver");
  WOSolve->setUseSVD (itsArgs.getBool ("useSVD", false));

  WOSolve->setNewWorkOrderID();
  itsPrevWOID = WOSolve->getWorkOrderID();  // Remember the issued workorder id
  WOSolve->setStrategyControllerID(getID());
  WOSolve->setNewDomain(nextInter);

  // Temporarily show on cout
  cout << "!!!!!!! Sent workorders: " << endl;
  WOPD->dump();
  WOSolve->dump();

  cout << "!!!!!!! " << endl;

  // Insert WorkOrders into database
  WOPD->insertDB(*itsOutWOPDConn);

  // Send workorders the same workorders to other prediffers (if there are more than 1)
  int nrPred = getNumberOfPrediffers();
  for (int i = 2; i <= nrPred; i++)
  {
    WOPD->setNewWorkOrderID();
    char str[32];
    sprintf(str, "%i", i);
    WOPD->setKSType("Prediff"+string(str));
    WOPD->insertDB(*itsOutWOPDConn);
  }

  WOSolve->insertDB(*itsOutWOSolveConn);

  return true;
}

void SC_Simple::postprocess()
{
  bool writeParms = itsArgs.getBool("writeParms", false);
  if (writeParms)
  {
    readSolution();
    // Controller writes found parameter values in the tables
    vector<ParmData> pData;
    getSolution()->getSolution(pData);
    double fStart = getSolution()->getStartFreq();
    double fEnd = getSolution()->getEndFreq();
    double tStart = getSolution()->getStartTime();
    double tEnd = getSolution()->getEndTime();
    getParmWriter().write(pData, fStart, fEnd, tStart, tEnd);
  }
}

void SC_Simple::readSolution()
{
  LOG_TRACE_FLOW("SC_Simple reading solution");

  DH_PL* solPtr = dynamic_cast<DH_PL*>(getSolution());

  // Wait for solution
  bool firstTime = true;
  int id = itsPrevWOID;
  char str[32];
  sprintf(str, "WOID=%i", id);
  string query(str);

  while (solPtr->queryDB(query, *itsInSolConn) <= 0)
  {
    if (firstTime)
    {
      cout << "No solution found by SC_Simple " << getID() 
	   << ". Waiting for solution..." << endl;
      firstTime = false;
    }
  }

  getSolution()->dump();

}

} // namespace LOFAR
