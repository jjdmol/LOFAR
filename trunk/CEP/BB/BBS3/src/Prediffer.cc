//# Prediffer.cc: Read and predict read visibilities
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
#include <BBS3/MMap.h>
#include <BBS3/FlagsMap.h>
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
#include <Common/BlobIStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/BlobArray.h>
#include <Common/VectorUtil.h>
#include <Common/DataConvert.h>
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

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <malloc.h>

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
  itsNCorr        (0),
  itsNrBl         (0),
  itsTimeIndex    (0),
  itsNrTimes      (0),
  itsNrTimesDone  (0),
  itsBlNext       (0),
  itsDataMap      (0),
  itsFlagsMap     (0),
  itsLockMappedMem(lockMappedMem)
{
  LOG_INFO_STR( "Prediffer constructor ("
		<< "'" << msName   << "', "
		<< "'" << meqModel << "', "
		<< "'" << skyModel << "', "
		<< itsCalcUVW << ")" );

  // Initially use all correlations.
  for (int i=0; i<4; ++i) {
    itsCorr[i] = true;
  }
  // Read the meta data and map files.
  readDescriptiveData (msName);
  fillStations (ant);                  // Selected antennas
  fillBaselines (ant);
  itsDataMap  = new MMap(msName + "/vis.dat", MMap::Read);
  itsFlagsMap = new FlagsMap(msName + "/vis.flg", MMap::Read);

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
  LOG_TRACE_FLOW( "Prediffer destructor" );

  itsParmGroup.clear();         // Delete all parameters

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
  for (vector<MeqExpr*>::iterator iter = itsComplexConv.begin();
       iter != itsComplexConv.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqJonesNode*>::iterator iter = itsJonesNodes.begin();
       iter != itsJonesNodes.end();
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
  delete itsFlagsMap;

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
  // Get meta data from description file.
  string name(fileName+"/vis.des");
  cout << fileName << endl;
  std::ifstream istr(name.c_str());
  BlobIBufStream bbs(istr);
  BlobIStream bis(bbs);
  bis.getStart("ms.des");
  double ra, dec;
  bis >> ra;
  bis >> dec;
  bis >> itsNCorr;
  bis >> itsNrChan;
  bis >> itsStartFreq;
  bis >> itsEndFreq;
  bis >> itsStepFreq;
  bis >> itsAnt1;
  bis >> itsAnt2;
  bis >> itsTimes;
  bis >> itsIntervals;
  bis >> itsAntPos;
  bis.getEnd();
  ASSERT (itsAnt1.size() == itsAnt2.size());
  itsNrBl = itsAnt1.size();
  getPhaseRef(ra, dec, itsTimes[0]);
  itsReverseChan = itsStartFreq > itsEndFreq;
  if (itsReverseChan) {
    double  tmp  = itsEndFreq;
    itsEndFreq   = itsStartFreq;
    itsStartFreq = tmp;
    itsStepFreq  = std::abs(itsStepFreq);
  }
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
void Prediffer::fillStations (const vector<int>& antnrs)
{
  uint nrant = itsAntPos.ncolumn();
  itsStations = vector<MeqStation*>(nrant, (MeqStation*)0);
  // Get all stations actually used.
  char str[8];
  for (uint i=0; i<antnrs.size(); i++) {
    uint ant = antnrs[i];
    ASSERT (ant < nrant);
    if (itsStations[ant] == 0) {
      // Store each position as a constant parameter.
      // Use the antenna name as the parameter name.
      Vector<Double> antpos = itsAntPos.column(ant);
      sprintf (str, "%d", ant+1);
      String name = string("SR") + str;
      MeqParmSingle* px = new MeqParmSingle ("AntPosX." + name,
					     &itsParmGroup, antpos(0));
      MeqParmSingle* py = new MeqParmSingle ("AntPosY." + name,
					     &itsParmGroup, antpos(1));
      MeqParmSingle* pz = new MeqParmSingle ("AntPosZ." + name,
					     &itsParmGroup, antpos(2));
      itsStations[ant] = new MeqStation(px, py, pz, name);
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
void Prediffer::fillBaselines (const vector<int>& antnrs)
{
  MeqDomain  domain;
  MeqRequest req(domain, 1, 1);

  // Convert antnrs to bools.
  uint maxAnt = itsStations.size();
  Vector<bool> useAnt(maxAnt);
  useAnt = false;
  for (uint i=0; i<antnrs.size(); i++) {
    uint ant = antnrs[i];
    ASSERT (ant < maxAnt);
    useAnt[ant] = true;
  }
  itsBaselines.reserve (itsAnt1.size());
  itsBLIndex.resize (maxAnt, maxAnt);
  itsBLIndex = -1;
  itsBLSelection.resize (maxAnt, maxAnt);
  itsBLSelection = false;
  
  for (uint i=0; i<itsAnt1.size(); i++) {
    uint a1 = itsAnt1[i];
    uint a2 = itsAnt2[i];
    ASSERT (a1 < maxAnt  &&  a2 < maxAnt);
    if (useAnt[a1] && useAnt[a2]) {
      // Assign a baseline number and set to selected.
      itsBLIndex(a1,a2) = itsBaselines.size();
      itsBLSelection(a1,a2) = true;
      // Create an MVBaseline object for each antenna pair.
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
  }
  countBaseCorr();
}

//----------------------------------------------------------------------
//
// ~countBaseCorr
//
// Count the selected nr of baselines and correlations
//
//----------------------------------------------------------------------
void Prediffer::countBaseCorr()
{
  itsNrSelBl = 0;
  const bool* sel = itsBLSelection.data();
  const int*  inx = itsBLIndex.data();
  for (uint i=0; i<itsBLSelection.nelements(); ++i) {
    if (sel[i]  &&  inx[i] >= 0) {
      itsNrSelBl++;
    }
  }
  ASSERTSTR (itsNrSelBl > 0, "No valid baselines selected");
  itsNSelCorr = 0;
  for (int i=0; i<itsNCorr; ++i) {
    if (itsCorr[i]) {
      itsNSelCorr++;
    }
  }
  ASSERTSTR (itsNSelCorr > 0, "No valid correlations selected");
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
  itsSources = itsGSMMEP.getPointSources (&itsParmGroup, Vector<int>());
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
					     &itsParmGroup, &itsMEP);
      MeqExpr* drot = new MeqStoredParmPolc ("drot." +
					     itsStations[i]->getName(),
					     &itsParmGroup, &itsMEP);
      MeqExpr* dell = new MeqStoredParmPolc ("dell." +
					     itsStations[i]->getName(),
					     &itsParmGroup, &itsMEP);
      MeqExpr* gain11 = new MeqStoredParmPolc ("gain.11." +
					       itsStations[i]->getName(),
					       &itsParmGroup, &itsMEP);
      MeqExpr* gain22 = new MeqStoredParmPolc ("gain.22." +
					       itsStations[i]->getName(),
					       &itsParmGroup, &itsMEP);
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
  itsSources = itsGSMMEP.getPointSources (&itsParmGroup, Vector<int>());
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
					     &itsParmGroup, &itsMEP);
      MeqExpr* drot = new MeqStoredParmPolc ("drot." +
					     itsStations[i]->getName(),
					     &itsParmGroup, &itsMEP);
      MeqExpr* dell = new MeqStoredParmPolc ("dell." +
					     itsStations[i]->getName(),
					     &itsParmGroup, &itsMEP);
      MeqExpr* gain11 = new MeqStoredParmPolc ("gain.11." +
					       itsStations[i]->getName(),
					       &itsParmGroup, &itsMEP);
      MeqExpr* gain22 = new MeqStoredParmPolc ("gain.22." +
					       itsStations[i]->getName(),
					       &itsParmGroup, &itsMEP);
      itsStatExpr[i] = new MeqStatExpr (frot, drot, dell, gain11, gain22);
      // Make an expression for all source parameters for this station.
      vector<MeqJonesExpr*> vec;
      for (int j=0; j<itsSources.size(); j++) {
	string nm = itsStations[i]->getName() + '.' +  itsSources[j].getName();
	MeqExpr* ej11r = new MeqStoredParmPolc ("EJ11." + ejname1 + nm,
						&itsParmGroup, &itsMEP);
	MeqExpr* ej11i = new MeqStoredParmPolc ("EJ11." + ejname2 + nm,
						&itsParmGroup, &itsMEP);
	MeqExpr* ej12r = new MeqStoredParmPolc ("EJ12." + ejname1 + nm,
						&itsParmGroup, &itsMEP);
	MeqExpr* ej12i = new MeqStoredParmPolc ("EJ12." + ejname2 + nm,
						&itsParmGroup, &itsMEP);
	MeqExpr* ej21r = new MeqStoredParmPolc ("EJ21." + ejname1 + nm,
						&itsParmGroup, &itsMEP);
	MeqExpr* ej21i = new MeqStoredParmPolc ("EJ21." + ejname2 + nm,
						&itsParmGroup, &itsMEP);
	MeqExpr* ej22r = new MeqStoredParmPolc ("EJ22." + ejname1 + nm,
						&itsParmGroup, &itsMEP);
	MeqExpr* ej22i = new MeqStoredParmPolc ("EJ22." + ejname2 + nm,
						&itsParmGroup, &itsMEP);
	MeqExpr *ej11, *ej12, *ej21, *ej22;
	if (asAP) {
	  ej11 = new MeqExprAPToComplex (ej11r, ej11i);
	  ej12 = new MeqExprAPToComplex (ej12r, ej12i);
	  ej21 = new MeqExprAPToComplex (ej21r, ej21i);
	  ej22 = new MeqExprAPToComplex (ej22r, ej22i);
	} else {
	  ej11 = new MeqExprToComplex (ej11r, ej11i);
	  ej12 = new MeqExprToComplex (ej12r, ej12i);
	  ej21 = new MeqExprToComplex (ej21r, ej21i);
	  ej22 = new MeqExprToComplex (ej22r, ej22i);
	}
	itsComplexConv.push_back (ej11);
	itsComplexConv.push_back (ej12);
	itsComplexConv.push_back (ej21);
	itsComplexConv.push_back (ej22);
	MeqJonesNode* jonesNode = new MeqJonesNode (ej11, ej12, ej21, ej22);
	itsJonesNodes.push_back (jonesNode);
	vec.push_back (jonesNode);
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
void Prediffer::initParms (const MeqDomain& domain)
{
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();

  itsParmData.clear();
  itsNrScid = 0;
  int i = 0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       ++iter, ++i)
  {
    if (*iter) {
      (*iter)->readPolcs (domain);
      int nr = (*iter)->initDomain (domain, itsNrScid);
      if (nr > 0) {
	itsParmData.push_back (ParmData((*iter)->getName(),
					(*iter)->getTableName(),
					(*iter)->getDBType(),
					(*iter)->getDBName(),
					nr, itsNrScid,
					(*iter)->getCoeffValues()));
	itsNrScid += nr;
      }
    }
  }

  // Unlock the parm tables.
  itsMEP.unlock();
  itsGSMMEP.unlock();
}

//----------------------------------------------------------------------
//
// ~nextInterval
//
// Move to the next interval (domain).
// Set the request belonging to that.
//
//----------------------------------------------------------------------
vector<uint32> Prediffer::setDomain (double fstart, double flength,
				     double tstart, double tlength)
{
  // Determine the first channel and nr of channels to process.
  ASSERT (fstart <= itsEndFreq);
  ASSERT (flength > 0  &&  tlength > 0);
  if (fstart < itsStartFreq) {
    itsFirstChan = 0;
  } else {
    itsFirstChan = int((fstart-itsStartFreq) / itsStepFreq);
  }
  double fend = fstart+flength;
  if (fend >= itsEndFreq) {
    itsLastChan = itsNrChan;
  } else {
    itsLastChan = int(0.5 + (fend-itsStartFreq) / itsStepFreq);
  }
  itsLastChan--;
  ASSERT (itsFirstChan <= itsLastChan);
  // Determine the first data channel to be mapped in.
  if (itsReverseChan) {
    itsDataFirstChan = itsNrChan - 1 - itsLastChan;
  } else {
    itsDataFirstChan = itsFirstChan;
  }

  // Map the part of the file matching the given times.
  NSTimer mapTimer;
  mapTimer.start();
  itsDataMap->unmapFile();
  itsFlagsMap->unmapFile();
  // Find the times matching the given time interval.
  // Normally the times are in sequential order, so we can continue searching.
  // Otherwise start the search at the start.
  if (tstart < itsTimes[itsTimeIndex]) {
    itsTimeIndex = 0;
  }
  // Find the time matching the start time.
  while (itsTimeIndex < itsTimes.nelements()
	 &&  tstart > itsTimes[itsTimeIndex]) {
    ++itsTimeIndex;
  }
  // Exit when no more chunks.
  if (itsTimeIndex >= itsTimes.nelements()) {
    return vector<uint32>();
  }
  
  cout << "BBSTest: BeginOfInterval" << endl;

  // Find the end of the interval.
  int startIndex = itsTimeIndex;
  double startTime = itsTimes[itsTimeIndex] - itsIntervals[itsTimeIndex]/2;
  double endTime = tstart + tlength;
  itsNrTimes     = 0;
  itsNrTimesDone = 0;
  itsBlNext      = 0;
  while (itsTimeIndex < itsTimes.nelements()
	 && endTime >= itsTimes[itsTimeIndex]) {
    ++itsTimeIndex;
    ++itsNrTimes;
  }
  ASSERT (itsNrTimes > 0);
  endTime = itsTimes[itsTimeIndex-1] + itsIntervals[itsTimeIndex-1]/2;
  
  // Map the correct data subset (this time interval)
  int64 startOffset = startIndex*itsNrBl*itsNrChan*itsNCorr*sizeof(fcomplex);
  size_t nrBytes = itsNrTimes*itsNrBl*itsNrChan*itsNCorr*sizeof(fcomplex);
  itsDataMap->mapFile(startOffset, nrBytes); 
  // Map the correct flags subset (this time interval)
  startOffset = startIndex*itsNrBl*itsNrChan*itsNCorr;
  
  nrBytes = itsNrTimes*itsNrBl*itsNrChan*itsNCorr;
  itsFlagsMap->mapFile(startOffset, nrBytes); 
  if (itsLockMappedMem) {
    // Make sure mapped data is resident in RAM
    itsDataMap->lockMappedMemory();
    itsFlagsMap->lockMappedMemory();
  }

  mapTimer.stop();
  cout << "BBSTest: file-mapping " << mapTimer << endl;

  NSTimer parmTimer;
  parmTimer.start();
  itsDomain = MeqDomain(itsStartFreq + itsFirstChan*itsStepFreq,
			itsStartFreq + (itsLastChan+1)*itsStepFreq,
			startTime,
			endTime);

  initParms (itsDomain);
  parmTimer.stop();
  cout << "BBSTest: initparms    " << parmTimer << endl;

  // Create the shape vector.
  vector<uint32> shape(4);
  shape[0] = 2*(itsLastChan-itsFirstChan+1);
  shape[1] = itsNrScid+1;
  // Use a buffer of, say, up to 100 KBytes.
  // All corr, freq and spid have to fit in it.
  itsNrBufTB = std::max (1, int(0.5 + 100000.
				/ (shape[0] * shape[1] * itsNSelCorr)
				/ sizeof(double)));
  shape[2] = itsNrBufTB*itsNSelCorr;
  shape[3] = itsNSelCorr * itsNrSelBl * itsNrTimes;   // total nr
//   itsSolveColName = itsDataColName;
  return shape;
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

  const vector<MeqParm*>& parmList = itsParmGroup.getParms();

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      (*iter)->setSolvable(false);
    }
  }
  itsParmData.resize (0);
}

//----------------------------------------------------------------------
//
// ~setSolvableParms
//
// Set the solvable flag (true or false) on all parameters whose
// name matches the parmPatterns pattern.
//
//----------------------------------------------------------------------
void Prediffer::setSolvableParms (const vector<string>& parms,
				  const vector<string>& excludePatterns,
				  bool isSolvable)
{
  LOG_INFO_STR( "setSolvableParms: "
		<< "isSolvable = " << isSolvable);

  const vector<MeqParm*>& parmList = itsParmGroup.getParms();

  // Convert patterns to regexes.
  vector<Regex> parmRegex;
  for (unsigned int i=0; i<parms.size(); i++) {
    parmRegex.push_back (Regex::fromPattern(parms[i]));
  }

  vector<Regex> excludeRegex;
  for (unsigned int i=0; i<excludePatterns.size(); i++) {
    excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
  }

  // Find all parms matching the parms.
  // Exclude them if matching an excludePattern
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    String parmName ((*iter)->getName());
    // Loop through all regex-es until a match is found.
    for (vector<Regex>::const_iterator incIter = parmRegex.begin();
	 incIter != parmRegex.end();
	 incIter++)
    {
      if (parmName.matches(*incIter)) {
	bool parmExc = false;
	// Test if not excluded.
	for (vector<Regex>::const_iterator excIter = excludeRegex.begin();
	     excIter != excludeRegex.end();
	     excIter++)
	{
	  if (parmName.matches(*excIter)) {
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

//----------------------------------------------------------------------
//
// ~getEquations
//
// Get the condition equations for the selected baselines and domain.
//
//----------------------------------------------------------------------
bool Prediffer::getEquations (double* result, char* flagResult,
			      const vector<uint32>& shape,
			      int& nresult)
{
  ASSERT (itsNrTimesDone < itsNrTimes);
  LOG_TRACE_FLOW("Prediffer::getEquations");
  // Check if the shape is correct.
  int nrchan = itsLastChan-itsFirstChan+1;
  ASSERT (shape[2] == uint(itsNrBufTB*itsNSelCorr)
	  && shape[1] == uint(itsNrScid+1)
	  && shape[0] == uint(2*nrchan));
  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  unsigned int freqOffset = itsDataFirstChan*itsNCorr;

  // Get the pointer to the mapped data.
  fcomplex* dataStart = (fcomplex*)itsDataMap->getStart();
  void* flagStart = itsFlagsMap->getStart();
  int flagStartBit = itsFlagsMap->getStartBit();
  ASSERTSTR(dataStart!=0 && flagStart!=0,
	    "No memory region mapped. Call map(..) first."
	    << " Perhaps you have forgotten to call nextInterval().");

  // Allocate a buffer to convert flags from bits to bools.
  // Use Block instead of vector, because vector uses bits.
  Block<bool> flags(nrchan*itsNCorr);
  bool* flagsPtr = &(flags[0]);
  // Loop through all baselines/times and create a request.
  uint nrDone = 0;
  while (itsNrTimesDone<itsNrTimes) {
    uint tStep = itsNrTimesDone;
    unsigned int timeOffset = tStep*itsNrBl*itsNrChan*itsNCorr;
    double time = itsTimes[itsTimeIndex-itsNrTimes+tStep];
    double interv = itsIntervals[itsTimeIndex-itsNrTimes+tStep];
    
    MeqDomain domain(startFreq, endFreq, time-interv/2, time+interv/2);
    MeqRequest request(domain, nrchan, 1, itsNrScid);

    // Loop through all baselines and get an equation if selected.
    while (itsBlNext<itsNrBl) {
      uint ant1 = itsAnt1[itsBlNext];
      uint ant2 = itsAnt2[itsBlNext];
      ASSERT (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow());
      if (itsBLSelection(ant1,ant2)  &&  itsBLIndex(ant1,ant2) >= 0) {
	if (nrDone == itsNrBufTB) {
	  break;
	}
	// Get pointer to correct data part.
	unsigned int blOffset = itsBlNext*itsNrChan*itsNCorr;
	fcomplex* data = dataStart + timeOffset + blOffset + freqOffset;
	// Convert the flag bits to bools.
	bitToBool (flagsPtr, flagStart, nrchan*itsNCorr,
		   timeOffset + blOffset + freqOffset + flagStartBit);
	// Get an equation for this baseline.
	int blindex = itsBLIndex(ant1,ant2);
	getEquation (result, flagResult, data, flagsPtr,
		     request, blindex, ant1, ant2);
	result += 2*nrchan*itsNSelCorr*(itsNrScid+1);
	flagResult += 2*nrchan*itsNSelCorr;
	++nrDone;
      }
      ++itsBlNext;
    }
    if (itsBlNext == itsNrBl) {
      // A baseline loop has been completed.
      ++itsNrTimesDone;
      itsBlNext = 0;
    }
    if (nrDone == itsNrBufTB) {
      break;
    }
  }
  nresult = nrDone*itsNSelCorr;
  if (itsNrTimesDone == itsNrTimes) {
    cout << "BBSTest: predict " << itsPredTimer << endl;
    cout << "BBSTest: formeqs " << itsEqTimer << endl;
    return false;
  }
  return true;
}

//----------------------------------------------------------------------
//
// ~getEquation
//
// Get the equation for the given baseline.
//
//----------------------------------------------------------------------
void Prediffer::getEquation (double* result, char* flagResult,
			     const fcomplex* data, const bool* flags,
			     const MeqRequest& request,
			     int blindex, int ant1, int ant2)
{
  itsPredTimer.start();
  MeqJonesExpr& expr = *(itsExpr[blindex]);
  expr.calcResult (request);         // This is the actual predict
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  bool showd = false;
  /// showd = (ant1==4 && ant2==8);
  // Put the results in a single array for easier handling.
  const MeqResult* predResults[4];
  predResults[0] = &(expr.getResult11());
  if (itsNCorr == 2) {
    predResults[1] = &(expr.getResult22());
  } else if (itsNCorr == 4) {
    predResults[1] = &(expr.getResult12());
    predResults[2] = &(expr.getResult21());
    predResults[3] = &(expr.getResult22());
  }
  // Loop through the correlations.
  for (int corr=0; corr<itsNCorr; ++corr) {
    if (itsCorr[corr]) {
      // Get the difference (measured - predicted) for all channels.
      const MeqMatrix& val = predResults[corr]->getValue();
      const double* vals = (const double*)(val.dcomplexStorage());
      if (showd) {
	cout << "corr=" << corr << ' ' << val << endl;
      }
      if (itsReverseChan) {
	int dch = nrchan;
	for (int ch=0; ch<nrchan; ++ch) {
	  --dch;
	  *result++ = real(data[corr+dch*itsNCorr]) - vals[2*ch];
	  *result++ = imag(data[corr+dch*itsNCorr]) - vals[2*ch+1];
	  if (showd) cout << *(result-2) << ' ' << *(result-1) << ' ';
	  // Use same flag for real and imaginary part.
	  flagResult[0] = (flags[corr+dch*itsNCorr] ? 1:0);
	  flagResult[1] = flagResult[0];
	  flagResult += 2;
	}
      } else {
	for (int ch=0; ch<nrchan; ++ch) {
	  *result++ = real(data[corr+ch*itsNCorr]) - vals[2*ch];
	  *result++ = imag(data[corr+ch*itsNCorr]) - vals[2*ch+1];
	  if (showd) cout << *(result-2) << ' ' << *(result-1) << ' ';
	  // Use same flag for real and imaginary part.
	  flagResult[0] = (flags[corr+ch*itsNCorr] ? 1:0);
	  flagResult[1] = flagResult[0];
	  flagResult += 2;
	}
      }
      if (showd) cout << endl;
      // Get the derivative for all solvable parameters.
      for (int scinx=0; scinx<itsNrScid; ++scinx) {
	if (! predResults[corr]->isDefined(scinx)) {
	  // Undefined, so set derivatives to 0.
	  for (int ch=0; ch<2*nrchan; ++ch) {
	    *result++ = 0;
	  }
	} else {
	  // Calculate the derivative for each channel (real and imaginary part).
	  double pert = predResults[corr]->getPerturbation(scinx).getDouble();
	  const MeqMatrix& pertVal = predResults[corr]->getPerturbedValue(scinx);
	  const double* pertVals = (const double*)(pertVal.dcomplexStorage());
	  if (showd) {
	    cout << "corr=" << corr << " spid=" << scinx << ' '
		 << pertVal << endl;
	  }
	  for (int ch=0; ch<2*nrchan; ++ch) {
	    *result++ = (pertVals[ch] - vals[ch]) / pert;
	    if (showd) cout << *(result-1) << ' ';
	  }
	  if (showd) cout << endl;
	}
      }
    }
  }
  itsEqTimer.stop();
}

//----------------------------------------------------------------------
//
// ~getResults
//
// Get the results for the selected baselines and domain.
//
//----------------------------------------------------------------------
vector<MeqResult> Prediffer::getResults (bool calcDeriv)
{
  vector<MeqResult> results;
  int nrchan = itsLastChan-itsFirstChan+1;
  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  for (unsigned int tStep=0; tStep<itsNrTimes; tStep++)
  {
    double time = itsTimes[itsTimeIndex-itsNrTimes+tStep];
    double interv = itsIntervals[itsTimeIndex-itsNrTimes+tStep];
    MeqDomain domain(startFreq, endFreq, time-interv/2, time+interv/2);
    MeqRequest request(domain, nrchan, 1, 0);
    if (calcDeriv) {
      request = MeqRequest(domain, nrchan, 1, itsNrScid);
    }
    for (unsigned int bl=0; bl<itsNrBl; bl++)
    {
      uInt ant1 = itsAnt1[bl];
      uInt ant2 = itsAnt2[bl];
      if (itsBLSelection(ant1,ant2) == true)
      {
	///showd = (ant1==4 && ant2==8);
	// Get the result for this baseline.
	int blindex = itsBLIndex(ant1,ant2);
	MeqJonesExpr& expr = *(itsExpr[blindex]);
	expr.calcResult (request);         // This is the actual predict
	results.push_back (expr.getResult11());
	results.push_back (expr.getResult12());
	results.push_back (expr.getResult21());
	results.push_back (expr.getResult22());
      }
    }
  }
  return results;
}

//----------------------------------------------------------------------
//
// ~subtractPeelSources
//
// Save the colA - colB in residualCol.
//
//----------------------------------------------------------------------
void Prediffer::subtractPeelSources (bool write)
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
  int64 startOffset = (itsTimeIndex-itsNrTimes)*itsNrBl*itsNrChan*itsNCorr*sizeof(fcomplex);
  size_t nrBytes = itsNrTimes*itsNrBl*itsNrChan*itsNCorr*sizeof(fcomplex);
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
    unsigned int timeOffset = tStep*itsNrBl*itsNrChan*itsNCorr;
    double time = itsTimes[itsTimeIndex-itsNrTimes+tStep];
    double interv = itsIntervals[itsTimeIndex-itsNrTimes+tStep];

    MeqDomain domain(startFreq, endFreq, time-interv/2, time+interv/2);
    MeqRequest request(domain, nrchan, 1, itsNrScid);
    
    for (unsigned int bl=0; bl<itsNrBl; bl++)
    {
      uInt ant1 = itsAnt1[bl];
      uInt ant2 = itsAnt2[bl];
      if (itsBLSelection(ant1,ant2) == true)
      {
        unsigned int blOffset = bl*itsNrChan*itsNCorr;
        unsigned int freqOffset = itsFirstChan*itsNCorr;
        // Set the data pointer
        Complex* dataStart = (Complex*)itsDataMap->getStart();

        ASSERTSTR(dataStart!=0, "No memory region mapped. Call map(..) first."
                  << " Perhaps you have forgotten to call nextInterval().");
        Complex* dataPtr = dataStart + timeOffset + blOffset + freqOffset;

	ASSERT (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
		&&  itsBLIndex(ant1,ant2) >= 0);
	int blindex = itsBLIndex(ant1,ant2);

	itsExpr[blindex]->calcResult (request);
	
	// Create a matrix from selected frequencies and all correlations.
	Matrix<Complex> data(IPosition(2, itsNCorr, nrchan), dataPtr, SHARE);

	Slice sliceFreq(0, nrchan);
	// Convert the DComplex predicted results to a Complex data array.
	Matrix<Complex> tmpPredict(IPosition(2,1,nrchan));
	convertArray (tmpPredict,
		      itsExpr[blindex]->getResult11().getValue().getDComplexMatrix());

	// Subtract data for all frequencies of correlation 1
	Matrix<Complex> dataCorr0 (data(Slice(0,1), sliceFreq));
	dataCorr0 -= tmpPredict;

	if (4 == itsNCorr) 
	{
	  // Subtract data for all frequencies of correlation 2
	  convertArray (tmpPredict,
			itsExpr[blindex]->getResult12().getValue().getDComplexMatrix());
	  Matrix<Complex> dataCorr1 (data(Slice(1,1), sliceFreq));
	  dataCorr1 -= tmpPredict;

	  // Subtract data for all frequencies of correlation 3
	  convertArray (tmpPredict,
			itsExpr[blindex]->getResult21().getValue().getDComplexMatrix());
	  Matrix<Complex> dataCorr2 (data(Slice(2,1), sliceFreq));
	  dataCorr2 -= tmpPredict;

	  // Subtract data for all frequencies of correlation 4
	  convertArray (tmpPredict,
			itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
	  Matrix<Complex> dataCorr3 (data(Slice(3,1), sliceFreq));
	  dataCorr3 -= tmpPredict;
	} 
	else if (2 == itsNCorr)
	{
	  // Subtract data for all frequencies of correlation 2
	  convertArray (tmpPredict,
			itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
	  Matrix<Complex> dataCorr1 (data(Slice(1,1), sliceFreq));
	  dataCorr1 -= tmpPredict;
	} 
	else if (1 != itsNCorr)
	{
	  throw AipsError("Number of correlations should be 1, 2, or 4");
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
// Select a subset of the MS data.
// This selection has to be done before the loop over domains.
//
//----------------------------------------------------------------------
void Prediffer::select (const vector<int>& ant1, 
			const vector<int>& ant2,
			bool useAutoCorrelations,
			const vector<int>& corr)
{
  ASSERT (ant1.size() == ant2.size());
  if (ant1.size() == 0) {
    // No baselines specified, select all baselines
    itsBLSelection = true;
  } else {
    itsBLSelection = false;
    for (unsigned int j=0; j<ant2.size(); j++) {
      ASSERT(ant2[j] < int(itsBLSelection.nrow()));
      for (unsigned int i=0; i<ant1.size(); i++) {
	ASSERT(ant1[i] < int(itsBLSelection.nrow()));
	itsBLSelection(ant1[i], ant2[j]) = true;     // select this baseline
      }
    }
  }
  // Unset auto-correlations if needed.
  if (!useAutoCorrelations) {
    for (unsigned int i=0; i<itsBLSelection.nrow(); i++) {
      itsBLSelection(i,i) = false;
    }
  } 
  // Fill the correlations to use. Use all if vector is empty.
  // If only 2 corr, YY is the 2nd one.
  for (int i=0; i<4; ++i) {
    itsCorr[i] = corr.empty();
  }
  for (uint i=0; i<corr.size(); ++i) {
    if (corr[i] < 4) {
      if (itsNCorr == 2  &&  corr[i] == 3) {
	itsCorr[1] = true;
      } else {
	itsCorr[corr[i]] = true;
      }
    }
  }
  countBaseCorr();
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
    int a1 = itsAnt1[bl];
    int a2 = itsAnt2[bl];
    if (itsBLSelection(a1,a2) == true)
    {
      if (!statFnd[itsAnt1[bl]]) {
	nStatFnd++;
	statFnd[itsAnt1[bl]] = true;
      }
      if (!statFnd[itsAnt2[bl]]) {
	nStatFnd++;
	statFnd[itsAnt2[bl]] = true;
      }
    }
  }

  // Map uvw data into memory
  size_t nrBytes = itsTimes.nelements() * itsNrBl * 3 * sizeof(double);
  double* uvwDataPtr = 0;
  MMap* mapPtr = new MMap(itsMSName+"/vis.uvw", MMap::Read);
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
    statuvw[3*itsAnt1[0]]   = 0;
    statuvw[3*itsAnt1[0]+1] = 0;
    statuvw[3*itsAnt1[0]+2] = 0;
    statDone[itsAnt1[0]] = true;
    itsStatUVW[itsAnt1[0]]->set (time, 0, 0, 0);

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
	int a1 = itsAnt1[bl];
	int a2 = itsAnt2[bl];
	if (itsBLSelection(a1,a2)  &&  itsBLIndex(a1,a2) >= 0)
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
Bool Prediffer::setPeelSources (const vector<int>& peelSources,
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

void Prediffer::updateSolvableParms (const vector<double>& values)
{
  // Iterate through all parms.
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if ((*iter)->isSolvable()) {
      MeqParmPolc* ppc = dynamic_cast<MeqParmPolc*>(*iter);
      ASSERT (ppc);
      ppc->update (values);
    }
  }
  resetEqLoop();
}

void Prediffer::updateSolvableParms (const vector<ParmData>& parmData)
{
  // Iterate through all parms.
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if ((*iter)->isSolvable()) {
      const string& pname = (*iter)->getName();
      // Update the parameter matching the name.
      for (vector<ParmData>::const_iterator iterpd = parmData.begin();
	   iterpd != parmData.end();
	   iterpd++) {
	if (iterpd->getName() == pname) {
	  MeqParmPolc* ppc = dynamic_cast<MeqParmPolc*>(*iter);
	  ASSERT (ppc);
	  ppc->update (iterpd->getValues());
	  break;
	}
	// A non-matching name is ignored.
      }
    }
  }
  resetEqLoop();
}

void Prediffer::updateSolvableParms()
{
  // Iterate through all parms.
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if ((*iter)->isSolvable()) {
      MeqParmPolc* ppc = dynamic_cast<MeqParmPolc*>(*iter);
      ASSERT (ppc);
      ppc->updateFromTable();
      
//       streamsize prec = cout.precision();
//       cout.precision(10);
//       cout << "****Read: " << (*iter)->getCoeffValues().getDouble()
// 	   << " for parameter " << (*iter)->getName() << endl;
//       cout.precision (prec);

    }
  }
  resetEqLoop();

  itsMEP.unlock();
  itsGSMMEP.unlock();
}

void Prediffer::resetEqLoop()
{
  itsNrTimesDone = 0;
  itsBlNext      = 0;
}

void Prediffer::showSettings() const
{
  cout << "Prediffer settings:" << endl;
  cout << "  msname:    " << itsMSName << endl;
  cout << "  mepname:   " << itsMEPName << endl;
  cout << "  gsmname:   " << itsGSMMEPName << endl;
  cout << "  solvparms: " << itsParmData << endl;
  if (itsReverseChan) {
    cout << "  stchan:    " << itsNrChan - 1 - itsFirstChan << endl;
    cout << "  endchan:   " << itsNrChan - 1 - itsLastChan << endl;
    cout << "    Note: channels are in reversed order" << endl;
  } else {
    cout << "  stchan:    " << itsFirstChan << endl;
    cout << "  endchan:   " << itsLastChan << endl;
  }
  cout << "  calcuvw  : " << itsCalcUVW << endl;
  cout << endl;
}

} // namespace LOFAR
