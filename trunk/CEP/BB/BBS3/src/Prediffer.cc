//# PredifferImpl.cc: Read and predict read visibilities
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

#include <BBS3/Prediffer.h>

#include <BBS3/MNS/MeqJonesNode.h>
#include <BBS3/MNS/MeqStatExpr.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <BBS3/MNS/MeqStoredParmPolc.h>
#include <BBS3/MNS/MeqParmSingle.h>
#include <BBS3/MNS/MeqPointDFT.h>
#include <BBS3/MNS/MeqPointSource.h>
#include <BBS3/MNS/MeqWsrtInt.h>
#include <BBS3/MNS/MeqWsrtPoint.h>
#include <BBS3/MNS/MeqLofarPoint.h>
#include <BBS3/MNS/MeqMatrixComplexArr.h>

#include <Common/Timer.h>
#include <Common/LofarLogger.h>

#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVBaseline.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Utilities/Regex.h>
#include <casa/OS/Timer.h>
#include <casa/Exceptions/Error.h>

#include <Common/BlobIStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/BlobArray.h>

#include <BBS3/MMap.h>

#include <stdexcept>
#include <iostream>
#include <fstream>

using namespace casa;

namespace LOFAR
{

//----------------------------------------------------------------------
//
// Constructor. Initialize a Prediffer object.
//
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the WSRT.
//
//----------------------------------------------------------------------
Prediffer::Prediffer(const string& msName,
		     const string& meqModel,
		     const string& skyModel,
		     const string& dbType,
		     const string& dbName,
		     const string& dbHost,
		     const string& dbPwd,
		     uInt    ddid,
		     const vector<int>& ant,
		     const string& modelType,
		     bool    calcUVW,
		     bool    lockMappedMem)
  :
  itsMSName       (msName),
  itsMEPName      (meqModel),
  itsMEP          (dbType, meqModel, dbName, dbPwd, dbHost),
  itsGSMMEPName   (skyModel),
  itsGSMMEP       (dbType, skyModel, dbName, dbPwd, dbHost),
  itsCalcUVW      (calcUVW),
  itsNPol         (0),
  itsNrBl         (0),
  itsSelAnt       (ant),
  itsTimeIndex    (0),
  itsNrTimes      (0),
  itsDataMap      (0),
  itsLockMappedMem(lockMappedMem)
{
  LOG_INFO_STR( "MeqCalibrater constructor ("
		<< "'" << msName   << "', "
		<< "'" << meqModel << "', "
		<< "'" << skyModel << "', "
		<< ddid << ", "
		<< itsCalcUVW << ")" );

  readDescriptiveData (msName);

  vector<unsigned int> ant1;
  vector<unsigned int> ant2;
  ASSERTSTR (ant.size() > 0, "No antennas selected");
  for (unsigned int i=0; i<ant.size(); i++)
  {
    for (unsigned int j=i+1; j<ant.size(); j++)
    {
      ant1.push_back(ant[i]);
      ant2.push_back(ant[j]);
    }
  }

  fillStations (ant1, ant2);     // Selected antennas
  fillBaselines (ant1, ant2);

  itsDataMap = new MMap(msName + ".dat", MMap::Read);

  // Set up the expression tree for all baselines.
  if (modelType == "WSRT") {
    makeWSRTExpr();
  } else if (modelType == "LOFAR.RI") {
    makeLOFARExpr(False);
  } else {
    makeLOFARExpr(True);
  }

  LOG_INFO_STR( "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
		<< ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor );
  LOG_INFO_STR( "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
		<< ' ' << MeqResultRep::nctor + MeqResultRep::ndtor );

  // Show frequency domain.
  LOG_INFO_STR( "Freq: " << itsStartFreq << ' ' << itsEndFreq << " (" <<
    itsEndFreq - itsStartFreq << " Hz) " << itsNrChan << " channels of "
	    << itsStepFreq << " Hz" );

  // Do the initial antenna selection.
  select (ant, ant, 0, -1); 
                                                             
  if (!itsCalcUVW) {
    // Fill the UVW coordinates from the MS instead of calculating them.
    fillUVW();
 }

  // initialize the ComplexArr pool with the most frequently used size
  // itsNrChan is the numnber frequency channels
  // 1 is the number of time steps. This code is limited to one timestep only
  MeqMatrixComplexArr::poolActivate(itsNrChan * 1);
  // Unlock the parm tables.
  itsMEP.unlock();
  itsGSMMEP.unlock();

}

//----------------------------------------------------------------------
//
// ~~Prediffer
//
// Destructor for a Prediffer object.
//
//----------------------------------------------------------------------
Prediffer::~Prediffer()
{
  LOG_TRACE_FLOW( "MeqCalibrater destructor" );

  MeqParm::clearParmList();     // Clear static parameter data

  for (vector<MeqJonesExpr*>::iterator iter = itsExpr.begin();
       iter != itsExpr.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqJonesExpr*>::iterator iter = itsResExpr.begin();
       iter != itsResExpr.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqJonesExpr*>::iterator iter = itsStatExpr.begin();
       iter != itsStatExpr.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqStatSources*>::iterator iter = itsStatSrc.begin();
       iter != itsStatSrc.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqLofarStatSources*>::iterator iter = itsLSSExpr.begin();
       iter != itsLSSExpr.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqStatUVW*>::iterator iter = itsStatUVW.begin();
       iter != itsStatUVW.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqStation*>::iterator iter = itsStations.begin();
       iter != itsStations.end();
       iter++) {
    delete *iter;
  }

  delete itsDataMap;

  // clear up the matrix pool
  MeqMatrixComplexArr::poolDeactivate();
}


//-------------------------------------------------------------------
//
// ~readDescriptiveData
//
// Get measurement set description from file
//
//-------------------------------------------------------------------
void Prediffer::readDescriptiveData(const string& fileName)
{
  // Get ra, dec, npol, nfreq, startFreq, endFreq, stepFreq, a1, a2, tim2 from file
  string name(fileName+".des");
  cout << fileName << endl;
  std::ifstream istr(name.c_str());
  BlobIBufStream bbs(istr);
  BlobIStream bis(bbs);
  bis.getStart("ms.des");
  double ra, dec;
  bis >> ra;
  bis >> dec;
  bis >> itsNPol;
  bis >> itsNrChan;
  bis >> itsStartFreq;
  bis >> itsEndFreq;
  bis >> itsStepFreq;
  bis >> itsAnt1Data;
  bis >> itsAnt2Data;
  bis >> itsTimes;
  bis >> itsIntervals;
  bis >> itsAntPos;

//   cout << "Descriptive Data:" << endl;
//   cout << ra << ", " << dec << ", " << itsNPol << ", " << itsNrChan << ", " << itsStartFreq << ", " 
//        << itsEndFreq << ", " << itsStepFreq << ", " << endl;
//   cout << endl;
//   cout << itsAnt1Data << endl;
//   cout << endl;
//   cout << itsAnt2Data << endl;
//   cout << endl;
//   cout << itsTimes << endl;
//   cout << itsIntervals << endl;
  bis.getEnd();

  ASSERT (itsAnt1Data.nelements() == itsAnt2Data.nelements());
  itsNrBl = itsAnt1Data.nelements();

  getPhaseRef(ra, dec, itsTimes[0]);
}

//----------------------------------------------------------------------
//                                                                       
// ~getPhaseRef
//
// Get the phase reference of the first field.
//
//----------------------------------------------------------------------
void Prediffer::getPhaseRef(double ra, double dec, double startTime)
{
  // Use the phase reference of the given J2000 ra/dec.
  MVDirection mvdir(ra, dec);
  MDirection phaseRef(mvdir, MDirection::J2000);
  itsPhaseRef = MeqPhaseRef (phaseRef, startTime);
}


//----------------------------------------------------------------------
//
// ~fillStations
//
// Fill the station positions and names.
//
//----------------------------------------------------------------------
void Prediffer::fillStations (const vector<unsigned int>& ant1,
			      const vector<unsigned int>& ant2)
{
  int nrant = itsAntPos.ncolumn();
  itsStations = vector<MeqStation*>(nrant, (MeqStation*)0);
  // Get all stations actually used.
  char str[8];
  for (uint i=0; i<ant1.size(); i++) {
    for (int j=0; j<2; j++) {
      int ant = ant1[i];
      if (j==1) ant = ant2[i];
      ASSERT (ant < nrant);
      if (itsStations[ant] == 0) {
	// Store each position as a constant parameter.
	// Use the antenna name as the parameter name.
	Vector<Double> antpos = itsAntPos.column(ant);
	sprintf (str, "%d", ant+1);
	String name = string("SR") + str;
	MeqParmSingle* px = new MeqParmSingle ("AntPosX." + name,
					       antpos(0));
	MeqParmSingle* py = new MeqParmSingle ("AntPosY." + name,
					       antpos(1));
	MeqParmSingle* pz = new MeqParmSingle ("AntPosZ." + name,
					       antpos(2));
	itsStations[ant] = new MeqStation(px, py, pz, name);
      }
    }
  }
}

//----------------------------------------------------------------------
//
// ~fillBaselines
//
// Create objects representing the baseline directions.
// Create an index giving the baseline index of an ordered antenna pair.
//
//----------------------------------------------------------------------
void Prediffer::fillBaselines (const vector<unsigned int>& ant1,
			       const vector<unsigned int>& ant2)
{
  MeqDomain  domain;
  MeqRequest req(domain, 1, 1);

  itsBaselines.reserve (ant1.size());
  itsBLIndex.resize (itsStations.size(), itsStations.size());
  itsBLIndex = -1;

  for (unsigned int i=0; i<ant1.size(); i++)
  {
    uInt a1 = ant1[i];
    uInt a2 = ant2[i];
    // Create an MVBaseline object for each antenna pair.
    ASSERT (a1 < itsStations.size()  &&  a2 < itsStations.size());
    itsBLIndex(a1,a2) = itsBaselines.size();
    MVPosition pos1
      (itsStations[a1]->getPosX()->getResult(req).getValue().getDouble(),
       itsStations[a1]->getPosY()->getResult(req).getValue().getDouble(),
       itsStations[a1]->getPosZ()->getResult(req).getValue().getDouble());
    MVPosition pos2
      (itsStations[a2]->getPosX()->getResult(req).getValue().getDouble(),
       itsStations[a2]->getPosY()->getResult(req).getValue().getDouble(),
       itsStations[a2]->getPosZ()->getResult(req).getValue().getDouble());

    itsBaselines.push_back (MVBaseline (pos1, pos2));
  }

  //>>>>>>> Set all baselines to "not selected".
  itsBLSelection.resize (itsStations.size(), itsStations.size()); 
  itsBLSelection = false;  
}

//----------------------------------------------------------------------
//
// ~makeWSRTExpr
//
// Make the expression tree per baseline for the WSRT.
//
//----------------------------------------------------------------------
void Prediffer::makeWSRTExpr()
{
  // Get the sources from the ParmTable.
  itsSources = itsGSMMEP.getPointSources (Vector<int>());
  for (int i=0; i<itsSources.size(); i++) {
    itsSources[i].setSourceNr (i);
    itsSources[i].setPhaseRef (&itsPhaseRef);
  }

  // Make expressions for each station.
  itsStatUVW  = vector<MeqStatUVW*>     (itsStations.size(),
					 (MeqStatUVW*)0);
  itsStatSrc  = vector<MeqStatSources*> (itsStations.size(),
					 (MeqStatSources*)0);
  itsStatExpr = vector<MeqJonesExpr*>   (itsStations.size(),
					 (MeqJonesExpr*)0);
  for (unsigned int i=0; i<itsStations.size(); i++) {
    if (itsStations[i] != 0) {
      // Expression to get UVW per station
      if (itsCalcUVW) {
	itsStatUVW[i] = new MeqStatUVW (itsStations[i], &itsPhaseRef);
      } else {
	itsStatUVW[i] = new MeqStatUVW;
      }
      // Expression to calculate contribution per station per source.
      itsStatSrc[i] = new MeqStatSources (itsStatUVW[i], &itsSources);
      // Expression representing station parameters.
      MeqExpr* frot = new MeqStoredParmPolc ("frot." +
					     itsStations[i]->getName(),
					     -1, i+1,
					     &itsMEP);
      MeqExpr* drot = new MeqStoredParmPolc ("drot." +
					     itsStations[i]->getName(),
					     -1, i+1,
					     &itsMEP);
      MeqExpr* dell = new MeqStoredParmPolc ("dell." +
					     itsStations[i]->getName(),
					     -1, i+1,
					     &itsMEP);
      MeqExpr* gain11 = new MeqStoredParmPolc ("gain.11." +
					       itsStations[i]->getName(),
					       -1, i+1,
					       &itsMEP);
      MeqExpr* gain22 = new MeqStoredParmPolc ("gain.22." +
					       itsStations[i]->getName(),
					       -1, i+1,
					       &itsMEP);
      itsStatExpr[i] = new MeqStatExpr (frot, drot, dell, gain11, gain22);
    }
  }    

  // Make an expression for each baseline.
  itsExpr.resize (itsBaselines.size());
  itsResExpr.resize (itsBaselines.size());
  // Create the histogram object for couting of used #cells in time and freq
  itsCelltHist.resize (itsBaselines.size());
  itsCellfHist.resize (itsBaselines.size());
  int nrant = itsBLIndex.nrow();
  for (int ant2=0; ant2<nrant; ant2++) {
    for (int ant1=0; ant1<nrant; ant1++) {
      int blindex = itsBLIndex(ant1,ant2);
      if (blindex >= 0) {
	// Create the DFT kernel.
	MeqPointDFT* dft = new MeqPointDFT (itsStatSrc[ant1],
					    itsStatSrc[ant2]);
	itsResExpr[blindex] = new MeqWsrtPoint (&itsSources, dft,
						&itsCelltHist[blindex],
						&itsCellfHist[blindex]);
	itsExpr[blindex] = new MeqWsrtInt (itsResExpr[blindex],
					   itsStatExpr[ant1],
					   itsStatExpr[ant2]);
      }
    }
  }
}

//----------------------------------------------------------------------
//
// ~makeLOFARExpr
//
// Make the expression tree per baseline for the WSRT.
//
//----------------------------------------------------------------------
void Prediffer::makeLOFARExpr(Bool asAP)
{
  // Get the sources from the ParmTable.
  itsSources = itsGSMMEP.getPointSources (Vector<int>());
  for (int i=0; i<itsSources.size(); i++) {
    itsSources[i].setSourceNr (i);
    itsSources[i].setPhaseRef (&itsPhaseRef);
  }

  // Make expressions for each station.
  itsStatUVW  = vector<MeqStatUVW*>          (itsStations.size(),
					      (MeqStatUVW*)0);
  itsStatSrc  = vector<MeqStatSources*>      (itsStations.size(),
					      (MeqStatSources*)0);
  itsLSSExpr  = vector<MeqLofarStatSources*> (itsStations.size(),
					      (MeqLofarStatSources*)0);
  itsStatExpr = vector<MeqJonesExpr*>        (itsStations.size(),
					      (MeqJonesExpr*)0);
  string ejname1 = "real.";
  string ejname2 = "imag.";
  if (asAP) {
    ejname1 = "ampl.";
    ejname2 = "phase.";
  }
  for (unsigned int i=0; i<itsStations.size(); i++) {
    if (itsStations[i] != 0) {
      // Expression to calculate UVW per station
      itsStatUVW[i] = new MeqStatUVW (itsStations[i], &itsPhaseRef);
      // Expression to calculate contribution per station per source.
      itsStatSrc[i] = new MeqStatSources (itsStatUVW[i], &itsSources);
      // Expression representing station parameters.
      MeqExpr* frot = new MeqStoredParmPolc ("frot." +
					     itsStations[i]->getName(),
					     -1, i+1,
					     &itsMEP);
      MeqExpr* drot = new MeqStoredParmPolc ("drot." +
					     itsStations[i]->getName(),
					     -1, i+1,
					     &itsMEP);
      MeqExpr* dell = new MeqStoredParmPolc ("dell." +
					     itsStations[i]->getName(),
					     -1, i+1,
					     &itsMEP);
      MeqExpr* gain11 = new MeqStoredParmPolc ("gain.11." +
					       itsStations[i]->getName(),
					       -1, i+1,
					       &itsMEP);
      MeqExpr* gain22 = new MeqStoredParmPolc ("gain.22." +
					       itsStations[i]->getName(),
					       -1, i+1,
					       &itsMEP);
      itsStatExpr[i] = new MeqStatExpr (frot, drot, dell, gain11, gain22);
      // Make an expression for all source parameters for this station.
      vector<MeqJonesExpr*> vec;
      for (int j=0; j<itsSources.size(); j++) {
	string nm = itsStations[i]->getName() + '.' +  itsSources[j].getName();
	MeqExpr* ej11r = new MeqStoredParmPolc ("EJ11." + ejname1 + nm,
						j+1, i+1,
						&itsMEP);
	MeqExpr* ej11i = new MeqStoredParmPolc ("EJ11." + ejname2 + nm,
						j+1, i+1,
						&itsMEP);
	MeqExpr* ej12r = new MeqStoredParmPolc ("EJ12." + ejname1 + nm,
						j+1, i+1,
						&itsMEP);
	MeqExpr* ej12i = new MeqStoredParmPolc ("EJ12." + ejname2 + nm,
						j+1, i+1,
						&itsMEP);
	MeqExpr* ej21r = new MeqStoredParmPolc ("EJ21." + ejname1 + nm,
						j+1, i+1,
						&itsMEP);
	MeqExpr* ej21i = new MeqStoredParmPolc ("EJ21." + ejname2 + nm,
						j+1, i+1,
						&itsMEP);
	MeqExpr* ej22r = new MeqStoredParmPolc ("EJ22." + ejname1 + nm,
						j+1, i+1,
						&itsMEP);
	MeqExpr* ej22i = new MeqStoredParmPolc ("EJ22." + ejname2 + nm,
						j+1, i+1,
						&itsMEP);
	if (asAP) {
	  MeqExpr* ej11 = new MeqExprAPToComplex (ej11r, ej11i);
	  MeqExpr* ej12 = new MeqExprAPToComplex (ej12r, ej12i);
	  MeqExpr* ej21 = new MeqExprAPToComplex (ej21r, ej21i);
	  MeqExpr* ej22 = new MeqExprAPToComplex (ej22r, ej22i);
	  vec.push_back (new MeqJonesNode (ej11, ej12, ej21, ej22));
	} else {
	  MeqExpr* ej11 = new MeqExprToComplex (ej11r, ej11i);
	  MeqExpr* ej12 = new MeqExprToComplex (ej12r, ej12i);
	  MeqExpr* ej21 = new MeqExprToComplex (ej21r, ej21i);
	  MeqExpr* ej22 = new MeqExprToComplex (ej22r, ej22i);
	  vec.push_back (new MeqJonesNode (ej11, ej12, ej21, ej22));
	}
      }
      itsLSSExpr[i] = new MeqLofarStatSources (vec, itsStatSrc[i]);
    }
  }    

  // Make an expression for each baseline.
  itsExpr.resize (itsBaselines.size());
  itsResExpr.resize (itsBaselines.size());
  // Create the histogram object for couting of used #cells in time and freq
  itsCelltHist.resize (itsBaselines.size());
  itsCellfHist.resize (itsBaselines.size());
  int nrant = itsBLIndex.nrow();
  for (int ant2=0; ant2<nrant; ant2++) {
    for (int ant1=0; ant1<nrant; ant1++) {
      int blindex = itsBLIndex(ant1,ant2);
      if (blindex >= 0) {
	// Create the DFT kernel.
	itsResExpr[blindex] = new MeqLofarPoint (&itsSources,
						 itsLSSExpr[ant1],
						 itsLSSExpr[ant2],
						 &itsCelltHist[blindex],
						 &itsCellfHist[blindex]);
	itsExpr[blindex] = new MeqWsrtInt (itsResExpr[blindex],
					   itsStatExpr[ant1],
					   itsStatExpr[ant2]);
      }
    }
  }
}


//----------------------------------------------------------------------
//
// ~initParms
//
// Initialize all parameters in the expression tree by calling
// its initDomain method.
//
//----------------------------------------------------------------------
void Prediffer::initParms (const MeqDomain& domain, bool readPolcs)
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  itsIsParmSolvable.resize (parmList.size());
  int i = 0;
  itsNrScid = 0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    itsIsParmSolvable[i] = false;
    if (*iter) {
      if (readPolcs) {
        (*iter)->readPolcs (domain);
      }

      int nr = (*iter)->initDomain (domain, itsNrScid);
      if (nr > 0) {
	itsIsParmSolvable[i] = true;
	itsNrScid += nr;
      }
    }
    i++;
  }
  // Unlock the parm tables.
  itsMEP.unlock();
  itsGSMMEP.unlock();
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
void Prediffer::setTimeInterval (double secInterval)
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
void Prediffer::resetIterator()
{
  LOG_TRACE_FLOW( "resetTimeIterator" );
  itsTimeIndex = 0;
}

