//# Solver.cc: Calculate parameter values using a least squares fitter
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>

#include <BBS3/Solver.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/Timer.h>

#include <Common/BlobIStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/BlobArray.h>

#include <iostream>
#include <fstream>
#include <stdexcept>

namespace LOFAR
{

//----------------------------------------------------------------------
//
// ~Solver
//
// Constructor. Initialize a Solver object.
//
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the WSRT.
//
//----------------------------------------------------------------------
Solver::Solver (const string& meqModel,
		const string& skyModel,
		const string& dbType,
		const string& dbName,
		const string& dbHost,
		const string& dbPwd)
  :
  itsMEPName      (meqModel),
  itsMEP          (dbType, meqModel, dbName, dbPwd, dbHost),
  itsGSMMEPName   (skyModel),
  itsGSMMEP       (dbType, skyModel, dbName, dbPwd, dbHost),
  itsSolver       (1)
{
  LOG_INFO_STR( "Solver constructor ("
		<< "'" << meqModel << "', "
		<< "'" << skyModel << "')");

  itsMEP.unlock();
  itsGSMMEP.unlock();

}

//----------------------------------------------------------------------
//
// ~~Solver
//
// Destructor for a Solver object.
//
//----------------------------------------------------------------------
Solver::~Solver()
{
  LOG_TRACE_FLOW( "Solver destructor" );
}


//----------------------------------------------------------------------
//
// ~setTimeInterval
//
// Set the time domain (interval) for which the solver will solve.
// The predict could be on a smaller domain (but not larger) than
// this domain.
//
//----------------------------------------------------------------------
void Solver::setTimeInterval (double secInterval)
{
  LOG_INFO_STR("setTimeInterval = " << secInterval);
  itsTimeInterval = secInterval;
}

//----------------------------------------------------------------------
//
// ~resetIterator
//
// Start iteration over time domains from the beginning.
//
//----------------------------------------------------------------------
void Solver::resetIterator()
{
  LOG_TRACE_FLOW( "resetTimeIterator" );
  //  itsTimeIndex = 0;
}

//----------------------------------------------------------------------
//
// ~nextInterval
//
// Move to the next interval (domain).
// Set the request belonging to that.
//
//----------------------------------------------------------------------
bool Solver::nextInterval(bool callReadPolcs)
{
  cout << "BBSTest: EndOfInterval" << endl;

//   // Exit when no more chunks.
//   if (itsTimeIndex >= itsTimes.nelements()) {
//     return false;
//   }
//   cout << "BBSTest: BeginOfInterval" << endl;
  
//   double timeSize = 0;
//   double timeStart = 0;
//   double timeStep = 0;
//   itsNrTimes = 0;
//   // Get the next chunk until the time interval size is exceeded.
//   while (timeSize < itsTimeInterval  &&  itsTimeIndex < itsTimes.nelements())
//   {
//       // If first time, calculate interval and start time.
//     if (timeStart == 0) {
//       timeStep  = itsIntervals[itsTimeIndex];
//       timeStart = itsTimes[itsTimeIndex] - timeStep/2;
//     }
//     timeSize += timeStep;
//     itsNrTimes++;
//     itsTimeIndex++;
//   }
  
//   // Map the correct data subset (this time interval)
//   long long startOffset = (itsTimeIndex-itsNrTimes)*itsNrBl*itsNrChan*itsNPol*sizeof(fcomplex);
//   size_t nrBytes = itsNrTimes*itsNrBl*itsNrChan*itsNPol*sizeof(fcomplex);
//   // Map this time interval
//   itsDataMap->mapFile(startOffset, nrBytes); 
//   if (itsLockMappedMem)
//   {                                             // Make sure mapped data is resident in RAM
//     itsDataMap->lockMappedMemory();
//   }

//   mapTimer.stop();
//   cout << "BBSTest: file-mapping " << mapTimer << endl;

//   NSTimer parmTimer;
//   parmTimer.start();
//   itsSolveDomain = MeqDomain(timeStart, timeStart + itsNrTimes*timeStep,
// 			     itsStartFreq + itsFirstChan*itsStepFreq,
// 			     itsStartFreq + (itsLastChan+1)*itsStepFreq);
//   initParms (itsSolveDomain, callReadPolcs);
//   parmTimer.stop();
//   cout << "BBSTest: initparms    " << parmTimer << endl;

// //   itsSolveColName = itsDataColName;
//   return true;
}


//----------------------------------------------------------------------
//
// ~clearSolvableParms
//
// Clear the solvable flag on all parms (make them non-solvable).
//
//----------------------------------------------------------------------
// void Solver::clearSolvableParms()
// {
//   const vector<MeqParm*>& parmList = MeqParm::getParmList();

//   LOG_TRACE_FLOW( "clearSolvableParms" );
//   itsSolvableParms.resize(0);

//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     if (*iter)
//     {
//       //cout << "clearSolvable: " << (*iter)->getName() << endl;
//       (*iter)->setSolvable(false);
//     }
//   }
// }

//----------------------------------------------------------------------
//
// ~setSolvableParms
//
// Set the solvable flag (true or false) on all parameters whose
// name matches the parmPatterns pattern.
//
//----------------------------------------------------------------------
// void Solver::setSolvableParms (vector<string>& parms,
// 				      vector<string>& excludePatterns,
// 				      bool isSolvable)
// {
//   itsSolvableParms.resize(parms.size());
//   for (unsigned int i = 0; i < parms.size(); i++)
//   {
//     itsSolvableParms[i] = parms[i];
//   }

//   const vector<MeqParm*>& parmList = MeqParm::getParmList();

//   LOG_INFO_STR( "setSolvableParms: "
// 		<< "isSolvable = " << isSolvable);

//   // Convert patterns to regexes.
//   vector<Regex> parmRegex;
//   for (unsigned int i=0; i<itsSolvableParms.nelements(); i++) {
//     parmRegex.push_back (Regex::fromPattern(itsSolvableParms[i]));
//   }

//   vector<Regex> excludeRegex;
//   for (unsigned int i=0; i<excludePatterns.size(); i++) {
//     excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
//   }

//   //
//   // Find all parms matching the itsSolvableParms
//   // Exclude them if matching an excludePattern
//   //
//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     String parmName ((*iter)->getName());

//     for (vector<Regex>::const_iterator incIter = parmRegex.begin();
// 	 incIter != parmRegex.end();
// 	 incIter++)
//     {
//       {
// 	if (parmName.matches(*incIter))
// 	{
// 	  bool parmExc = false;
// 	  for (vector<Regex>::const_iterator excIter = excludeRegex.begin();
// 	       excIter != excludeRegex.end();
// 	       excIter++)
// 	  {
// 	    if (parmName.matches(*excIter))
// 	    {
// 	      parmExc = true;
// 	      break;
// 	    }
// 	  }
// 	  if (!parmExc) {
// 	    LOG_TRACE_OBJ_STR( "setSolvable: " << (*iter)->getName());
// 	    (*iter)->setSolvable(isSolvable);
// 	  }
// 	  break;
// 	}
//       }
//     }
//   }
// }


//----------------------------------------------------------------------
//
// ~solve
//
// Solve for the solvable parameters on the current time domain.
// Returns solution and fitness.
//
//----------------------------------------------------------------------

void Solver::solve(bool useSVD, vector<string>& resultParmNames, 
			  vector<double>& resultParmValues,
			  Quality& resultQuality)
{
  //  LOG_INFO_STR( "solve using file " << itsDataMap->getFileName());

//   Timer timer;
//   NSTimer solveTimer, totTimer;
//   totTimer.start();
//   if (itsNrScid == 0) {
//     throw AipsError ("No parameters are set to solvable");
//   }
//   int nrpoint = 0;

//   double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
//   double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
//   int nrchan = 1+itsLastChan-itsFirstChan;

//   bool showd = false;
  
//   // Complex values are separated in real and imaginary.
//   // Loop over all times in current time interval.
//   for (unsigned int tStep=0; tStep<itsNrTimes; tStep++)
//   {
//     unsigned int timeOffset = tStep*itsNrBl*itsNrChan*itsNPol;
//     double time = itsTimes[itsTimeIndex-itsNrTimes+tStep];
//     double interv = itsIntervals[itsTimeIndex-itsNrTimes+tStep];
 
//     MeqDomain domain(time-interv/2, time+interv/2, startFreq, endFreq);
//     MeqRequest request(domain, 1, nrchan, itsNrScid);

//     for (unsigned int bl=0; bl<itsNrBl; bl++)
//     {
//       uInt ant1 = itsAnt1Data(bl);
//       uInt ant2 = itsAnt2Data(bl);
//       if (itsBLSelection(ant1,ant2) == true)
//       {
//        // Get condition equations (measured-predicted and all derivatives).
//         vector<MeqResult> res = getCondeq (request, bl, ant1, ant2, showd);
//         // Add them all to the solver.
//       } // Matches: if (itsBLSelection== )
//     } // End loop baselines
//   } // End loop timesteps


//   // The actual solve
//   //  if (Debug(1)) timer.show("fill ");

//   ASSERT (nrpoint >= itsNrScid);
//   // Solve the equation. 
//   uInt rank;
//   double fit;

//   LOG_INFO_STR( "Solution before: " << itsSolution);
//   //  cout << "Solution before: " << itsSolution << endl;
//   // It looks as if LSQ has a bug so that solveLoop and getCovariance
//   // interact badly (maybe both doing an invert).
//   // So make a copy to separate them.
//   Matrix<double> covar;
//   Vector<double> errors;
// ///   LSQaips tmpSolver = itsSolver;
// ///   tmpSolver.getCovariance (covar);
// ///   tmpSolver.getErrors (errors);
//   int nrs = itsSolution.nelements();
//   Vector<double> sol(nrs);
//   double* solData = itsSolution.doubleStorage();
//   for (int i=0; i<itsSolution.nelements(); i++) {
//     sol[i] = solData[i];
//   }
//   solveTimer.start();
//   bool solFlag = itsSolver.solveLoop (fit, rank, sol, useSVD);
//   solveTimer.stop();
//   for (int i=0; i<itsSolution.nelements(); i++) {
//     solData[i] = sol[i];
//   }
//   //  if (Debug(1)) timer.show("solve");
//   LOG_INFO_STR( "Solution after:  " << itsSolution );
//   //cout << "Solution after:  " << itsSolution << endl;


  
//   resultParmValues.clear();
//   for (int nr=0; nr < itsSolution.nx(); nr++)
//   {
//     resultParmValues.push_back(itsSolution.getDouble(nr, 0));
//   }

//   resultParmNames.clear();
 
//   // Update all parameters.
//   const vector<MeqParm*>& parmList = MeqParm::getParmList();
//   int i=0;
//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     if (itsIsParmSolvable[i]) {
//       resultParmNames.push_back((*iter)->getName());

//       (*iter)->update (itsSolution);
//     }
//     i++;
//   }

//   resultQuality.init();
//   resultQuality.itsSolFlag = solFlag;
//   resultQuality.itsRank = rank;
//   resultQuality.itsFit = fit;
//   resultQuality.itsMu = itsSolver.getWeightedSD();;
//   resultQuality.itsStddev = itsSolver.getSD();;
//   resultQuality.itsChi = itsSolver.getChi();
//   cout << resultQuality << endl;

//   totTimer.stop();
//   cout << "BBSTest: solver     " << solveTimer << endl;
//   cout << "BBSTest: total-iter " << totTimer << endl;
//   timer.show("BBSTest: solve ");

  return;
}


//----------------------------------------------------------------------
//
// ~saveParms
//
// Save parameters which have an updated warning.
//
//----------------------------------------------------------------------
void Solver::saveParms()
{
//   NSTimer saveTimer;
//   saveTimer.start();
//   const vector<MeqParm*>& parmList = MeqParm::getParmList();

//   LOG_TRACE_RTTI( "saveParms");

//   ASSERT (!itsSolution.isNull()  &&  itsSolution.nx() == itsNrScid);
//   int i=0;
//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     if (itsIsParmSolvable[i]) {
//       (*iter)->save();
//     }
//     i++;
//   }
//   // Unlock the parm tables.
//   itsMEP.unlock();
//   itsGSMMEP.unlock();
//   saveTimer.stop();
//   cout << "BBSTest: save-parm    " << saveTimer << endl;
}

//----------------------------------------------------------------------
//
// ~saveAllSolvableParms
//
// Save all solvable parameters.
//
//----------------------------------------------------------------------
// void Solver::saveAllSolvableParms()
// {
//   const vector<MeqParm*>& parmList = MeqParm::getParmList();

//   LOG_TRACE_RTTI( "saveAllSolvableParms");

//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     if ((*iter)->isSolvable())
//     {
//       (*iter)->save();
//     }
//   }
//   // Unlock the parm tables.
//   itsMEP.unlock();
//   itsGSMMEP.unlock();
// }

//----------------------------------------------------------------------
//
// ~addParm
//
// Add the result of a parameter for the current domain to a GlishRecord
// for the purpose of passing the information back to a glish script.
//
//----------------------------------------------------------------------
// void Solver::addParm(const MeqParm& parm, bool denormalize,
// 			    GlishRecord& rec)
// {
//   GlishRecord parmRec;

//   MeqMatrix m;
//   m = MeqMatrix (double(), itsNrScid, 1);
  
//   parmRec.add("parmid", Int(parm.getParmId()));

//   try {
//     parm.getCurrentValue(m, denormalize);
//     GlishArray ga(m.getDoubleMatrix());
//     parmRec.add("value",  ga);
//   } catch (...) {
//     parmRec.add("value", "<?>");
//   }
  
//   rec.add(parm.getName(), parmRec);
// }

//----------------------------------------------------------------------
//
// ~getParms
//
// Get a description of the parameters whose name matches the
// parmPatterns pattern. The description shows the result of the
// evaluation of the parameter on the current time domain.
//
//----------------------------------------------------------------------
// GlishRecord Solver::getParms(Vector<String>& parmPatterns,
// 				    Vector<String>& excludePatterns,
// 				    int isSolvable, bool denormalize)
// {
//   LOG_TRACE_RTTI( "getParms: " );
//   if (itsDataMap->getStart() == 0) {
//     throw AipsError("nextInterval needs to be done before getParms");
//   }
//   GlishRecord rec;
//   //  vector<MeqParm*> parmVector;

//   const vector<MeqParm*>& parmList = MeqParm::getParmList();

//   // Convert patterns to regexes.
//   vector<Regex> parmRegex;
//   for (unsigned int i=0; i<parmPatterns.nelements(); i++) {
//     parmRegex.push_back (Regex::fromPattern(parmPatterns[i]));
//   }
//   vector<Regex> excludeRegex;
//   for (unsigned int i=0; i<excludePatterns.nelements(); i++) {
//     excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
//   }
//   //
//   // Find all parms matching the parmPatterns
//   // Exclude them if matching an excludePattern
//   //
//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     bool ok = true;
//     if (isSolvable == 0) {
//       ok = !((*iter)->isSolvable());
//     } else if (isSolvable > 0) {
//       ok = ((*iter)->isSolvable());
//     }
//     if (ok) {
//       String parmName ((*iter)->getName());

//       for (vector<Regex>::const_iterator incIter = parmRegex.begin();
// 	   incIter != parmRegex.end();
// 	   incIter++)
//       {
// 	if (parmName.matches(*incIter))
// 	{
// 	  bool parmExc = false;
// 	  for (vector<Regex>::const_iterator excIter = excludeRegex.begin();
// 	       excIter != excludeRegex.end();
// 	       excIter++)
// 	  {
// 	    if (parmName.matches(*excIter))
// 	    {
// 	      parmExc = true;
// 	      break;
// 	    }
// 	  }
// 	  if (!parmExc) {
// 	    addParm (**iter, denormalize, rec);
// 	  }
// 	  break;
// 	}
//       }
//     }
//   }
//   return rec;
// }

//----------------------------------------------------------------------
//
// ~getParmNames
//
// Get the names of the parameters whose name matches the parmPatterns.
// Exclude the names that match the excludePatterns.
// E.g. getParmNames("*") returns all parameter names.
//
//----------------------------------------------------------------------
// GlishArray Solver::getParmNames(Vector<String>& parmPatterns,
// 				       Vector<String>& excludePatterns)
// {
//   const int PARMNAMES_CHUNKSIZE = 100;
//   int maxlen = PARMNAMES_CHUNKSIZE;
//   int current=0;
//   Vector<String> parmNameVector(maxlen);
//   //  vector<MeqParm*> parmVector;

//   const vector<MeqParm*>& parmList = MeqParm::getParmList();

//   LOG_TRACE_RTTI( "getParmNames: ");

//   // Convert patterns to regexes.
//   vector<Regex> parmRegex;
//   for (unsigned int i=0; i<parmPatterns.nelements(); i++) {
//     parmRegex.push_back (Regex::fromPattern(parmPatterns[i]));
//   }
//   vector<Regex> excludeRegex;
//   for (unsigned int i=0; i<excludePatterns.nelements(); i++) {
//     excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
//   }
//   //
//   // Find all parms matching the parmPatterns
//   // Exclude them if matching an excludePattern
//   //
//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     String parmName ((*iter)->getName());

//     for (vector<Regex>::const_iterator incIter = parmRegex.begin();
// 	 incIter != parmRegex.end();
// 	 incIter++)
//     {
//       {
// 	if (parmName.matches(*incIter))
// 	{
// 	  bool parmExc = false;
// 	  for (vector<Regex>::const_iterator excIter = excludeRegex.begin();
// 	       excIter != excludeRegex.end();
// 	       excIter++)
// 	  {
// 	    if (parmName.matches(*excIter))
// 	    {
// 	      parmExc = true;
// 	      break;
// 	    }
// 	  }
// 	  if (!parmExc) {
// 	    if (current >= maxlen)
// 	    {
// 	      maxlen += PARMNAMES_CHUNKSIZE;
// 	      parmNameVector.resize(maxlen, True);
// 	    }
// 	    parmNameVector[current++] = parmName;
// 	  }
// 	  break;
// 	}
//       }
//     }
//   }

//   parmNameVector.resize(current, True);
//   GlishArray arr(parmNameVector);

//   return arr;
// }

//----------------------------------------------------------------------
//
// ~getSolveDomain
//
// Store the current solve domain in a GlishRecord for the purpose
// of passing it back to the calling glish script.
//
//----------------------------------------------------------------------
// GlishRecord Solver::getSolveDomain()
// {
//   GlishRecord rec;
//   rec.add("startx", itsSolveDomain.startX());
//   rec.add("endx",   itsSolveDomain.endX());
//   rec.add("starty", itsSolveDomain.startY());
//   rec.add("endy",   itsSolveDomain.endY());
//   return rec;
//}


//----------------------------------------------------------------------
//
// ~select
//
// Select a subset of the MS data.                                        >>>>>>>> Keep this, possibility to use a subset of data (it sets itsFirstChan o.a.)
// The baselines and the channels (frequency window) to be used can be specified.
// This selection has to be done before the loop over domains.
//
//----------------------------------------------------------------------
// void Solver::select(const vector<int>& ant1, 
// 			   const vector<int>& ant2, 
// 			   int firstChan, int lastChan)
// {
//   if (firstChan < 0  ||  firstChan >= itsNrChan) {
//     itsFirstChan = 0;
//   } else {
//     itsFirstChan = firstChan;
//   }
//   if (lastChan < 0  ||  lastChan >= itsNrChan) {
//     itsLastChan = itsNrChan-1;
//   } else {
//     itsLastChan = lastChan;
//   }
//   ASSERT (itsFirstChan <= itsLastChan);

//   ASSERT ( ant1.size() == ant2.size());
//   itsBLSelection = false;
// //   if (ant1.size() == 0)  // If no baselines specified, select all baselines
// //   {
// //     itsBLSelection = true;
// //   }
// //   else
// //   {
//     for (unsigned int i=0; i<ant1.size(); i++)
//     {
//       ASSERT(ant1[i] < int(itsBLSelection.nrow()));
//       for (unsigned int j=0; j<ant2.size(); j++)
//       {
// 	ASSERT(ant2[i] < int(itsBLSelection.nrow()));
// 	itsBLSelection(ant1[i], ant2[j]) = true;     // Select baseline subset
//       }
//       //    }
//   }
  
//}

//----------------------------------------------------------------------
//
// ~fillUVW
//
// Calculate the station UVW coordinates from the MS.
//
//----------------------------------------------------------------------
// void Solver::fillUVW()
// {
//   LOG_TRACE_RTTI( "get UVW coordinates from MS" );
//   int nant = itsStatUVW.size();
//   vector<bool> statFnd (nant);
//   vector<bool> statDone (nant);
//   vector<double> statuvw(3*nant);

//   // Determine the number of stations (found)
//   statFnd.assign (statFnd.size(), false);
//   int nStatFnd = 0;
//   for (unsigned int bl=0; bl < itsNrBl; bl++)
//   {
//     int a1 = itsAnt1Data[bl];
//     int a2 = itsAnt2Data[bl];
//     if (itsBLSelection(a1,a2) == true)
//     {
//       if (!statFnd[itsAnt1Data[bl]]) {
// 	nStatFnd++;
// 	statFnd[itsAnt1Data[bl]] = true;
//       }
//       if (!statFnd[itsAnt2Data[bl]]) {
// 	nStatFnd++;
// 	statFnd[itsAnt2Data[bl]] = true;
//       }
//     }
//   }

//   // Map uvw data into memory
//   size_t nrBytes = itsTimes.nelements() * itsNrBl * 3 * sizeof(double);
//   double* uvwDataPtr = 0;
//   MMap* mapPtr = new MMap(itsMSName+".uvw", MMap::Read);
//   mapPtr->mapFile(0, nrBytes);
//   uvwDataPtr = (double*)mapPtr->getStart();
//   if (itsLockMappedMem)
//   {                                     // Make sure mapped data is resident in RAM
//     mapPtr->lockMappedMemory();
//   }   

//   // Step time by time through the MS.
//   for (unsigned int tStep=0; tStep < itsTimes.nelements(); tStep++)
//   {
//     // Set uvw pointer to beginning of this time
//     unsigned int tOffset = tStep * itsNrBl * 3;
//     double* uvw = uvwDataPtr + tOffset;

//     double time = itsTimes[tStep];
    
//     // Set UVW of first station to 0 (UVW coordinates are relative!).
//     statDone.assign (statDone.size(), false);
//     statuvw[3*itsAnt1Data[0]]   = 0;
//     statuvw[3*itsAnt1Data[0]+1] = 0;
//     statuvw[3*itsAnt1Data[0]+2] = 0;
//     statDone[itsAnt1Data[0]] = true;
//     itsStatUVW[itsAnt1Data[0]]->set (time, 0, 0, 0);

// //     cout << "itsStatUVW[" << itsAnt1Data[0] << "] time: " << time << " 0, 0, 0" << endl;

//     int ndone = 1;
//     // Loop until all found stations are handled. This is necessary when not all 
//     // stations can be calculated in one loop (depends on the order)
//     while (ndone < nStatFnd) 
//     {
//       int nd = 0;
//       // Loop over baselines
//       for (unsigned int bl=0; bl < itsNrBl; bl++)
//       {
// 	int a1 = itsAnt1Data[bl];
// 	int a2 = itsAnt2Data[bl];
// 	if (itsBLSelection(a1,a2) == true)
// 	{
// 	  if (!statDone[a2]) {
// 	    if (statDone[a1]) {
// 	      statuvw[3*a2]   = uvw[3*bl]   - statuvw[3*a1];
// 	      statuvw[3*a2+1] = uvw[3*bl+1] - statuvw[3*a1+1];
// 	      statuvw[3*a2+2] = uvw[3*bl+2] - statuvw[3*a1+2];
// 	      statDone[a2] = true;
// 	      itsStatUVW[a2]->set (time, statuvw[3*a2], statuvw[3*a2+1],
// 				   statuvw[3*a2+2]);

// // 	      cout << "itsStatUVW[" << a2 << "] time: " << time << statuvw[3*a2] << " ," << statuvw[3*a2+1] << " ," << statuvw[3*a2+2] << endl;

// 	      ndone++;
// 	      nd++;
// 	    }
// 	  } else if (!statDone[a1]) {
// 	    if (statDone[a2]) {
// 	      statuvw[3*a1]   = statuvw[3*a2]   - uvw[3*bl];
// 	      statuvw[3*a1+1] = statuvw[3*a2+1] - uvw[3*bl+1];
// 	      statuvw[3*a1+2] = statuvw[3*a2+2] - uvw[3*bl+2];
// 	      statDone[a1] = true;
// 	      itsStatUVW[a1]->set (time, statuvw[3*a1], statuvw[3*a1+1],
// 				   statuvw[3*a1+2]);
// // 	      cout << "itsStatUVW[" << a1 << "] time: " << time << statuvw[3*a1] << " ," << statuvw[3*a1+1] << " ," << statuvw[3*a1+2] << endl;

// 	      ndone++;
// 	      nd++;
// 	    }
// 	  }
// 	  if (ndone == nStatFnd) {
// 	    break;
// 	  }

// 	} // End if (itsBLSelection(ant1,ant2) ==...

//       } // End loop baselines

//       //	  ASSERT (nd > 0);

//     } // End loop stations found
//   } // End loop time

//   // Finished with map
//   delete mapPtr;
// }


//----------------------------------------------------------------------
//
// ~peel
//
// Define the source numbers to use in a peel step.
//
//----------------------------------------------------------------------
// Bool Solver::peel(const vector<int>& peelSources,
// 			 const vector<int>& extraSources)
// {
//   Vector<Int> peelSourceNrs(peelSources.size());
//   for (unsigned int i=0; i<peelSources.size(); i++)
//   {
//     peelSourceNrs[i] = peelSources[i];
//   }
//   Vector<Int> extraSourceNrs(extraSources.size());
//   for (unsigned int i=0; i<extraSources.size(); i++)
//   {
//     extraSourceNrs[i] = extraSources[i];
//   }

//   // Make a shallow copy to get a non-const object.  
//   Vector<Int> tmpPeel(peelSourceNrs);
//   Vector<Int> sourceNrs;
//   if (extraSourceNrs.nelements() == 0) {
//     sourceNrs.reference (tmpPeel);
//   } else {
//     sourceNrs.resize (peelSourceNrs.nelements() + extraSourceNrs.nelements());
//     sourceNrs(Slice(0,peelSourceNrs.nelements())) = peelSourceNrs;
//     sourceNrs(Slice(peelSourceNrs.nelements(), extraSourceNrs.nelements())) =
//       extraSourceNrs;
//   }
//   LOG_TRACE_OBJ_STR( "peel: sources " << tmpPeel << " predicting sources "
// 	    << sourceNrs );

//   ASSERT (peelSourceNrs.nelements() > 0);
//   vector<int> src(sourceNrs.nelements());
//   for (unsigned int i=0; i<src.size(); i++) {
//     src[i] = sourceNrs[i];
//     LOG_TRACE_OBJ_STR( "Predicting source " << sourceNrs[i] );
//   }
//   itsSources.setSelected (src);
//   itsPeelSourceNrs.reference (tmpPeel);
//   return True;
// }

//----------------------------------------------------------------------
//
// ~getStatistics
//
// Get a description of the parameters whose name matches the
// parmPatterns pattern. The description shows the result of the
// evaluation of the parameter on the current time domain.
//
//----------------------------------------------------------------------
// GlishRecord Solver::getStatistics (bool detailed, bool clear)
// {
//   GlishRecord rec;

//   LOG_TRACE_RTTI( "getStatistics: " );

//   // Get the total counts.
//   rec.add ("timecellstotal", MeqHist::merge (itsCelltHist));
//   rec.add ("freqcellstotal", MeqHist::merge (itsCellfHist));

//   if (detailed) {
//     // Get the counts per station.
//     int nrant = itsBLIndex.nrow();
//     for (int ant2=0; ant2<nrant; ant2++) {
//       String str2 = String::toString(ant2);
//       for (int ant1=0; ant1<nrant; ant1++) {
// 	int blindex = itsBLIndex(ant1,ant2);
// 	if (blindex >= 0) {
// 	  String str = String::toString(ant1) + '_' + str2;
// 	  rec.add ("timecells_" + str, itsCelltHist[blindex].get());
// 	  rec.add ("freqcells_" + str, itsCellfHist[blindex].get());
// 	}
//       }
//     }
//   }
//   if (clear) {
//     int nrant = itsBLIndex.nrow();
//     for (int ant2=0; ant2<nrant; ant2++) {
//       String str2 = String::toString(ant2);
//       for (int ant1=0; ant1<nrant; ant1++) {
// 	int blindex = itsBLIndex(ant1,ant2);
// 	if (blindex >= 0) {
// 	  itsCelltHist[blindex].clear();
// 	  itsCellfHist[blindex].clear();
// 	}
//       }
//     }
//   }
//   return rec;
// }


// void Solver::getParmValues (vector<string>& names,
// 				   vector<double>& values)
// {
//   vector<MeqMatrix> vals;
//   getParmValues (names, vals);
//   values.resize (0);
//   values.reserve (vals.size());
//   for (vector<MeqMatrix>::const_iterator iter = vals.begin();
//        iter != vals.end();
//        iter++) {
//     ASSERT (iter->nelements() == 1);
//     values.push_back (iter->getDouble());
//   }
// }

// void Solver::getParmValues (vector<string>& names,
// 				   vector<MeqMatrix>& values)
// {
//   MeqMatrix val;
//   names.resize (0);
//   values.resize (0);
//   // Iterate through all parms and get solvable ones.
//   const vector<MeqParm*>& parmList = MeqParm::getParmList();
//   int i=0;
//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     if ((*iter)->isSolvable()) {
//       names.push_back ((*iter)->getName());
//       (*iter)->getCurrentValue (val, false);
//       values.push_back (val);
//     }
//     i++;
//   }
// }

// void Solver::setParmValues (const vector<string>& names,
// 				   const vector<double>& values)
// {
//   vector<MeqMatrix> vals;
//   vals.reserve (values.size());
//   for (vector<double>::const_iterator iter = values.begin();
//        iter != values.end();
//        iter++) {
//     vals.push_back (MeqMatrix (*iter));
//   }
//   setParmValues (names, vals);
// }

// void Solver::setParmValues (const vector<string>& names,
// 				   const vector<MeqMatrix>& values)
// {
//   ASSERT (names.size() == values.size());
//   // Iterate through all parms and get solvable ones.
//   const vector<MeqParm*>& parmList = MeqParm::getParmList();
//   int i;
//   for (vector<MeqParm*>::const_iterator iter = parmList.begin();
//        iter != parmList.end();
//        iter++)
//   {
//     const string& pname = (*iter)->getName();
//     i = 0;
//     for (vector<string>::const_iterator itern = names.begin();
// 	 itern != names.end();
// 	 itern++) {
//       if (*itern == pname) {
// 	const MeqParmPolc* ppc = dynamic_cast<const MeqParmPolc*>(*iter);
// 	ASSERT (ppc);
// 	MeqParmPolc* pp = const_cast<MeqParmPolc*>(ppc);
// 	const vector<MeqPolc>& polcs = pp->getPolcs();
// 	ASSERT (polcs.size() == 1);
// 	MeqPolc polc = polcs[0];
// 	polc.setCoeffOnly (values[i]);
// 	pp->setPolcs (vector<MeqPolc>(1,polc));
// 	break;
//       }
//       i++;
//       // A non-matching name is ignored; no warning is given.
//     }
//   }
// }

// void Solver::showSettings() const
// {
//   cout << "Solver settings:" << endl;
//   cout << "  mepname:   " << itsMEPName << endl;
//   cout << "  gsmname:   " << itsGSMMEPName << endl;
//   cout << "  time interval: " << itsTimeInterval << endl;
//   cout << "  solvparms: " << itsSolvableParms << endl;
// }

// void Solver::showParmValues()
// {
//   vector<string> parms(itsSolvableParms.nelements());
//   for (unsigned int i=0; i<itsSolvableParms.nelements(); i++)
//   {
//     parms[i] = itsSolvableParms[i];
//   }

//   vector <double> vals;
//   getParmValues (parms, vals);

//   vector<string> :: iterator i;
//   char str [20];
//   vector<double> :: iterator j = vals.begin();
//   for (i = parms.begin (); i != parms.end (); ++i, ++j) {
//     sprintf (str, "%12.9f ", *j);
//     cout << "BBSTest: parm " << *i << " = " << str << endl;
//   }
// }


void Solver::setEquations (const dcomplex* data, int nresult, int nrspid,
		   int nrtime, int nrfreq, int prediffer)
{}

void Solver::setSolvableParmData (const ParmData&, int prediffer)
{}

} // namespace LOFAR
