//#  SC_Simple.cc:  The peeling calibration strategy
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

SC_Simple::SC_Simple(int id, DH_Solution* inDH, DH_WOPrediff* woPD,
		     DH_WOSolve* woSolve, const KeyValueMap& args)
  : StrategyController(id, inDH, woPD, woSolve),
    itsFirstCall      (true),
    itsPrevWOID       (0),
    itsArgs           (args),
    itsCurIter        (-1),
    itsCurStartTime   (0)
{
  itsNrIterations = itsArgs.getInt("nrIterations", 0);
}

SC_Simple::~SC_Simple()
{}

bool SC_Simple::execute()
{
  itsCurIter++;
  bool nextInter = false;
  if (itsFirstCall)
  {
    itsFirstCall = false;
    nextInter = true;
    itsWOPD->setNewBaselines(true);
    itsWOPD->setNewPeelSources(true);
    itsWOPD->setSubtractSources(false);
    itsWOSolve->setNewDomain(true);
    itsCurStartTime = itsArgs.getFloat ("startTime", 0);
  }
  else
  {
    // Read solution of previously issued workorders
    readSolution();

    itsWOPD->setNewBaselines(false);
    itsWOPD->setNewPeelSources(false);
    itsWOPD->setSubtractSources(false);
    itsWOSolve->setNewDomain(false);

    // If solution for this interval is good enough, go to next. TBA
    // For now: if number of iterations reached: go to next interval.
    if (fmod(itsCurIter, itsNrIterations) == 0)
    {
      nextInter = true;
      itsCurStartTime += itsArgs.getFloat ("timeInterval", 10);
    }
  }

  // Set prediffer workorder data
  itsWOPD->setStatus(DH_WOPrediff::New);
  itsWOPD->setKSType("Prediff1");
  itsWOPD->setStartFreq (itsArgs.getFloat ("startFreq", 0));
  itsWOPD->setFreqLength (itsArgs.getFloat ("freqLength", 0));
  itsWOPD->setStartTime (itsCurStartTime);
  float timeLength = itsArgs.getFloat ("timeLength", 10);
  itsWOPD->setTimeLength (timeLength);
  itsWOPD->setModelType (itsArgs.getString ("modelType", "notfound"));
  itsWOPD->setUseAutoCorrelations(itsArgs.getBool ("useAutoCorr", true));
  itsWOPD->setCalcUVW (itsArgs.getBool ("calcUVW", false));
  itsWOPD->setLockMappedMemory (itsArgs.getBool ("lockMappedMem", false));
  KeyValueMap msParams = (const_cast<KeyValueMap&>(itsArgs))["MSDBparams"].getValueMap();
  vector<int> ant = (const_cast<KeyValueMap&>(itsArgs))["antennas"].getVecInt();
  vector<string> pNames = (const_cast<KeyValueMap&>(itsArgs))["solvableParams"].getVecString();
  vector<int> srcs = (const_cast<KeyValueMap&>(itsArgs))["sources"].getVecInt();
  itsWOPD->setVarData (msParams, ant, pNames, srcs);

  itsWOPD->setNewWorkOrderID();
  itsWOPD->setStrategyControllerID(getID());
  itsWOPD->setNewDomain(nextInter);

  
  // Set solver workorder data  
  itsWOSolve->setStatus(DH_WOSolve::New);
  itsWOSolve->setKSType("Solver");
  itsWOSolve->setUseSVD (itsArgs.getBool ("useSVD", false));

  itsWOSolve->setNewWorkOrderID();
  itsPrevWOID = itsWOSolve->getWorkOrderID();  // Remember the issued workorder id
  itsWOSolve->setStrategyControllerID(getID());
  itsWOSolve->setNewDomain(nextInter);

  // Temporarily show on cout
  cout << "!!!!!!! Sent workorders: " << endl;
  itsWOPD->dump();
  itsWOSolve->dump();

  cout << "!!!!!!! " << endl;

  // Insert WorkOrders into database
  itsWOPD->insertDB();
  itsWOSolve->insertDB();

  return true;
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

  while (solPtr->queryDB(query) <= 0)
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