//----------------------------------------------------------------------
//
// ~nextInterval
//
// Move to the next interval (domain).
// Set the request belonging to that.
//
//----------------------------------------------------------------------
bool Prediffer::nextInterval(bool callReadPolcs)
{
  NSTimer mapTimer;
  mapTimer.start();
  itsDataMap->unmapFile();

  cout << "BBSTest: EndOfInterval" << endl;

  // Exit when no more chunks.
  if (itsTimeIndex >= itsTimes.nelements()) {
    return false;
  }
  cout << "BBSTest: BeginOfInterval" << endl;
  
  double timeSize = 0;
  double timeStart = 0;
  double timeStep = 0;
  itsNrTimes = 0;
  // Get the next chunk until the time interval size is exceeded.
  while (timeSize < itsTimeInterval  &&  itsTimeIndex < itsTimes.nelements())
  {
      // If first time, calculate interval and start time.
    if (timeStart == 0) {
      timeStep  = itsIntervals[itsTimeIndex];
      timeStart = itsTimes[itsTimeIndex] - timeStep/2;
    }
    timeSize += timeStep;
    itsNrTimes++;
    itsTimeIndex++;
  }
  
  // Map the correct data subset (this time interval)
  long long startOffset = (itsTimeIndex-itsNrTimes)*itsNrBl*itsNrChan*itsNPol*sizeof(fcomplex);
  size_t nrBytes = itsNrTimes*itsNrBl*itsNrChan*itsNPol*sizeof(fcomplex);
  // Map this time interval
  itsDataMap->mapFile(startOffset, nrBytes); 
  if (itsLockMappedMem) {
    // Make sure mapped data is resident in RAM
    itsDataMap->lockMappedMemory();
  }

  mapTimer.stop();
  cout << "BBSTest: file-mapping " << mapTimer << endl;

  NSTimer parmTimer;
  parmTimer.start();
  itsSolveDomain = MeqDomain(timeStart, timeStart + itsNrTimes*timeStep,
			     itsStartFreq + itsFirstChan*itsStepFreq,
			     itsStartFreq + (itsLastChan+1)*itsStepFreq);
  initParms (itsSolveDomain, callReadPolcs);
  parmTimer.stop();
  cout << "BBSTest: initparms    " << parmTimer << endl;

//   itsSolveColName = itsDataColName;
  return true;
}


