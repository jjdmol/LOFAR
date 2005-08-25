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
#include <BBS3/MNS/MeqLMN.h>
#include <BBS3/MNS/MeqDFTPS.h>
#include <BBS3/MNS/MeqBaseDFTPS.h>
#include <BBS3/MNS/MeqBaseLinPS.h>
#include <BBS3/MNS/MeqStatExpr.h>
#include <BBS3/MNS/MeqJonesSum.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <BBS3/MNS/MeqMatrixComplexArr.h>
#include <BBS3/MNS/MeqStoredParmPolc.h>
#include <BBS3/MNS/MeqParmSingle.h>
#include <BBS3/MNS/MeqPointSource.h>
#include <BBS3/MNS/MeqJonesCMul3.h>

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
#include <casa/Containers/Record.h>
#include <casa/IO/AipsIO.h>
#include <casa/IO/MemoryIO.h>
#include <casa/Exceptions/Error.h>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <malloc.h>

#include <BBS3/BBSTestLogger.h>

#if defined _OPENMP
#include <omp.h>
#endif

using namespace casa;

#if 0
void *operator new(size_t size)
{
    long *ptr = (long *) malloc(size + 24);

    size += 23, size >>= 3;
    ptr[0] = size;
    ptr[1] = 0x6161F898097771BC;
    ptr[size] = 0x6161F898097771BD;
    return (void *) (ptr + 2);
}

void operator delete(void *addr)
{
    if (addr != 0) {
	long *ptr = (long *) addr - 2;
	if (ptr[1] != 0x6161F898097771BC || ptr[ptr[0]] != 0x6161F898097771BD) {
	    std::cerr << ptr << ": " << ptr[0] << ' ' << ptr[1] << ' ' << ptr[ptr[0]] << '\n';
	    for (;;);
	}
	long value = 0x7777000000000000 + (long) __builtin_return_address(0);
	for (size_t size = ptr[0], i = 0; i <= size; i ++)
	    ptr[i] = value;
	free(ptr);
    }
}
#endif

namespace LOFAR
{

//----------------------------------------------------------------------
//
// Constructor. Initialize a Prediffer object.
//
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the given model type.
//
//----------------------------------------------------------------------
Prediffer::Prediffer(const string& msName,
		     const string& meqModel,
		     const string& skyModel,
		     const string& dbType,
		     const string& dbName,
		     const string& dbUser,
		     const string& dbPwd,
		     const string& dbHost,
		     const int dbMasterPort,
		     const vector<int>& ant,
		     const string& modelType,
		     const vector<vector<int> >& sourceGroups,
		     bool      calcUVW)
  :
  itsMSName       (msName),
  itsMEPName      (meqModel),
  itsMEP          (dbType, meqModel, dbName, dbUser, dbPwd, dbHost, dbMasterPort, false),
  itsGSMMEPName   (skyModel),
  itsGSMMEP       (dbType, skyModel, dbName, dbUser, dbPwd, dbHost, dbMasterPort, false),
  itsCalcUVW      (calcUVW),
  itsSrcGrp       (sourceGroups),
  itsNCorr        (0),
  itsNrBl         (0),
  itsTimeIndex    (0),
  itsNrTimes      (0),
  itsNrTimesDone  (0),
  itsBlNext       (0),
  itsVisMapped    (True),
  itsDataMap      (0),
  itsFlagsMap     (0)
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

  // Get all sources from the GSM and check the source groups.
  getSources();

  // Set up the expression tree for all baselines.
  bool asAP = true;
  bool useStatParm = false;
  bool useEJ = false;
  vector<string> types = StringUtil::split(modelType, '.');
  for (uint i=0; i<types.size(); ++i) {
    if (types[i] == "RI") {
      asAP = false;
      useEJ = true;
    } else if (types[i] == "AP") {
      useEJ = true;
    } else if (types[i] == "USESP") {
      useStatParm = true;
    }
  }
  makeLOFARExpr(useEJ, asAP, useStatParm);

  // Show frequency domain.
  LOG_INFO_STR( "Freq: " << itsStartFreq << ' ' << itsEndFreq << " (" <<
    itsEndFreq - itsStartFreq << " Hz) " << itsNrChan << " channels of "
	    << itsStepFreq << " Hz" );

  if (!itsCalcUVW) {
    // Fill the UVW coordinates from the MS instead of calculating them.
    fillUVW();
 }

