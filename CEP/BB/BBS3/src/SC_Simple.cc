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
    itsWOID           (0),
    itsArgs           (args),
    itsCurIter        (-1)
{
  itsNrIterations = itsArgs.getInt("nrIterations", 0);
}

SC_Simple::~SC_Simple()
{}

bool SC_Simple::execute()
{
  itsWOID++;
  itsCurIter++;
  if (itsFirstCall)
  {
    itsFirstCall = false;
    // Set prediffer workorder data
    itsWOPD->setInitialize(true);
    itsWOPD->setWorkOrderID(itsWOID);
    itsWOPD->setStatus(DH_WOPrediff::New);
    itsWOPD->setKSType("Prediff1");
    itsWOPD->setFirstChannel (itsArgs.getInt ("startChan", 0));
    itsWOPD->setLastChannel (itsArgs.getInt ("endChan", 0));
    float timeInterval = itsArgs.getFloat ("timeInterval", 10);
    itsWOPD->setTimeInterval (timeInterval);
    itsWOPD->setDDID (itsArgs.getInt ("ddid", 0));
    itsWOPD->setModelType (itsArgs.getString ("modelType", "notfound"));
    itsWOPD->setCalcUVW (itsArgs.getBool ("calcUVW", false));
    itsWOPD->setLockMappedMemory (itsArgs.getBool ("lockMappedMem", false));
    KeyValueMap msParams = (const_cast<KeyValueMap&>(itsArgs))["MSDBparams"].getValueMap();
    vector<int> ant = (const_cast<KeyValueMap&>(itsArgs))["antennas"].getVecInt();
    vector<string> pNames = (const_cast<KeyValueMap&>(itsArgs))["solvableParams"].getVecString();
    vector<int> srcs = (const_cast<KeyValueMap&>(itsArgs))["sources"].getVecInt();
    itsWOPD->setVarData (msParams, ant, pNames, srcs);

    // Set solver workorder data
    itsWOSolve->setInitialize(true);
    itsWOSolve->setWorkOrderID(itsWOID);
    itsWOSolve->setStatus(DH_WOSolve::New);
    itsWOSolve->setKSType("Solver");
    itsWOSolve->setUseSVD (itsArgs.getBool ("useSVD", false)); 
    itsWOSolve->setVarData (msParams, timeInterval, pNames);

  }
  else
  {
    readSolution();
    itsWOPD->setInitialize(false);
    itsWOSolve->setInitialize(false);
  }

  // If solution for this interval is good enough, go to next. TBA
  // For now: if number of iterations reached: go to next interval.
  bool nextInter = false;
  if (fmod(itsCurIter, itsNrIterations) == 0)
  {
    nextInter = true;
  }

  itsWOSolve->setNextInterval(nextInter);
  itsWOPD->setNextInterval(nextInter);

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
  while (solPtr->queryDB("woid=" + itsWOID) <= 0)
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