//----------------------------------------------------------------------
//
// ~clearSolvableParms
//
// Clear the solvable flag on all parms (make them non-solvable).
//
//----------------------------------------------------------------------
void Prediffer::clearSolvableParms()
{
  LOG_TRACE_FLOW( "clearSolvableParms" );

  const vector<MeqParm*>& parmList = MeqParm::getParmList();
  itsSolvableParms.resize(0);

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
      //cout << "clearSolvable: " << (*iter)->getName() << endl;
      (*iter)->setSolvable(false);
    }
  }
}

//----------------------------------------------------------------------
//
// ~setSolvableParms
//
// Set the solvable flag (true or false) on all parameters whose
// name matches the parmPatterns pattern.
//
//----------------------------------------------------------------------
void Prediffer::setSolvableParms (vector<string>& parms,
				      vector<string>& excludePatterns,
				      bool isSolvable)
{
  LOG_INFO_STR( "setSolvableParms: "
		<< "isSolvable = " << isSolvable);

  itsSolvableParms.resize(parms.size());
  for (unsigned int i = 0; i < parms.size(); i++)
  {
    itsSolvableParms[i] = parms[i];
  }
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  // Convert patterns to regexes.
  vector<Regex> parmRegex;
  for (unsigned int i=0; i<itsSolvableParms.nelements(); i++) {
    parmRegex.push_back (Regex::fromPattern(itsSolvableParms[i]));
  }

  vector<Regex> excludeRegex;
  for (unsigned int i=0; i<excludePatterns.size(); i++) {
    excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
  }

  //
  // Find all parms matching the itsSolvableParms
  // Exclude them if matching an excludePattern
  //
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    String parmName ((*iter)->getName());

    for (vector<Regex>::const_iterator incIter = parmRegex.begin();
	 incIter != parmRegex.end();
	 incIter++)
    {
      {
	if (parmName.matches(*incIter))
	{
	  bool parmExc = false;
	  for (vector<Regex>::const_iterator excIter = excludeRegex.begin();
	       excIter != excludeRegex.end();
	       excIter++)
	  {
	    if (parmName.matches(*excIter))
	    {
	      parmExc = true;
	      break;
	    }
	  }
	  if (!parmExc) {
	    LOG_TRACE_OBJ_STR( "setSolvable: " << (*iter)->getName());
	    (*iter)->setSolvable(isSolvable);
	  }
	  break;
	}
      }
    }
  }
}