  // initialize the ComplexArr pool with the most frequently used size
  // itsNrChan is the number of frequency channels
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
  std::ifstream istr(name.c_str());
  ASSERTSTR (istr, "File " << fileName << "/vis.des could not be opened");
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
      Vector<double> antpos = itsAntPos.column(ant);
      sprintf (str, "%d", ant+1);
      String name = string("SR") + str;
      MeqParmSingle* px = new MeqParmSingle("AntPosX." + name,
					    &itsParmGroup, antpos(0));
      MeqParmSingle* py = new MeqParmSingle("AntPosY." + name,
					    &itsParmGroup, antpos(1));
      MeqParmSingle* pz = new MeqParmSingle("AntPosZ." + name,
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
  // Convert antnrs to bools.
  uint maxAnt = itsStations.size();
  Vector<bool> useAnt(maxAnt);
  useAnt = false;
  for (uint i=0; i<antnrs.size(); i++) {
    uint ant = antnrs[i];
    ASSERT (ant < maxAnt);
    useAnt[ant] = true;
  }
  itsNrSelBl = 0;
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
      itsBLSelection(a1,a2) = true;
      itsBLIndex(a1,a2) = itsNrSelBl++;
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
  int nrSelBl = 0;
  const bool* sel = itsBLSelection.data();
  const int*  inx = itsBLIndex.data();
  for (uint i=0; i<itsBLSelection.nelements(); ++i) {
    if (sel[i]  &&  inx[i] >= 0) {
      nrSelBl++;
    }
  }
  ASSERTSTR (nrSelBl > 0, "No valid baselines selected");
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
// ~getSources
//
// Get all sources from the GSM table.
//
//----------------------------------------------------------------------
void Prediffer::getSources()
{
  // Get the sources from the ParmTable
  itsSources = itsGSMMEP.getPointSources (&itsParmGroup, Vector<int>());
  int nrsrc = itsSources.size();
  for (int i=0; i<nrsrc; ++i) {
    itsSources[i].setSourceNr (i);
  }
  // Make a map for the sources actually used.
  itsSrcNrMap.reserve (nrsrc);
  if (itsSrcGrp.size() == 0) {
    // Set groups if nothing given.
    itsSrcGrp.resize (nrsrc);
    vector<int> vec(1);
    for (int i=0; i<nrsrc; ++i) {
      vec[0] = i+1;                 // source nrs are 1-relative
      itsSrcGrp[i] = vec;
      itsSrcNrMap.push_back (i);
      itsSources[i].setGroupNr (i);   // group nrs are 0-relative
    }
  } else {
    for (uint j=0; j<itsSrcGrp.size(); ++j) {
      vector<int> srcs = itsSrcGrp[j];
      ASSERTSTR (srcs.size() > 0, "Sourcegroup " << j << " is empty");
      for (uint i=0; i<srcs.size(); ++i) {
	ASSERTSTR (srcs[i] > 0  &&  srcs[i] <= nrsrc,
		   "Sourcenr must be > 0 and <= #sources (=" << nrsrc << ')');
	ASSERTSTR (itsSources[srcs[i]-1].getGroupNr() < 0,
		   "Sourcenr " << srcs[i] << " multiply used in groups");
        itsSources[srcs[i]-1].setGroupNr (j);
	itsSources[srcs[i]-1].setSourceNr (itsSrcNrMap.size());
	itsSrcNrMap.push_back (srcs[i]-1);
      }
    }
  }
  // Make an LMN node for each source used.
  int nrused = itsSrcNrMap.size();
  itsLMN.reserve (nrused);
  for (int i=0; i<nrused; ++i) {
    int src = itsSrcNrMap[i];
    MeqLMN* lmn = new MeqLMN(&(itsSources[src]));
    lmn->setPhaseRef (&itsPhaseRef);
    itsLMN.push_back (lmn);
  }
}

//----------------------------------------------------------------------
//
// ~makeLOFARExpr
//
// Make the expression tree per baseline for LOFAR.
//
//----------------------------------------------------------------------
void Prediffer::makeLOFARExpr (bool useEJ, bool asAP, bool useStatParm)
{
  // Allocate the vectors holding the expressions.
  int nrstat = itsStations.size();
  int nrsrc  = itsSrcNrMap.size();
  int nrgrp  = itsSrcGrp.size();
  itsStatUVW.reserve  (nrstat);
  // EJ is real/imag or ampl/phase
  string ejname1 = "real.";
  string ejname2 = "imag.";
  if (asAP) {
    ejname1 = "ampl.";
    ejname2 = "phase.";
  }
  // Vector containing StatExpr-s.
  vector<MeqJonesExpr> statExpr(nrstat);
  // Vector containing DFTPS-s.
  vector<MeqExpr> pdfts(nrsrc*nrstat);
  // Vector containing all EJ-s per station per source group.
  vector<MeqJonesExpr> grpEJ(nrgrp*nrstat);
  // Fill the vectors for each station.
  for (int i=0; i<nrstat; ++i) {
    MeqStatUVW* uvw = 0;
    // Do it only if the station is actually used.
    if (itsStations[i] != 0) {
      // Expression to calculate UVW per station
      uvw = new MeqStatUVW (itsStations[i], &itsPhaseRef);
      // Do pure station parameters only if told so.
      if (useStatParm) {
	MeqExpr frot (new MeqStoredParmPolc ("frot." +
			  		     itsStations[i]->getName(),
					     &itsParmGroup, &itsMEP));
	MeqExpr drot (new MeqStoredParmPolc ("drot." +
					     itsStations[i]->getName(),
					     &itsParmGroup, &itsMEP));
	MeqExpr dell (new MeqStoredParmPolc ("dell." +
					     itsStations[i]->getName(),
					     &itsParmGroup, &itsMEP));
	MeqExpr gain11 (new MeqStoredParmPolc ("gain.11." +
					       itsStations[i]->getName(),
					       &itsParmGroup, &itsMEP));
	MeqExpr gain22 (new MeqStoredParmPolc ("gain.22." +
					       itsStations[i]->getName(),
					       &itsParmGroup, &itsMEP));
	statExpr[i] = MeqJonesExpr(new MeqStatExpr (frot, drot, dell,
						    gain11, gain22));
      }
      // Make a DFT per station per source.
      for (int src=0; src<nrsrc; ++src) {
	pdfts[i*nrsrc + src] = MeqExpr(new MeqDFTPS (itsLMN[src], uvw));
      }
      if (useEJ) {
	// Make a complex gain expression per station per source group.
	MeqExprRep* ej11;
	MeqExprRep* ej12;
	MeqExprRep* ej21;
	MeqExprRep* ej22;
	for (int j=0; j<nrgrp; j++) {
	  ostringstream ostr;
	  ostr << j+1;
	  string nm = itsStations[i]->getName() + ".SG" + ostr.str();
	  MeqExpr ej11r (new MeqStoredParmPolc ("EJ11." + ejname1 + nm,
						&itsParmGroup, &itsMEP));
	  MeqExpr ej11i (new MeqStoredParmPolc ("EJ11." + ejname2 + nm,
						&itsParmGroup, &itsMEP));
	  MeqExpr ej12r (new MeqStoredParmPolc ("EJ12." + ejname1 + nm,
						&itsParmGroup, &itsMEP));
	  MeqExpr ej12i (new MeqStoredParmPolc ("EJ12." + ejname2 + nm,
						&itsParmGroup, &itsMEP));
	  MeqExpr ej21r (new MeqStoredParmPolc ("EJ21." + ejname1 + nm,
						&itsParmGroup, &itsMEP));
	  MeqExpr ej21i (new MeqStoredParmPolc ("EJ21." + ejname2 + nm,
						&itsParmGroup, &itsMEP));
	  MeqExpr ej22r (new MeqStoredParmPolc ("EJ22." + ejname1 + nm,
						&itsParmGroup, &itsMEP));
	  MeqExpr ej22i (new MeqStoredParmPolc ("EJ22." + ejname2 + nm,
						&itsParmGroup, &itsMEP));
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
	  grpEJ[i*nrgrp + j] = new MeqJonesNode (MeqExpr(ej11), MeqExpr(ej12),
						 MeqExpr(ej21), MeqExpr(ej22));
	}
      }
    }
    itsStatUVW.push_back (uvw);
  }    

  // Make an expression for each baseline.
  itsExpr.resize (itsNrSelBl);
  if (useStatParm) {
    itsResExpr.resize (itsNrSelBl);
  }
  int nrant = itsBLIndex.nrow();
  for (int ant2=0; ant2<nrant; ant2++) {
    for (int ant1=0; ant1<nrant; ant1++) {
      int blindex = itsBLIndex(ant1,ant2);
      if (blindex >= 0) {
	vector<MeqJonesExpr> vecAll;
	// Loop through all source groups.
	for (int grp=0; grp<nrgrp; ++grp) {
	  const vector<int>& srcgrp = itsSrcGrp[grp];
	  vector<MeqJonesExpr> vec;
	  vec.reserve (srcgrp.size());
	  for (uint j=0; j<srcgrp.size(); ++j) {
	    // Create the total DFT per source.
	    int src = srcgrp[j] - 1;
	    MeqExpr expr1 (new MeqBaseDFTPS (pdfts[ant1*nrsrc + src],
					     pdfts[ant2*nrsrc + src],
					     itsLMN[src]));
	    vec.push_back (MeqJonesExpr
			  (new MeqBaseLinPS(expr1,
					    &(itsSources[itsSrcNrMap[src]]))));
	  }
	  MeqJonesExpr sum;
	  // Sum all sources in the group.
	  if (vec.size() == 1) {
	    sum = vec[0];
	  } else {
	    sum = MeqJonesExpr (new MeqJonesSum(vec));
	  }
	  // Multiple by ionospheric gain/phase per station.
	  if (useEJ) {
	    vecAll.push_back (new MeqJonesCMul3(grpEJ[ant1*nrgrp + grp],
						sum,
						grpEJ[ant2*nrgrp + grp]));
	  } else {
	    vecAll.push_back (sum);
	  }
	}
	// Sum all groups.
	MeqJonesExpr sumAll;
	if (vecAll.size() == 1) {
	  sumAll = vecAll[0];
	} else {
	  sumAll = MeqJonesExpr (new MeqJonesSum(vecAll));
	}
	if (useStatParm) {
	  itsExpr[blindex] = new MeqJonesCMul3(statExpr[ant1],
					       sumAll,
					       statExpr[ant2]);
	} else {
	  itsExpr[blindex] = sumAll;
	}
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
int Prediffer::setDomain (double fstart, double flength,
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
  if (itsTimeIndex >= itsTimes.nelements()
  ||  tstart < itsTimes[itsTimeIndex]) {
    itsTimeIndex = 0;
  }
  // Find the time matching the start time.
  while (itsTimeIndex < itsTimes.nelements()
	 &&  tstart > itsTimes[itsTimeIndex]) {
    ++itsTimeIndex;
  }
  // Exit when no more chunks.
  if (itsTimeIndex >= itsTimes.nelements()) {
    return 0;
  }
  
  BBSTestLogger BBSTest;
  BBSTest.log("BeginOfInterval");

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
  
  // Map the correct data subset (this time interval).
  int64 nr1 = itsNrChan*itsNCorr;
  int64 nrb = itsNrBl;
  int64 nr2 = nrb*itsNrTimes;
  int64 nrValues = nr1*nr2;
  nr2 = nrb*startIndex;
  int64 startOffset = nr1*nr2;
  itsDataMap->mapFile(nr1*nr2*sizeof(fcomplex), nrValues*sizeof(fcomplex)); 
  // Map the correct flags subset (this time interval)
  itsFlagsMap->mapFile(startOffset, nrValues); 

  mapTimer.stop();
  BBSTest.log("file-mapping", mapTimer);

  NSTimer parmTimer;
  parmTimer.start();
  itsDomain = MeqDomain(itsStartFreq + itsFirstChan*itsStepFreq,
			itsStartFreq + (itsLastChan+1)*itsStepFreq,
			startTime,
			endTime);

  initParms (itsDomain);
  parmTimer.stop();
  BBSTest.log("initparms", parmTimer);
  // Return the (estimated) maximum buffer size needed to marshall the
  // fitter object.
  return (itsNrScid+1)*itsNrScid/2*8 + 1000;
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
    if (*iter) {
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
}

//----------------------------------------------------------------------
//
// ~fillFitter
//
// Fill the fitter with the condition equations for the selected baselines
// and domain.
//
//----------------------------------------------------------------------
void Prediffer::fillFitter (casa::LSQFit& fitter)
{
  fitter.set (itsNrScid);

#if defined _OPENMP
  vector<LSQFit> threadPrivateFitters(omp_get_max_threads() - 1);
  for (int i = 0; i < omp_get_max_threads() - 1; i ++)
    threadPrivateFitters[i].set(itsNrScid);
#endif

  LOG_TRACE_FLOW("Prediffer::fillFitter");
  int nrchan = itsLastChan-itsFirstChan+1;
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

  // Loop through all baselines/times and create a request.
  for (uint tStep=0; tStep<itsNrTimes; ++tStep) {
    unsigned int timeOffset = tStep*itsNrBl*itsNrChan*itsNCorr;
    double time = itsTimes[itsTimeIndex-itsNrTimes+tStep];
    double interv = itsIntervals[itsTimeIndex-itsNrTimes+tStep];
    
    MeqDomain domain(startFreq, endFreq, time-interv/2, time+interv/2);
    MeqRequest request(domain, nrchan, 1, itsNrScid);
    // Loop through all baselines and fill its equations if selected.
    //static NSTimer timer("Prediffer::fillFitter", true);
    //timer.start();
#pragma omp parallel
    {
      // Allocate a buffer to convert flags from bits to bools.
      // Use Block instead of vector, because vector uses bits.
      Block<bool> flags(nrchan*itsNCorr);
      bool* flagsPtr = &(flags[0]);

      vector<double> result(nrchan*itsNSelCorr*(itsNrScid+1)*2);
      vector<char> flagResult(nrchan*itsNSelCorr*2);

      LSQFit *myFitter = &fitter;
#if defined _OPENMP
      int threadNr = omp_get_thread_num();

      if (threadNr > 0)
	myFitter = &threadPrivateFitters[threadNr - 1];
#endif

#pragma omp for
      for (int bl=0; bl< (int) itsNrBl; ++bl) {
	uint ant1 = itsAnt1[bl];
	uint ant2 = itsAnt2[bl];
	ASSERT (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow());
	if (itsBLSelection(ant1,ant2)  &&  itsBLIndex(ant1,ant2) >= 0) {
	  // Get pointer to correct data part.
	  unsigned int blOffset = bl*itsNrChan*itsNCorr;
	  fcomplex* data = dataStart + timeOffset + blOffset + freqOffset;
	  // Convert the flag bits to bools.
	  bitToBool (flagsPtr, flagStart, nrchan*itsNCorr,
		     timeOffset + blOffset + freqOffset + flagStartBit);
	  // Get an equation for this baseline.
	  int blindex = itsBLIndex(ant1,ant2);
	  fillEquation (*myFitter, itsNSelCorr, nrchan*2,
			&(result[0]), &(flagResult[0]), data, flagsPtr,
			request, blindex, ant1, ant2);
	}
      }
    } // end omp parallel
    //timer.stop();
  }

#if defined _OPENMP
  for (int i = omp_get_max_threads() - 1; -- i >= 0;)
    fitter.merge(threadPrivateFitters[i]);
#endif

  BBSTestLogger::log("predict", itsPredTimer);
  BBSTestLogger::log("formeqs", itsEqTimer);
}


//----------------------------------------------------------------------
//
// ~fillEquation
//
// Fill the fitter with the equations for the given baseline.
//
//----------------------------------------------------------------------
void Prediffer::fillEquation (casa::LSQFit& fitter, int nresult, int nrval,
			      double* result, char* flagResult,
			      const fcomplex* data, const bool* flags,
			      const MeqRequest& request,
			      int blindex, int ant1, int ant2)
{
  // Get all equations.
  getEquation (result, flagResult, data, flags, request, blindex, ant1, ant2);
  // Add all equations to the fitter.
  itsEqTimer.start();
  // Use a consecutive vector to assemble all derivatives.
  int nrspid = itsNrScid;
  vector<double> derivVec(nrspid);
  double* derivs = &(derivVec[0]);
  // Each result is a 2d array of [nrval,nrspid+1] (nrval varies most rapidly).
  // The first value is the difference; the others the derivatives. 
  for (int i=0; i<nresult; ++i) {
    for (int j=0; j<nrval; ++j) {
      // Each value result,freq gives an equation (unless flagged).
      if (*flagResult++ == 0) {
	double diff = result[0];
	const double* derivdata = result + nrval;
	for (int k=0; k<nrspid; ++k) {
	  derivs[k] = derivdata[k*nrval];
	}
	fitter.makeNorm (&(derivVec[0]), 1., diff);
	//itsNUsed++;
      } else {
	//itsNFlag++;
      }
      // Go to next time,freq value.
      result++;
    }
    // Go to next result (note that data has already been incremented nrval
    // times, so here we use nrspid instead of nrspid+1.
    result += nrspid*nrval;
  }
  itsEqTimer.stop();
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
  MeqJonesExpr& expr = itsExpr[blindex];
  // Do the actual predict.
  MeqJonesResult jresult = expr.getResult (request); 
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  bool showd = false;
  /// showd = (ant1==4 && ant2==8);
  // Put the results in a single array for easier handling.
  const MeqResult* predResults[4];
  predResults[0] = &(jresult.getResult11());
  if (itsNCorr == 2) {
    predResults[1] = &(jresult.getResult22());
  } else if (itsNCorr == 4) {
    predResults[1] = &(jresult.getResult12());
    predResults[2] = &(jresult.getResult21());
    predResults[3] = &(jresult.getResult22());
  }

  // Loop through the correlations.
  for (int corr=0; corr<itsNCorr; ++corr) {
    if (itsCorr[corr]) {
      // Get the difference (measured - predicted) for all channels.
      const MeqMatrix& val = predResults[corr]->getValue();
      const double *realVals, *imagVals;
      val.dcomplexStorage(realVals, imagVals);
      if (itsReverseChan) {
	int dch = nrchan;
	for (int ch=0; ch<nrchan; ++ch) {
	  --dch;
	  *result++ = real(data[corr+dch*itsNCorr]) - realVals[ch];
	  *result++ = imag(data[corr+dch*itsNCorr]) - imagVals[ch];
	  // Use same flag for real and imaginary part.
	  flagResult[0] = (flags[corr+dch*itsNCorr] ? 1:0);
	  flagResult[1] = flagResult[0];
	  flagResult += 2;
	}
      } else {
	for (int ch=0; ch<nrchan; ++ch) {
	  *result++ = real(data[corr+ch*itsNCorr]) - realVals[ch];
	  *result++ = imag(data[corr+ch*itsNCorr]) - imagVals[ch];
	  // Use same flag for real and imaginary part.
	  flagResult[0] = (flags[corr+ch*itsNCorr] ? 1:0);
	  flagResult[1] = flagResult[0];
	  flagResult += 2;
	}
      }
      if (showd) {
	cout << "corr=" << corr << ' ' << val << endl;
	for (int ch=0; ch<nrchan; ++ch) {
	  cout << real(data[corr+ch*itsNCorr]) - realVals[ch] << ' '
	       << imag(data[corr+ch*itsNCorr]) - imagVals[ch] << ' ';
	}
	cout << endl;
      }
      // Get the derivative for all solvable parameters.
      for (int scinx=0; scinx<itsNrScid; ++scinx) {
	if (! predResults[corr]->isDefined(scinx)) {
	  // Undefined, so set derivatives to 0.
	  for (int ch=0; ch<2*nrchan; ++ch) {
	    *result++ = 0;
	  }
	} else {
	  // Calculate the derivative for each channel (real and imaginary part).
	  double invPert = 1.0 / predResults[corr]->getPerturbation(scinx);
	  const MeqMatrix& pertVal = predResults[corr]->getPerturbedValue(scinx);
	  const double *pertRealVals, *pertImagVals;
	  pertVal.dcomplexStorage(pertRealVals, pertImagVals);
	  for (int ch=0; ch<nrchan; ++ch) {
	    *result++ = (pertRealVals[ch] - realVals[ch]) * invPert;
	    *result++ = (pertImagVals[ch] - imagVals[ch]) * invPert;
	    if (showd) cout << result[-2] << ' ' << result[-1] << ' ';
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
	// Get the result for this baseline.
	int blindex = itsBLIndex(ant1,ant2);
	MeqJonesExpr& expr = itsExpr[blindex];
	// This is the actual predict.
	MeqJonesResult result = expr.getResult (request);
	results.push_back (result.getResult11());
	results.push_back (result.getResult12());
	results.push_back (result.getResult21());
	results.push_back (result.getResult22());
      }
    }
  }
  return results;
}

//----------------------------------------------------------------------
//
// ~saveData
//
// Save the data for the given baseline.
// Optionally it is subtracted from the current data.
//
//----------------------------------------------------------------------
void Prediffer::saveData (bool subtract, fcomplex* data,
			  const MeqRequest& request,
			  int blindex, int ant1, int ant2)
{
  itsPredTimer.start();
  MeqJonesExpr& expr = itsExpr[blindex];
  // Do the actual predict.
  MeqJonesResult jresult = expr.getResult (request); 
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  bool showd = false;
  /// showd = (ant1==4 && ant2==8);
  // Put the results in a single array for easier handling.
  const MeqResult* predResults[4];
  predResults[0] = &(jresult.getResult11());
  if (itsNCorr == 2) {
    predResults[1] = &(jresult.getResult22());
  } else if (itsNCorr == 4) {
    predResults[1] = &(jresult.getResult12());
    predResults[2] = &(jresult.getResult21());
    predResults[3] = &(jresult.getResult22());
  }
  // Loop through the correlations.
  for (int corr=0; corr<itsNCorr; ++corr) {
    if (itsCorr[corr]) {
      const MeqMatrix& val = predResults[corr]->getValue();
      const double *realVals, *imagVals;
      val.dcomplexStorage(realVals, imagVals);
      // Subtract predicted from the data or store the predict in data.
      if (itsReverseChan) {
	int dch = nrchan;
	if (subtract) {
	  for (int ch=0; ch<nrchan; ++ch) {
	    --dch;
	    data[corr+dch*itsNCorr] -= makefcomplex(realVals[ch], imagVals[ch]);
	  }
	} else {
	  for (int ch=0; ch<nrchan; ++ch) {
	    --dch;
	    data[corr+dch*itsNCorr] = makefcomplex(realVals[ch], imagVals[ch]);
	  }
	}
      } else {
	if (subtract) {
	  for (int ch=0; ch<nrchan; ++ch) {
	    data[corr+ch*itsNCorr] -= makefcomplex(realVals[ch], imagVals[ch]);
	  }
	} else {
	  for (int ch=0; ch<nrchan; ++ch) {
	    data[corr+ch*itsNCorr] = makefcomplex(realVals[ch], imagVals[ch]);
	  }
	}
      }
      if (showd) {
	cout << "corr=" << corr << ' ' << val << endl;
	for (int ch=0; ch<nrchan; ++ch) {
	  cout << data[corr+ch*itsNCorr] << ' ';
	}
	cout << endl;
      }
    }
  }
}

//----------------------------------------------------------------------
//
// ~getData
//
// Get the mapped data for the selected baselines and domain.
//
//----------------------------------------------------------------------
void Prediffer::getData (Array<Complex>& dataArr, Array<Bool>& flagArr)
{
  int nrchan = itsLastChan-itsFirstChan+1;
  unsigned int freqOffset = itsDataFirstChan*itsNCorr;
  dataArr.resize (IPosition(4, itsNCorr, nrchan, itsNrBl, itsNrTimes));
  flagArr.resize (IPosition(4, itsNCorr, nrchan, itsNrBl, itsNrTimes));
  Complex* datap = dataArr.data();
  Bool* flagp = flagArr.data();

  // Get the pointer to the mapped data.
  fcomplex* dataStart = (fcomplex*)itsDataMap->getStart();
  void* flagStart = itsFlagsMap->getStart();
  int flagStartBit = itsFlagsMap->getStartBit();
  ASSERTSTR(dataStart!=0 && flagStart!=0,
	    "No memory region mapped. Call map(..) first."
	    << " Perhaps you have forgotten to call nextInterval().");
  for (unsigned int tStep=0; tStep<itsNrTimes; tStep++)
  {
    unsigned int timeOffset = tStep*itsNrBl*itsNrChan*itsNCorr;
    for (unsigned int bl=0; bl<itsNrBl; bl++)
    {
      // Get pointer to correct data part.
      unsigned int blOffset = bl*itsNrChan*itsNCorr;
      // Convert the flag bits to bools.
      bitToBool (flagp, flagStart, nrchan*itsNCorr,
		 timeOffset + blOffset + freqOffset + flagStartBit);
      memcpy (datap, dataStart + timeOffset + blOffset + freqOffset,
	      nrchan*itsNCorr*sizeof(Complex));
      datap += nrchan*itsNCorr;
      flagp += nrchan*itsNCorr;
    }
  }
}

//----------------------------------------------------------------------
//
// ~ssaveResidualData
//
// Save data-predict or predict in data.
//
//----------------------------------------------------------------------
void Prediffer::saveResidualData (bool subtract, bool write)
{
  cout << "saveResidualData to '" << itsDataMap->getFileName() << "'" << endl;

  // Map .res file if not mapped yet.
  if (itsVisMapped) {
    size_t size = 0;
    int64  offs = 0;
    if (itsDataMap) {
      size = itsDataMap->getSize();
      offs = itsDataMap->getOffset();
      delete itsDataMap;
      itsDataMap = 0;
    }
    itsDataMap  = new MMap(itsMSName + "/vis.res", MMap::ReWr);
    if (size > 0) {
      itsDataMap->mapFile (offs, size);
    }
  }

  cout << "Using peel sources " << itsPeelSourceNrs << endl;;
  itsSources.setSelected (itsPeelSourceNrs);

  int nrchan = itsLastChan-itsFirstChan+1;
  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  unsigned int freqOffset = itsDataFirstChan*itsNCorr;

  // Get the pointer to the mapped data.
  fcomplex* dataStart = (fcomplex*)itsDataMap->getStart();
  ASSERTSTR(dataStart!=0,
	    "No memory region mapped. Call map(..) first."
	    << " Perhaps you have forgotten to call nextInterval().");

  // Loop through all baselines/times and create a request.
  for (uint tStep=0; tStep<itsNrTimes; ++tStep) {
    unsigned int timeOffset = tStep*itsNrBl*itsNrChan*itsNCorr;
    double time = itsTimes[itsTimeIndex-itsNrTimes+tStep];
    double interv = itsIntervals[itsTimeIndex-itsNrTimes+tStep];
    
    MeqDomain domain(startFreq, endFreq, time-interv/2, time+interv/2);
    MeqRequest request(domain, nrchan, 1, itsNrScid);
    // Loop through all baselines and subtract the predict from the data.
    for (uint bl=0; bl<itsNrBl; ++bl) {
      uint ant1 = itsAnt1[bl];
      uint ant2 = itsAnt2[bl];
      ASSERT (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow());
      if (itsBLSelection(ant1,ant2)  &&  itsBLIndex(ant1,ant2) >= 0) {
	// Get pointer to correct data part.
	unsigned int blOffset = bl*itsNrChan*itsNCorr;
	fcomplex* data = dataStart + timeOffset + blOffset + freqOffset;
	// Subtract for this baseline.
	int blindex = itsBLIndex(ant1,ant2);
	saveData (subtract, data, request, blindex, ant1, ant2);
      }
    }
  }
  BBSTestLogger::log("predict", itsPredTimer);
  BBSTestLogger::log("saveData", itsEqTimer);
  // Make sure data is written.
  if (write) {
    itsDataMap->flush();
  }
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
bool Prediffer::setPeelGroups (const vector<int>& peelGroups,
			       const vector<int>& extraGroups)
{
  vector<int> allNrs;
  for (uint i=0; i<extraGroups.size(); ++i) {
    ASSERT (extraGroups[i] >= 0  &&  extraGroups[i] < int(itsSrcGrp.size()));
    const vector<int>& grp = itsSrcGrp[i];
    for (uint j=0; j<grp.size(); ++j) {
      allNrs.push_back (grp[j] - 1);
    }
  }
  vector<int> peelNrs;
  for (uint i=0; i<peelGroups.size(); ++i) {
    ASSERT (peelGroups[i] >= 0  &&  peelGroups[i] < int(itsSrcGrp.size()));
    const vector<int>& grp = itsSrcGrp[i];
    for (uint j=0; j<grp.size(); ++j) {
      peelNrs.push_back (grp[j] - 1);
      allNrs.push_back (grp[j] - 1);
    }
  }
  LOG_TRACE_OBJ_STR( "peel sources " << peelNrs << "; predict sources "
		     << allNrs );
  ASSERT (peelNrs.size() > 0);
  itsSources.setSelected (allNrs);
  itsPeelSourceNrs = peelNrs;
  return true;
}

void Prediffer::updateSolvableParms (const vector<double>& values)
{
  // Iterate through all parms.
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      if ((*iter)->isSolvable()) {
	MeqParmPolc* ppc = dynamic_cast<MeqParmPolc*>(*iter);
	ASSERT (ppc);
	ppc->update (values);
      }
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
    if (*iter) {
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
    if (*iter) {
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

void Prediffer::writeParms()
{
  NSTimer saveTimer;
  saveTimer.start();
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      if ((*iter)->isSolvable()) {
	(*iter)->save();
      }
    }
  }
  // Unlock the parm tables.
  itsMEP.unlock();
  itsGSMMEP.unlock();
  saveTimer.stop();
  BBSTestLogger::log("write-parm", saveTimer);
  cout << "wrote timeIndex=" << itsTimeIndex
       << " nrTimes=" << itsNrTimes << endl;
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

void Prediffer::marshall (const LSQFit& fitter, void* buffer, int bufferSize)
{
  // Store the fitter in a few steps:
  // - convert to a Record
  // Make a non-expandable buffer and use it in AipsIO.
  casa::MemoryIO buf(buffer, bufferSize, ByteIO::Update);
  casa::AipsIO aio(&buf);
  casa::Record rec;
  casa::String str;
  // Convert the fitter to a Record and serialize it into the buffer.
  ASSERT (fitter.toRecord (str, rec));
  aio << rec;
}

void Prediffer::demarshall (LSQFit& fitter, const void* buffer, int bufferSize)
{
  casa::MemoryIO buf(buffer, bufferSize);
  casa::AipsIO aio(&buf);
  casa::Record rec;
  aio >> rec;
  casa::String str;
  ASSERT (fitter.fromRecord (str, rec));
}

} // namespace LOFAR