vector<MeqResult> getCondeq (const MeqRequest& request,
			     int bl, int ant1, int ant2,
			     bool showd)
{
  /*
  NSTimer predTimer, eqTimer;
  unsigned int blOffset = bl*itsNrChan*itsNPol;
  unsigned int freqOffset = itsFirstChan*itsNPol;
  // Set the data pointer
  fcomplex* dataStart = (fcomplex*)itsDataMap->getStart();
  ASSERTSTR(dataStart!=0, "No memory region mapped. Call map(..) first."
	    << " Perhaps you have forgotten to call nextInterval().");
  fcomplex* data = dataStart + timeOffset + blOffset + freqOffset;
  
  ASSERT (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
	  &&  itsBLIndex(ant1,ant2) >= 0);
  int blindex = itsBLIndex(ant1,ant2);
  ///    if (MeqPointDFT::doshow) {
    ///      cout << "Info: " << ant1 << ' ' << ant2 << ' ' << blindex << endl;
    ///    }

  MeqJonesExpr& expr = *(itsExpr[blindex]);
  predTimer.start();
  expr.calcResult (request);         // This is the actual predict
  predTimer.stop();

  // Form the equations for this row.
  // Make a default derivative vector with values 0.
  MeqResult condeq[4];
  ASSERT (itsNPol <= 4);
  for (int i=0; i<itsNPol; i++) {
    condeq[i] = MeqResult(nspid);
  }

  MeqMatrix defaultDeriv (DComplex(0,0), 1, nrchan);
  const complex<double>* defaultDerivPtr = defaultDeriv.dcomplexStorage();
    
  // >>>>>>>>>>>>>> Diff 
  // Calculate the derivatives and get pointers to them.
  // Use the default if no perturbed value defined.                    //>>>> uses predicted data
  vector<const complex<double>*> derivs(itsNPol*itsNrScid);
  bool foundDeriv = false;
  if (showd) {
    cout << "xx val " << expr.getResult11().getValue() << endl;;
    cout << "xy val " << expr.getResult12().getValue() << endl;;
    cout << "yx val " << expr.getResult21().getValue() << endl;;
    cout << "yy val " << expr.getResult22().getValue() << endl;;
  }
  for (int scinx=0; scinx<itsNrScid; scinx++) {
    MeqMatrix val;
    if (expr.getResult11().isDefined(scinx)) {
      val = expr.getResult11().getPerturbedValue(scinx);
      if (showd) {
	cout << "xx" << scinx << ' ' << val;
      }
      val -= expr.getResult11().getValue();
      if (showd) {
	cout << "  diff=" << val;
      }
      ///cout << "Diff  " << val << endl;
      val /= expr.getResult11().getPerturbation(scinx);
      if (showd) {
	cout << "  der=" << val << endl;
      }
      ///cout << "Deriv " << val << endl;
      derivs[scinx] = val.dcomplexStorage();
      foundDeriv = true;
    } else {
      derivs[scinx] = defaultDerivPtr;
    }
    if (itsNPol == 4) {
      if (expr.getResult12().isDefined(scinx)) {
	val = expr.getResult12().getPerturbedValue(scinx);
	if (showd) {
	  cout << "xy" << scinx << ' ' << val;
	}
	val -= expr.getResult12().getValue();
	if (showd) {
	  cout << "  diff=" << val;
	}
	val /= expr.getResult12().getPerturbation(scinx);
	if (showd) {
	  cout << "  der=" << val << endl;
	}
	derivs[scinx + itsNrScid] = val.dcomplexStorage();
	foundDeriv = true;
      } else {
	derivs[scinx + itsNrScid] = defaultDerivPtr;
      }
      if (expr.getResult21().isDefined(scinx)) {
	val = expr.getResult21().getPerturbedValue(scinx);
	if (showd) {
	  cout << "yx" << scinx << ' ' << val;
	}
	val -= expr.getResult21().getValue();
	if (showd) {
	  cout << "  diff=" << val;
	}
	val /= expr.getResult21().getPerturbation(scinx);
	if (showd) {
	  cout << "  der=" << val << endl;
	}
	derivs[scinx + 2*itsNrScid] = val.dcomplexStorage();
	foundDeriv = true;
      } else {
	derivs[scinx + 2*itsNrScid] = defaultDerivPtr;
      }
    }
    if (itsNPol > 1) {
      if (expr.getResult22().isDefined(scinx)) {
	val = expr.getResult22().getPerturbedValue(scinx);
	if (showd) {
	  cout << "yy" << scinx << ' ' << val;
	}
	val -= expr.getResult22().getValue();
	if (showd) {
	  cout << "  diff=" << val;
	}
	val /= expr.getResult22().getPerturbation(scinx);
	if (showd) {
	  cout << "  der=" << val << endl;
	}
	derivs[scinx + (itsNPol-1)*itsNrScid] = val.dcomplexStorage();
	foundDeriv = true;
      } else {
	derivs[scinx + (itsNPol-1)*itsNrScid] = defaultDerivPtr;
      }
    }
  }
    
  // Only add to solver if at least one derivative was found.
  // Otherwise these data are not dependent on the solvable parameters.
  if (foundDeriv) {
    // Get pointer to array storage; the data in it is contiguous.    //>>>>>> uses MS data
    Complex* dataPtr = data;
    
    // 	  cout << "First data element for time=" << time
    // 	       << ", ant1=" << ant1 << ", ant2=" << ant2
    // 	       << " : " << *dataPtr << endl;
    
    vector<double> derivVec(2*itsNrScid);
    double* derivReal = &(derivVec[0]);
    double* derivImag = &(derivVec[itsNrScid]);
    // Fill in all equations.
    if (itsNPol == 1) {
      {
	const MeqMatrix& xx = expr.getResult11().getValue();
	for (int i=0; i<nrchan; i++) {
	  for (int j=0; j<itsNrScid; j++) {
	    derivReal[j] = derivs[j][i].real();
	    derivImag[j] = derivs[j][i].imag();
	  }
	  DComplex diff (dataPtr[i].real(), dataPtr[i].imag());
	  diff -= xx.getDComplex(0,i);
	  double val = diff.real();
	  itsSolver.makeNorm (derivReal, 1., val);
	  val = diff.imag();
	  itsSolver.makeNorm (derivImag, 1., val);
	  nrpoint++;
	}
      }
    } else if (itsNPol == 2) {
      {
	const MeqMatrix& xx = expr.getResult11().getValue();
	for (int i=0; i<nrchan; i++) {
	  for (int j=0; j<itsNrScid; j++) {
	    derivReal[j] = derivs[j][i].real();
	    derivImag[j] = derivs[j][i].imag();
	  }
	  DComplex diff (dataPtr[i*2].real(), dataPtr[i*2].imag());
	  diff -= xx.getDComplex(0,i);
	  double val = diff.real();
	  itsSolver.makeNorm (derivReal, 1., val);
	  val = diff.imag();
	  itsSolver.makeNorm (derivImag, 1., val);
	  nrpoint++;
	}
      }
      {
	const MeqMatrix& yy = expr.getResult22().getValue();
	for (int i=0; i<nrchan; i++) {
	  for (int j=0; j<itsNrScid; j++) {
	    derivReal[j] = derivs[j+itsNrScid][i].real();
	    derivImag[j] = derivs[j+itsNrScid][i].imag();
	  }
	  DComplex diff (dataPtr[i*2+1].real(), dataPtr[i*2+1].imag());
	  diff -= yy.getDComplex(0,i);
	  double val = diff.real();
	  itsSolver.makeNorm (derivReal, 1., val);
	  val = diff.imag();
	  itsSolver.makeNorm (derivImag, 1., val);
	  nrpoint++;
	}
      }
    } else if (itsNPol == 4) {
      {
	const MeqMatrix& xx = expr.getResult11().getValue();
	for (int i=0; i<nrchan; i++) {
	  for (int j=0; j<itsNrScid; j++) {
	    derivReal[j] = derivs[j][i].real();
	    derivImag[j] = derivs[j][i].imag();
	    ///cout << derivReal[j] << ' ' << derivImag[j] << ", ";
	    if (showd) {
	      cout << "derxx: " << j << ' '
		   << derivReal[j] << ' ' << derivImag[j] << endl;
	    }
	  }
	  ///cout << endl;
	  DComplex diff (dataPtr[i*4].real(), dataPtr[i*4].imag());
	  ///cout << "Value " << diff << ' ' << xx.getDComplex(0,i) << endl;
	  diff -= xx.getDComplex(0,i);
	  if (showd) {
	    cout << "diffxx: " << i << ' '
		 << diff.real() << ' ' << diff.imag() << endl;
	  }
	  double val = diff.real();
	  itsSolver.makeNorm (derivReal, 1., val);
	  val = diff.imag();
	  itsSolver.makeNorm (derivImag, 1., val);
	  nrpoint++;
	}
      }
      {
	const MeqMatrix& xy = expr.getResult12().getValue();
	for (int i=0; i<nrchan; i++) {
	  for (int j=0; j<itsNrScid; j++) {
	    derivReal[j] = derivs[j+itsNrScid][i].real();
	    derivImag[j] = derivs[j+itsNrScid][i].imag();
	    if (showd) {
	      cout << "derxy: " << i << ' '
		   << derivReal[j] << ' ' << derivImag[j] << endl;
	    }
	  }
	  DComplex diff (dataPtr[i*4+1].real(), dataPtr[i*4+1].imag());
	  diff -= xy.getDComplex(0,i);
	  if (showd) {
	    cout << "diffxy: " << i << ' '
		 << diff.real() << ' ' << diff.imag() << endl;
	  }
	  double val = diff.real();
	  itsSolver.makeNorm (derivReal, 1., val);
	  val = diff.imag();
	  itsSolver.makeNorm (derivImag, 1., val);
	  nrpoint++;
	}
      }
      {
	const MeqMatrix& yx = expr.getResult21().getValue();
	for (int i=0; i<nrchan; i++) {
	  for (int j=0; j<itsNrScid; j++) {
	    derivReal[j] = derivs[j+2*itsNrScid][i].real();
	    derivImag[j] = derivs[j+2*itsNrScid][i].imag();
	    if (showd) {
	      cout << "deryx: " << i << ' '
		   << derivReal[j] << ' ' << derivImag[j] << endl;
	    }
	  }
	  DComplex diff (dataPtr[i*4+2].real(), dataPtr[i*4+2].imag());
	  diff -= yx.getDComplex(0,i);
	  if (showd) {
	    cout << "diffyx: " << i << ' '
		 << diff.real() << ' ' << diff.imag() << endl;
	  }
	  double val = diff.real();
	  itsSolver.makeNorm (derivReal, 1., val);
	  val = diff.imag();
	  itsSolver.makeNorm (derivImag, 1., val);
	  nrpoint++;
	}
      }
      {
	const MeqMatrix& yy = expr.getResult22().getValue();
	for (int i=0; i<nrchan; i++) {
	  for (int j=0; j<itsNrScid; j++) {
	    derivReal[j] = derivs[j+3*itsNrScid][i].real();
	    derivImag[j] = derivs[j+3*itsNrScid][i].imag();
	    if (showd) {
	      cout << "deryy: " << i << ' '
		   << derivReal[j] << ' ' << derivImag[j] << endl;
	    }
	  }
	  DComplex diff (dataPtr[i*4+3].real(), dataPtr[i*4+3].imag());
	  diff -= yy.getDComplex(0,i);
	  if (showd) {
	    cout << "diffyy: " << i << ' '
		 << diff.real() << ' ' << diff.imag() << endl;
	  }
	  double val = diff.real();
	  itsSolver.makeNorm (derivReal, 1., val);
	  val = diff.imag();
	  itsSolver.makeNorm (derivImag, 1., val);
	  nrpoint++;
	}
      }
    } else {
      throw AipsError("Number of polarizations should be 1, 2, or 4");
    }
  }
  */
}


//----------------------------------------------------------------------
//
// ~saveResidualData
//
// Save the colA - colB in residualCol.
//
//----------------------------------------------------------------------
void Prediffer::saveResidualData()
{
  if (itsDataMap->getFileName() != itsMSName+".res")
  {
    // copy file <msname>.dat to <msname>.res
    ifstream dataFile((itsDataMap->getFileName()).c_str()); 
    string resFileName(itsMSName+".res");
    ofstream resFile(resFileName.c_str());
    resFile << dataFile.rdbuf();
    dataFile.close();
    resFile.close();

    // Use residuals file for further solving
    delete itsDataMap;
    itsDataMap = new MMap(resFileName, MMap::ReWr);
  }

  cout << "saveResidualData to '" << itsDataMap->getFileName() << "'" << endl;

  // Map the correct data subset (this time interval) for writing
  long long startOffset = (itsTimeIndex-itsNrTimes)*itsNrBl*itsNrChan*itsNPol*sizeof(fcomplex);
  size_t nrBytes = itsNrTimes*itsNrBl*itsNrChan*itsNPol*sizeof(fcomplex);
  itsDataMap->mapFile(startOffset, nrBytes);
  if (itsLockMappedMem) {
    // Make sure mapped data is resident in RAM
    itsDataMap->lockMappedMemory();
  }

  vector<int> src(itsPeelSourceNrs.nelements());
  cout << "Using peel sources ";
  for (unsigned int i=0; i<src.size(); i++) {
    src[i] = itsPeelSourceNrs[i];
    cout << src[i] << ',';
  }
  cout << endl;
  itsSources.setSelected (src);

  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  int nrchan = 1+itsLastChan-itsFirstChan;

  // Loop over all times in current time interval.
  for (unsigned int tStep=0; tStep<itsNrTimes; tStep++)
  {
    unsigned int timeOffset = tStep*itsNrBl*itsNrChan*itsNPol;
    double time = itsTimes[itsTimeIndex-itsNrTimes+tStep];
    double interv = itsIntervals[itsTimeIndex-itsNrTimes+tStep];

    MeqDomain domain(time-interv/2, time+interv/2, startFreq, endFreq);
    MeqRequest request(domain, 1, nrchan, itsNrScid);
    
    for (unsigned int bl=0; bl<itsNrBl; bl++)
    {
      uInt ant1 = itsAnt1Data(bl);
      uInt ant2 = itsAnt2Data(bl);
      if (itsBLSelection(ant1,ant2) == true)
      {
        unsigned int blOffset = bl*itsNrChan*itsNPol;
        unsigned int freqOffset = itsFirstChan*itsNPol;
        // Set the data pointer
        fcomplex* dataStart = (fcomplex*)itsDataMap->getStart();

        ASSERTSTR(dataStart!=0, "No memory region mapped. Call map(..) first."
                  << " Perhaps you have forgotten to call nextInterval().");
        fcomplex* dataPtr = dataStart + timeOffset + blOffset + freqOffset;

	ASSERT (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
		&&  itsBLIndex(ant1,ant2) >= 0);
	int blindex = itsBLIndex(ant1,ant2);

	itsExpr[blindex]->calcResult (request);
	
	// Create a matrix from selected frequencies and all polarisations
	Matrix<Complex> data(IPosition(2, itsNPol, nrchan), dataPtr, SHARE);

	Slice sliceFreq(0, nrchan);
	// Convert the DComplex predicted results to a Complex data array.
	Matrix<Complex> tmpPredict(IPosition(2,1,nrchan));
	convertArray (tmpPredict,
		      itsExpr[blindex]->getResult11().getValue().getDComplexMatrix());

	// Subtract data for all frequencies of polarisation 1
	Matrix<Complex> dataPol0 (data(Slice(0,1), sliceFreq));
	dataPol0 -= tmpPredict;

	if (4 == itsNPol) 
	{
	  // Subtract data for all frequencies of polarisation 2
	  convertArray (tmpPredict,
			itsExpr[blindex]->getResult12().getValue().getDComplexMatrix());
	  Matrix<Complex> dataPol1 (data(Slice(1,1), sliceFreq));
	  dataPol1 -= tmpPredict;

	  // Subtract data for all frequencies of polarisation 3
	  convertArray (tmpPredict,
			itsExpr[blindex]->getResult21().getValue().getDComplexMatrix());
	  Matrix<Complex> dataPol2 (data(Slice(2,1), sliceFreq));
	  dataPol2 -= tmpPredict;

	  // Subtract data for all frequencies of polarisation 4
	  convertArray (tmpPredict,
			itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
	  Matrix<Complex> dataPol3 (data(Slice(3,1), sliceFreq));
	  dataPol3 -= tmpPredict;
	} 
	else if (2 == itsNPol)
	{
	  // Subtract data for all frequencies of polarisation 2
	  convertArray (tmpPredict,
			itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
	  Matrix<Complex> dataPol1 (data(Slice(1,1), sliceFreq));
	  dataPol1 -= tmpPredict;
	} 
	else if (1 != itsNPol)
	{
	  throw AipsError("Number of polarizations should be 1, 2, or 4");
	}
	///    if (MeqPointDFT::doshow) cout << "result: " << data << endl;

      } // End if (itsBLSelection(ant1,ant2) == ...)
    } // End loop bl
  } // End loop tStep

  // Make sure file is updated
  itsDataMap->unmapFile();

}


//----------------------------------------------------------------------
//
// ~select
//
// Select a subset of the MS data.                                        >>>>>>>> Keep this, possibility to use a subset of data (it sets itsFirstChan o.a.)
// The baselines and the channels (frequency window) to be used can be specified.
// This selection has to be done before the loop over domains.
//
//----------------------------------------------------------------------
void Prediffer::select (const vector<int>& ant1, 
			const vector<int>& ant2, 
			int firstChan, int lastChan)
{
  if (firstChan < 0  ||  firstChan >= itsNrChan) {
    itsFirstChan = 0;
  } else {
    itsFirstChan = firstChan;
  }
  if (lastChan < 0  ||  lastChan >= itsNrChan) {
    itsLastChan = itsNrChan-1;
  } else {
    itsLastChan = lastChan;
  }
  ASSERT (itsFirstChan <= itsLastChan);

  ASSERT ( ant1.size() == ant2.size());
  itsBLSelection = false;
//   if (ant1.size() == 0)  // If no baselines specified, select all baselines
//   {
//     itsBLSelection = true;
//   }
//   else
//   {
    for (unsigned int i=0; i<ant1.size(); i++)
    {
      ASSERT(ant1[i] < int(itsBLSelection.nrow()));
      for (unsigned int j=0; j<ant2.size(); j++)
      {
	ASSERT(ant2[i] < int(itsBLSelection.nrow()));
	itsBLSelection(ant1[i], ant2[j]) = true;     // Select baseline subset
      }
      //    }
  }
  
}

//----------------------------------------------------------------------
//
// ~fillUVW
//
// Calculate the station UVW coordinates from the MS.
//
//----------------------------------------------------------------------
void Prediffer::fillUVW()
{
  LOG_TRACE_RTTI( "get UVW coordinates from MS" );
  int nant = itsStatUVW.size();
  vector<bool> statFnd (nant);
  vector<bool> statDone (nant);
  vector<double> statuvw(3*nant);

  // Determine the number of stations (found)
  statFnd.assign (statFnd.size(), false);
  int nStatFnd = 0;
  for (unsigned int bl=0; bl < itsNrBl; bl++)
  {
    int a1 = itsAnt1Data[bl];
    int a2 = itsAnt2Data[bl];
    if (itsBLSelection(a1,a2) == true)
    {
      if (!statFnd[itsAnt1Data[bl]]) {
	nStatFnd++;
	statFnd[itsAnt1Data[bl]] = true;
      }
      if (!statFnd[itsAnt2Data[bl]]) {
	nStatFnd++;
	statFnd[itsAnt2Data[bl]] = true;
      }
    }
  }

  // Map uvw data into memory
  size_t nrBytes = itsTimes.nelements() * itsNrBl * 3 * sizeof(double);
  double* uvwDataPtr = 0;
  MMap* mapPtr = new MMap(itsMSName+".uvw", MMap::Read);
  mapPtr->mapFile(0, nrBytes);
  if (itsLockMappedMem) {
    // Make sure mapped data is resident in RAM
    mapPtr->lockMappedMemory();
  }   
  uvwDataPtr = (double*)mapPtr->getStart();

  // Step time by time through the MS.
  for (unsigned int tStep=0; tStep < itsTimes.nelements(); tStep++)
  {
    // Set uvw pointer to beginning of this time
    unsigned int tOffset = tStep * itsNrBl * 3;
    double* uvw = uvwDataPtr + tOffset;

    double time = itsTimes[tStep];
    
    // Set UVW of first station to 0 (UVW coordinates are relative!).
    statDone.assign (statDone.size(), false);
    statuvw[3*itsAnt1Data[0]]   = 0;
    statuvw[3*itsAnt1Data[0]+1] = 0;
    statuvw[3*itsAnt1Data[0]+2] = 0;
    statDone[itsAnt1Data[0]] = true;
    itsStatUVW[itsAnt1Data[0]]->set (time, 0, 0, 0);

//     cout << "itsStatUVW[" << itsAnt1Data[0] << "] time: " << time << " 0, 0, 0" << endl;

    int ndone = 1;
    // Loop until all found stations are handled. This is necessary when not all 
    // stations can be calculated in one loop (depends on the order)
    while (ndone < nStatFnd) 
    {
      int nd = 0;
      // Loop over baselines
      for (unsigned int bl=0; bl < itsNrBl; bl++)
      {
	int a1 = itsAnt1Data[bl];
	int a2 = itsAnt2Data[bl];
	if (itsBLSelection(a1,a2) == true)
	{
	  if (!statDone[a2]) {
	    if (statDone[a1]) {
	      statuvw[3*a2]   = uvw[3*bl]   - statuvw[3*a1];
	      statuvw[3*a2+1] = uvw[3*bl+1] - statuvw[3*a1+1];
	      statuvw[3*a2+2] = uvw[3*bl+2] - statuvw[3*a1+2];
	      statDone[a2] = true;
	      itsStatUVW[a2]->set (time, statuvw[3*a2], statuvw[3*a2+1],
				   statuvw[3*a2+2]);

// 	      cout << "itsStatUVW[" << a2 << "] time: " << time << statuvw[3*a2] << " ," << statuvw[3*a2+1] << " ," << statuvw[3*a2+2] << endl;

	      ndone++;
	      nd++;
	    }
	  } else if (!statDone[a1]) {
	    if (statDone[a2]) {
	      statuvw[3*a1]   = statuvw[3*a2]   - uvw[3*bl];
	      statuvw[3*a1+1] = statuvw[3*a2+1] - uvw[3*bl+1];
	      statuvw[3*a1+2] = statuvw[3*a2+2] - uvw[3*bl+2];
	      statDone[a1] = true;
	      itsStatUVW[a1]->set (time, statuvw[3*a1], statuvw[3*a1+1],
				   statuvw[3*a1+2]);
// 	      cout << "itsStatUVW[" << a1 << "] time: " << time << statuvw[3*a1] << " ," << statuvw[3*a1+1] << " ," << statuvw[3*a1+2] << endl;

	      ndone++;
	      nd++;
	    }
	  }
	  if (ndone == nStatFnd) {
	    break;
	  }

	} // End if (itsBLSelection(ant1,ant2) ==...

      } // End loop baselines

      //	  ASSERT (nd > 0);

    } // End loop stations found
  } // End loop time

  // Finished with map
  delete mapPtr;
}


//----------------------------------------------------------------------
//
// ~peel
//
// Define the source numbers to use in a peel step.
//
//----------------------------------------------------------------------
Bool Prediffer::peel(const vector<int>& peelSources,
			 const vector<int>& extraSources)
{
  Vector<Int> peelSourceNrs(peelSources.size());
  for (unsigned int i=0; i<peelSources.size(); i++)
  {
    peelSourceNrs[i] = peelSources[i];
  }
  Vector<Int> extraSourceNrs(extraSources.size());
  for (unsigned int i=0; i<extraSources.size(); i++)
  {
    extraSourceNrs[i] = extraSources[i];
  }

  // Make a shallow copy to get a non-const object.  
  Vector<Int> tmpPeel(peelSourceNrs);
  Vector<Int> sourceNrs;
  if (extraSourceNrs.nelements() == 0) {
    sourceNrs.reference (tmpPeel);
  } else {
    sourceNrs.resize (peelSourceNrs.nelements() + extraSourceNrs.nelements());
    sourceNrs(Slice(0,peelSourceNrs.nelements())) = peelSourceNrs;
    sourceNrs(Slice(peelSourceNrs.nelements(), extraSourceNrs.nelements())) =
      extraSourceNrs;
  }
  LOG_TRACE_OBJ_STR( "peel: sources " << tmpPeel << " predicting sources "
	    << sourceNrs );

  ASSERT (peelSourceNrs.nelements() > 0);
  vector<int> src(sourceNrs.nelements());
  for (unsigned int i=0; i<src.size(); i++) {
    src[i] = sourceNrs[i];
    LOG_TRACE_OBJ_STR( "Predicting source " << sourceNrs[i] );
  }
  itsSources.setSelected (src);
  itsPeelSourceNrs.reference (tmpPeel);
  return True;
}


void Prediffer::getParmValues (vector<string>& names,
			       vector<double>& values)
{
  vector<MeqMatrix> vals;
  getParmValues (names, vals);
  values.resize (0);
  values.reserve (vals.size());
  for (vector<MeqMatrix>::const_iterator iter = vals.begin();
       iter != vals.end();
       iter++) {
    ASSERT (iter->nelements() == 1);
    values.push_back (iter->getDouble());
  }
}

void Prediffer::getParmValues (vector<string>& names,
			       vector<MeqMatrix>& values)
{
  MeqMatrix val;
  names.resize (0);
  values.resize (0);
  // Iterate through all parms and get solvable ones.
  const vector<MeqParm*>& parmList = MeqParm::getParmList();
  int i=0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if ((*iter)->isSolvable()) {
      names.push_back ((*iter)->getName());
      (*iter)->getCurrentValue (val, false);
      values.push_back (val);
    }
    i++;
  }
}

void Prediffer::setParmValues (const vector<string>& names,
			       const vector<double>& values)
{
  vector<MeqMatrix> vals;
  vals.reserve (values.size());
  for (vector<double>::const_iterator iter = values.begin();
       iter != values.end();
       iter++) {
    vals.push_back (MeqMatrix (*iter));
  }
  setParmValues (names, vals);
}

void Prediffer::setParmValues (const vector<string>& names,
			       const vector<MeqMatrix>& values)
{
  ASSERT (names.size() == values.size());
  // Iterate through all parms and get solvable ones.
  const vector<MeqParm*>& parmList = MeqParm::getParmList();
  int i;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    const string& pname = (*iter)->getName();
    i = 0;
    for (vector<string>::const_iterator itern = names.begin();
	 itern != names.end();
	 itern++) {
      if (*itern == pname) {
	const MeqParmPolc* ppc = dynamic_cast<const MeqParmPolc*>(*iter);
	ASSERT (ppc);
	MeqParmPolc* pp = const_cast<MeqParmPolc*>(ppc);
	const vector<MeqPolc>& polcs = pp->getPolcs();
	ASSERT (polcs.size() == 1);
	MeqPolc polc = polcs[0];
	polc.setCoeffOnly (values[i]);
	pp->setPolcs (vector<MeqPolc>(1,polc));
	break;
      }
      i++;
      // A non-matching name is ignored; no warning is given.
    }
  }
}

void Prediffer::showSettings() const
{
  cout << "Prediffer settings:" << endl;
  cout << "  msname:    " << itsMSName << endl;
  cout << "  mepname:   " << itsMEPName << endl;
  cout << "  gsmname:   " << itsGSMMEPName << endl;
  cout << "  time interval: " << itsTimeInterval << endl;
  cout << "  solvparms: " << itsSolvableParms << endl;
  cout << "  stchan:    " << itsFirstChan << endl;
  cout << "  endchan:   " << itsLastChan << endl;
  cout << "  calcuvw  : " << itsCalcUVW << endl;
  cout << "  antennas : " ;
  for (unsigned int i=0; i<itsSelAnt.size() ; i++)
  {
    cout << itsSelAnt[i] << ", ";
  }
  cout << endl;
  cout << endl;
}

void Prediffer::updateSolvableParms()
{

}

std::list<MeqResult> Prediffer::getEquations ()
{
  LOG_TRACE_FLOW("Prediffer::getEquations");
}

} // namespace LOFAR
