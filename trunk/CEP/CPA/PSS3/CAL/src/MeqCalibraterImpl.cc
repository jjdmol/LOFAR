//# MeqCalibraterImpl.cc: Implementation of the MeqCalibrater DO
//#
//# Copyright (C) 2002
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

#include <CAL/MeqCalibraterImpl.h>

#include <MNS/MeqJonesNode.h>
#include <MNS/MeqStatExpr.h>
#include <MNS/MeqMatrixTmp.h>
#include <MNS/MeqStoredParmPolc.h>
#include <MNS/MeqParmSingle.h>
#include <MNS/MeqPointDFT.h>
#include <MNS/MeqPointSource.h>
#include <MNS/MeqWsrtInt.h>
#include <MNS/MeqWsrtPoint.h>
#include <MNS/MeqLofarPoint.h>
#include <MNS/MeqMatrixComplexArr.h>

#include <Common/Debug.h>

#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <scimath/Functionals/Polynomial.h>
#include <tasking/Glish/GlishArray.h>
#include <tasking/Glish/GlishRecord.h>
#include <tasking/Glish/GlishValue.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <casa/BasicSL/Constants.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/OS/Timer.h>
#include <casa/Quanta/MVBaseline.h>
#include <casa/Quanta/MVPosition.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ExprNodeSet.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableParse.h>
#include <casa/Utilities/Regex.h>
#include <tasking/Tasking/MethodResult.h>
#include <tasking/Tasking/Parameter.h>

#include <stdexcept>
#include <stdio.h>


//----------------------------------------------------------------------
//
// ~getPhaseRef
//
// Get the phase reference of the first field.
//
//----------------------------------------------------------------------
void MeqCalibrater::getPhaseRef()
{
  MSField mssub(itsMS.field());
  ROMSFieldColumns mssubc(mssub);
  // Use the phase reference of the first field.
  MDirection phaseRef = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));
  // Use the time in the first MS row.
  double startTime = itsMSCol.time()(0);
  itsPhaseRef = MeqPhaseRef (phaseRef, startTime);
}

//----------------------------------------------------------------------
//
// ~getFreq
//
// Get the frequency window for the given data description id
// (giving the spectral window id).
//
//----------------------------------------------------------------------
void MeqCalibrater::getFreq (int ddid)
{
  // Get the frequency domain of the given data descriptor id
  // which gives the spwid.
  MSDataDescription mssub1(itsMS.dataDescription());
  ROMSDataDescColumns mssub1c(mssub1);
  int spw = mssub1c.spectralWindowId()(ddid);
  MSSpectralWindow mssub(itsMS.spectralWindow());
  ROMSSpWindowColumns mssubc(mssub);
  Vector<double> chanFreq = mssubc.chanFreq()(spw);
  Vector<double> chanWidth = mssubc.chanWidth()(spw);
  // So far, only equal frequency spacings are possible.
  if (! allEQ (chanWidth, chanWidth(0))) {
    throw AipsError("Channels must have equal spacings");
  }
  itsNrChan    = chanWidth.nelements();
  itsStepFreq  = chanWidth(0);
  itsStartFreq = chanFreq(0) - itsStepFreq/2;
  itsEndFreq   = itsStartFreq + itsNrChan*itsStepFreq;
}

//----------------------------------------------------------------------
//
// ~fillStations
//
// Fill the station positions and names.
//
//----------------------------------------------------------------------
void MeqCalibrater::fillStations (const Vector<Int>& ant1,
				  const Vector<Int>& ant2)
{
  MSAntenna          mssub(itsMS.antenna());
  ROMSAntennaColumns mssubc(mssub);
  int nrant = mssub.nrow();
  itsStations = vector<MeqStation*>(nrant, (MeqStation*)0);
  // Get all stations actually used.
  char str[8];
  for (uInt i=0; i<ant1.nelements(); i++) {
    for (int j=0; j<2; j++) {
      int ant = ant1(i);
      if (j==1) ant = ant2(i);
      Assert (ant < nrant);
      if (itsStations[ant] == 0) {
	// Store each position as a constant parameter.
	// Use the antenna name as the parameter name.
	Vector<Double> antpos = mssubc.position()(ant);
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
void MeqCalibrater::fillBaselines (const Vector<int>& ant1,
				   const Vector<int>& ant2)
{
  MeqDomain  domain;
  MeqRequest req(domain, 1, 1);

  itsBaselines.reserve (ant1.nelements());
  itsBLIndex.resize (itsStations.size(), itsStations.size());
  itsBLIndex = -1;

  for (unsigned int i=0; i<ant1.nelements(); i++)
  {
    uInt a1 = ant1(i);
    uInt a2 = ant2(i);
    // Create an MVBaseline object for each antenna pair.
    Assert (a1 < itsStations.size()  &&  a2 < itsStations.size());
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
}

//----------------------------------------------------------------------
//
// ~makeWSRTExpr
//
// Make the expression tree per baseline for the WSRT.
//
//----------------------------------------------------------------------
void MeqCalibrater::makeWSRTExpr()
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
void MeqCalibrater::makeLOFARExpr(Bool asAP)
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
// ~MeqCalibrater
//
// Constructor. Initialize a MeqCalibrater object.
//
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the WSRT.
//
//----------------------------------------------------------------------
MeqCalibrater::MeqCalibrater(const String& msName,
			     const String& meqModel,
			     const String& skyModel,
			     const String& dbType,
			     const String& dbName,
			     const String& dbPwd,
			           uInt    ddid,
			     const Vector<Int>& ant1,
			     const Vector<Int>& ant2,
			     const String& modelType,
			           bool    calcUVW,
			     const String& dataColName,
			     const String& residualColName)
  :
  itsMS       (msName, Table::Update),
  itsMSCol    (itsMS),
  itsMEP      (dbType, meqModel, dbName, dbPwd),
  itsGSMMEP   (dbType, skyModel, dbName, dbPwd),
  itsCalcUVW  (calcUVW),
  itsDataColName (dataColName),
  itsResColName  (residualColName),
  itsSolver   (1)
{
  cdebug(1) << "MeqCalibrater constructor (";
  cdebug(1) << "'" << msName   << "', ";
  cdebug(1) << "'" << meqModel << "', ";
  cdebug(1) << "'" << skyModel << "', ";
  cdebug(1) << ddid << ", " << endl;
  cdebug(1) << itsCalcUVW << ")" << endl;

  // Get phase reference (for field 0).
  getPhaseRef();

  // We only handle field 0, the given data desc id, and antennas.
  // Sort the MS in order of baseline.
  TableExprNode expr = (itsMS.col("FIELD_ID")==0 &&
			itsMS.col("DATA_DESC_ID")==int(ddid));
  if (ant1.nelements() > 0) {
    expr = expr && itsMS.col("ANTENNA1").in (TableExprNodeSet(ant1));
  }
  if (ant2.nelements() > 0) {
    expr = expr && itsMS.col("ANTENNA2").in (TableExprNodeSet(ant2));
  }
  itsSelMS = itsMS(expr);
  Block<String> keys(2);
  keys[0] = "ANTENNA1";
  keys[1] = "ANTENNA2";
  // Sort uniquely to get all baselines.
  // It looks as if QuicSort|NoDuplicates is incorrect
  // (it looks as if it skips some entries).
  Table sortMS = itsSelMS.sort (keys, Sort::Ascending);
  Table blMS = sortMS.sort (keys, Sort::Ascending,
			    Sort::InsSort | Sort::NoDuplicates);

  // Generate the baseline objects for the baselines.
  // If we ever want to solve for station positions, we cannot use the
  // fixed MVBaseline objects, but instead they should be recalculated
  // after each solve iteration.
  // It also forms an index giving for each (ordered) antenna pair the
  // index in the vector of baselines. -1 means that no baseline exists
  // for that antenna pair.
  ROScalarColumn<int> ant1col(blMS, "ANTENNA1");
  ROScalarColumn<int> ant2col(blMS, "ANTENNA2");
  Vector<int> ant1data = ant1col.getColumn();
  Vector<int> ant2data = ant2col.getColumn();
  // First find all used stations (from the ANTENNA subtable of the MS).
  fillStations (ant1data, ant2data);
  Matrix<int> index;
  fillBaselines (ant1data, ant2data);

  // Set up the expression tree for all baselines.
  if (modelType == "WSRT") {
    makeWSRTExpr();
  } else if (modelType == "LOFAR.RI") {
    makeLOFARExpr(False);
  } else {
    makeLOFARExpr(True);
  }

  cdebug(1) << "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
	    << ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor << endl;
  cdebug(1) << "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
	    << ' ' << MeqResultRep::nctor + MeqResultRep::ndtor << endl;

  // Calculate frequency domain.
  getFreq (ddid);
  cdebug(1) << "Freq: " << itsStartFreq << ' ' << itsEndFreq << " (" <<
    itsEndFreq - itsStartFreq << " Hz) " << itsNrChan << " channels of "
	    << itsStepFreq << " Hz" << endl;

  // By default select all rows and all channels.
  // This also sets up the iterator.
  select ("", 0, -1);

  if (!itsCalcUVW) {
    // Fill the UVW coordinates from the MS instead of calculating them.
    fillUVW();
  }

  // initialize the ComplexArr pool with the most frequently used size
  // itsNrChan is the numnber frequency channels
  // 1 is the number of time steps. this code is limited to one timestep only
  MeqMatrixComplexArr::poolActivate(itsNrChan * 1);
  // Unlock the parm tables.
  itsMEP.unlock();
  itsGSMMEP.unlock();
}

//----------------------------------------------------------------------
//
// ~~MeqCalibrater
//
// Destructor for a MeqCalibrater object.
//
//----------------------------------------------------------------------
MeqCalibrater::~MeqCalibrater()
{
  cdebug(1) << "MeqCalibrater destructor" << endl;
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

  // clear up the matrix pool
  MeqMatrixComplexArr::poolDeactivate();
}

//----------------------------------------------------------------------
//
// ~initParms
//
// Initialize all parameters in the expression tree by calling
// its initDomain method.
//
//----------------------------------------------------------------------
void MeqCalibrater::initParms (const MeqDomain& domain)
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
      (*iter)->readPolcs (domain);
      int nr = (*iter)->initDomain (domain, itsNrScid);
      if (nr > 0) {
	itsIsParmSolvable[i] = true;
	itsNrScid += nr;
      }
    }
    i++;
  }

  if (itsNrScid > 0) {
    // Get the initial values of all solvable parms.
    // Resize the solution vector if needed.
    if (itsSolution.isNull()  ||  itsSolution.nx() != itsNrScid) {
      itsSolution = MeqMatrix (double(), itsNrScid, 1);
    }
    int i = 0;
    for (vector<MeqParm*>::const_iterator iter = parmList.begin();
	 iter != parmList.end();
	 iter++)
    {
      if (itsIsParmSolvable[i]) {
	(*iter)->getInitial (itsSolution);
      }
      i++;
    }
    // Initialize the solver.
    itsSolver.set (itsNrScid);
  }
  // Unlock the parm tables.
  itsMEP.unlock();
  itsGSMMEP.unlock();
  // Unlock the MS.
  itsMS.unlock();
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
void MeqCalibrater::setTimeInterval (double secInterval)
{
  cdebug(1) << "setTimeInterval = " << secInterval << endl;
  itsTimeInterval = secInterval;
}

//----------------------------------------------------------------------
//
// ~resetIterator
//
// Start iteration over time domains from the beginning.
//
//----------------------------------------------------------------------
void MeqCalibrater::resetIterator()
{
  cdebug(1) << "resetTimeIterator" << endl;
  itsIter.reset();
}

//----------------------------------------------------------------------
//
// ~nextInterval
//
// Move to the next interval (domain).
// Set the request belonging to that.
//
//----------------------------------------------------------------------
bool MeqCalibrater::nextInterval()
{
  itsCurRows.resize(0);
  // Exit when no more chunks.
  if (itsIter.pastEnd()) {
    return false;
  }
  double timeSize = 0;
  double timeStart = 0;
  double timeStep = 0;
  int nrtim = 0;
  // Get the next chunk until the time interval size is exceeded.
  while (timeSize < itsTimeInterval  &&  !itsIter.pastEnd()) {
    ROScalarColumn<double> timcol(itsIter.table(), "TIME");
    ROScalarColumn<double> intcol(itsIter.table(), "INTERVAL");
    // If first time, calculate interval and start time.
    if (timeStart == 0) {
      timeStep  = intcol(0);
      timeStart = timcol(0) - timeStep/2;
    }
    // Check all intervals are equal.
    ////    Assert (allNear (intcol.getColumn(), timeStep, 1.0e-5));
    timeSize += timeStep;
    int nrr = itsCurRows.nelements();
    int nrn = itsIter.table().nrow();
    itsCurRows.resize (nrr+nrn, True);
    itsCurRows(Slice(nrr,nrn)) = itsIter.table().rowNumbers(itsMS);
    nrtim++;
    itsIter++;
  }
  itsSolveRows.reference (itsCurRows);
  itsSolveDomain = MeqDomain(timeStart, timeStart + nrtim*timeStep,
			     itsStartFreq + itsFirstChan*itsStepFreq,
			     itsStartFreq + (itsLastChan+1)*itsStepFreq);
  initParms (itsSolveDomain);
  itsSolveColName = itsDataColName;
  return true;
}

//----------------------------------------------------------------------
//
// ~clearSolvableParms
//
// Clear the solvable flag on all parms (make them non-solvable).
//
//----------------------------------------------------------------------
void MeqCalibrater::clearSolvableParms()
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cdebug(1) << "clearSolvableParms" << endl;

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
void MeqCalibrater::setSolvableParms (Vector<String>& parmPatterns,
				      Vector<String>& excludePatterns,
				      Bool isSolvable)
{
  vector<MeqParm*> parmVector;

  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cdebug(1) << "setSolvableParms: "
	    << "isSolvable = " << isSolvable << endl;

  // Convert patterns to regexes.
  vector<Regex> parmRegex;
  for (unsigned int i=0; i<parmPatterns.nelements(); i++) {
    parmRegex.push_back (Regex::fromPattern(parmPatterns[i]));
  }

  vector<Regex> excludeRegex;
  for (unsigned int i=0; i<excludePatterns.nelements(); i++) {
    excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
  }

  //
  // Find all parms matching the parmPatterns
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
	    cdebug(1) << "setSolvable: " << (*iter)->getName() << endl;
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
// ~solve
//
// Solve for the solvable parameters on the current time domain.
//
//----------------------------------------------------------------------
GlishRecord MeqCalibrater::solve(bool useSVD)
{
  cdebug(1) << "solve using column " << itsSolveColName << endl;
  cout << "solve using column " << itsSolveColName << endl;

  if (itsSolveRows.nelements() == 0) {
    throw AipsError("nextInterval needs to be done before solve");
  }
  if (itsNrScid == 0) {
    throw AipsError ("No parameters are set to solvable");
  }
  int nrpoint = 0;
  Timer timer;

  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  int nrchan = 1+itsLastChan-itsFirstChan;
  Slicer dataSlicer (IPosition(2,0,itsFirstChan),
		     IPosition(2,Slicer::MimicSource,nrchan));

  ROArrayColumn<Complex> dataCol (itsMS, itsSolveColName);
  // Complex values are separated in real and imaginary.
  // Loop through all rows in the current solve domain.
  for (unsigned int rowinx=0; rowinx<itsSolveRows.nelements(); rowinx++) {
    bool showd = false;
    //bool showd = rowinx==0;
    ///for (unsigned int rowinx=0; rowinx<5; rowinx++) {
    int rownr = itsSolveRows(rowinx);
    ///    MeqPointDFT::doshow = rownr==44;
    uInt ant1 = itsMSCol.antenna1()(rownr);
    uInt ant2 = itsMSCol.antenna2()(rownr);
    bool showdd = (ant1==0 && ant2==4);
    Assert (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
	    &&  itsBLIndex(ant1,ant2) >= 0);
    int blindex = itsBLIndex(ant1,ant2);
    ///    if (MeqPointDFT::doshow) {
      ///      cout << "Info: " << ant1 << ' ' << ant2 << ' ' << blindex << endl;
      ///    }
    double time = itsMSCol.time()(rownr);
    double step = itsMSCol.interval()(rownr);
    MeqDomain domain(time-step/2, time+step/2, startFreq, endFreq);
    MeqRequest request(domain, 1, nrchan, itsNrScid);
    MeqJonesExpr& expr = *(itsExpr[blindex]);
    expr.calcResult (request);
    // Form the equations for this row.
    // Make a default derivative vector with values 0.
    MeqMatrix defaultDeriv (DComplex(0,0), 1, nrchan);
    const complex<double>* defaultDerivPtr = defaultDeriv.dcomplexStorage();
    // Get the data of this row for the given channels.
    Matrix<Complex> data = dataCol.getSlice (rownr, dataSlicer);
    int npol = data.shape()(0);
    // Calculate the derivatives and get pointers to them.
    // Use the default if no perturbed value defined.
    vector<const complex<double>*> derivs(npol*itsNrScid);
    bool foundDeriv = false;
    if (showd) {
      cout << "xx val " << expr.getResult11().getValue() << endl;;
      cout << "xy val " << expr.getResult12().getValue() << endl;;
      cout << "yx val " << expr.getResult21().getValue() << endl;;
      cout << "yy val " << expr.getResult22().getValue() << endl;;
    }
    if (showdd) {
      cout << "rownr " << rowinx << ' ' << rownr << endl;
      cout << "first data value " << data(0,0) << endl;
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
      if (npol == 4) {
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
      if (npol > 1) {
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
	  derivs[scinx + (npol-1)*itsNrScid] = val.dcomplexStorage();
	  foundDeriv = true;
	} else {
	  derivs[scinx + (npol-1)*itsNrScid] = defaultDerivPtr;
	}
      }
    }
    // Only add to solver if at least one derivative was found.
    // Otherwise these data are not dependent on the solvable parameters.
    if (foundDeriv) {
      // Get pointer to array storage; the data in it is contiguous.
      Complex* dataPtr = &(data(0,0));
      if (showdd) {
	cout << "first dataptr value " << *dataPtr << endl;
      }
      vector<double> derivVec(2*itsNrScid);
      double* derivReal = &(derivVec[0]);
      double* derivImag = &(derivVec[itsNrScid]);
      // Fill in all equations.
      if (npol == 1) {
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
      } else if (npol == 2) {
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
      } else if (npol == 4) {
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
  }
  if (Debug(1)) timer.show("fill ");

  Assert (nrpoint >= itsNrScid);
  // Solve the equation.
  uInt rank;
  double fit;
  cdebug(1) << "Solution before: " << itsSolution << endl;
  cout << "Solution before: " << itsSolution << endl;
  // It looks as if LSQ has a bug so that solveLoop and getCovariance
  // interact badly (maybe both doing an invert).
  // So make a copy to separate them.
  Matrix<double> covar;
  Vector<double> errors;
  LSQaips tmpSolver = itsSolver;
  ///tmpSolver.getCovariance (covar);
  tmpSolver.getErrors (errors);
  int nrs = itsSolution.nelements();
  Vector<double> sol(nrs);
  double* solData = itsSolution.doubleStorage();
  for (int i=0; i<itsSolution.nelements(); i++) {
    sol[i] = solData[i];
  }
  bool solFlag = itsSolver.solveLoop (fit, rank, sol, useSVD);
  for (int i=0; i<itsSolution.nelements(); i++) {
    solData[i] = sol[i];
  }
  if (Debug(1)) timer.show("solve");
  cdebug(1) << "Solution after:  " << itsSolution << endl;
  cout << "Solution after:  " << itsSolution << endl;
  
  // Update all parameters.
  const vector<MeqParm*>& parmList = MeqParm::getParmList();
  int i=0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (itsIsParmSolvable[i]) {
      (*iter)->update (itsSolution);
    }
    i++;
  }

  GlishRecord rec;
  rec.add ("solflag", solFlag);
  rec.add ("sol", GlishArray(sol));
  rec.add ("rank", Int(rank));
  rec.add ("fit", fit);
  rec.add ("diag", GlishArray(errors));
  rec.add ("covar", GlishArray(covar));
  rec.add ("mu", itsSolver.getWeightedSD());
  rec.add ("stddev", itsSolver.getSD());
  rec.add ("chi", itsSolver.getChi());

  itsMS.unlock();
  return rec;
}

//----------------------------------------------------------------------
//
// ~saveParms
//
// Save parameters which have an updated warning.
//
//----------------------------------------------------------------------
void MeqCalibrater::saveParms()
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cdebug(1) << "saveParms" << endl;

  Assert (!itsSolution.isNull()  &&  itsSolution.nx() == itsNrScid);
  int i=0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (itsIsParmSolvable[i]) {
      (*iter)->save();
    }
    i++;
  }
  // Unlock the parm tables.
  itsMEP.unlock();
  itsGSMMEP.unlock();
}

//----------------------------------------------------------------------
//
// ~predict
//
// Predict visibilities for the current time domain and save
// in the column with name modelDataColName.
//
//----------------------------------------------------------------------
void MeqCalibrater::predict (const String& modelDataColName)
{
  cdebug(1) << "predict('" << modelDataColName << "')" << endl;

  if (itsCurRows.nelements() == 0) {
    throw AipsError("nextInterval needs to be done before predict");
  }
  Timer timer;

  ////  itsMS.reopenRW();
  // Add the dataColName column if not existing yet.
  if (! itsMS.tableDesc().isColumn(modelDataColName)) {
    ArrayColumnDesc<Complex> mdcol(modelDataColName);
    itsMS.addColumn (mdcol);
    cdebug(1) << "Added column " << modelDataColName << " to the MS" << endl;
  }

  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  int nrchan = 1+itsLastChan-itsFirstChan;

  ArrayColumn<Complex> mdcol(itsMS, modelDataColName);

  // Loop through all rows in the current solve domain.
  for (unsigned int i=0; i<itsCurRows.nelements(); i++) {
    int rownr = itsCurRows(i);
    ///    cout << "Processing row " << rownr << " out of "
      ///	 << itsCurRows.nelements() << endl;
    ///    MeqPointDFT::doshow = rownr==44;
    uInt ant1 = itsMSCol.antenna1()(rownr);
    uInt ant2 = itsMSCol.antenna2()(rownr);
    Assert (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
	    &&  itsBLIndex(ant1,ant2) >= 0);
    int blindex = itsBLIndex(ant1,ant2);
    ///    if (MeqPointDFT::doshow) {
      ///      cout << "Info: " << ant1 << ' ' << ant2 << ' ' << blindex << endl;
      ///    }
    double time = itsMSCol.time()(rownr);
    double step = itsMSCol.interval()(rownr);
    MeqDomain domain(time-step/2, time+step/2, startFreq, endFreq);
    MeqRequest request(domain, 1, nrchan, itsNrScid);
    itsExpr[blindex]->calcResult (request);
    // Write the requested data into the MS.
    // Use the same polarizations as for the original data.
    IPosition shp = itsMSCol.data().shape(rownr);
    Assert (shp(1) == itsNrChan);
    Matrix<Complex> data(shp);
    Slice sliceFreq(itsFirstChan, nrchan);
    // Store the DComplex results into the Complex data array.
    Array<Complex> tmp0 (data(Slice(0,1), sliceFreq));
    convertArray (tmp0,
      itsExpr[blindex]->getResult11().getValue().getDComplexMatrix());
    if (4 == shp(0)) {
      Array<Complex> tmp1 (data(Slice(1,1), sliceFreq));
      convertArray (tmp1,
	itsExpr[blindex]->getResult12().getValue().getDComplexMatrix());
      Array<Complex> tmp2 (data(Slice(2,1), sliceFreq));
      convertArray (tmp2,
	itsExpr[blindex]->getResult21().getValue().getDComplexMatrix());
      Array<Complex> tmp3 (data(Slice(3,1), sliceFreq));
      convertArray (tmp3,
	itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
    } else if (2 == shp(0)) {
      Array<Complex> tmp1 (data(Slice(1,1), sliceFreq));
      convertArray (tmp1,
	itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
    } else if (1 != shp(0)) {
      throw AipsError("Number of polarizations should be 1, 2, or 4");
    }
    ///    if (MeqPointDFT::doshow) cout << "result: " << data << endl;
    mdcol.put (rownr, data);
  }

  itsMS.unlock();
  timer.show();
  if (Debug(1)) timer.show();
}

//----------------------------------------------------------------------
//
// ~saveResidualData
//
// Save the colA - colB in residualCol.
//
//----------------------------------------------------------------------
void MeqCalibrater::saveResidualData()
{
  cout << "saveResidualData to '" << itsResColName << "', data in '";
  cout << itsSolveColName << "')" << endl;

  // Make sure the MS is writable.
  itsMS.reopenRW();
  // Create the column if it does not exist yet.
  // Make it an indirect variable shaped column.
  if (! itsMS.tableDesc().isColumn (itsResColName)) {
    ArrayColumnDesc<Complex> cdesc(itsResColName);
    itsMS.addColumn (cdesc);
    itsMS.flush();
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
  Slicer dataSlicer (IPosition(2,0,itsFirstChan),
		     IPosition(2,Slicer::MimicSource,nrchan));

  ROArrayColumn<Complex> dataCol(itsMS, itsSolveColName);
  ArrayColumn<Complex> rescol(itsMS, itsResColName);

  // Loop through all rows in the current solve domain.
  for (unsigned int i=0; i<itsSolveRows.nelements(); i++) {
    int rownr = itsSolveRows(i);
    ///    cout << "Processing row " << rownr << " out of "
      ///	 << itsSolveRows.nelements() << endl;
    ///    MeqPointDFT::doshow = rownr==44;
    uInt ant1 = itsMSCol.antenna1()(rownr);
    uInt ant2 = itsMSCol.antenna2()(rownr);
    Assert (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
	    &&  itsBLIndex(ant1,ant2) >= 0);
    int blindex = itsBLIndex(ant1,ant2);
    ///    if (MeqPointDFT::doshow) {
      ///      cout << "Info: " << ant1 << ' ' << ant2 << ' ' << blindex << endl;
      ///    }
    double time = itsMSCol.time()(rownr);
    double step = itsMSCol.interval()(rownr);
    MeqDomain domain(time-step/2, time+step/2, startFreq, endFreq);
    MeqRequest request(domain, 1, nrchan, itsNrScid);
    itsExpr[blindex]->calcResult (request);
    //    itsResExpr[blindex]->calcResult (request);
    // Set shape of residual data cell to that of the data.
    rescol.setShape (rownr, dataCol.shape(rownr));
    // Subtract predicted from data and write into the MS.
    // Use the same polarizations as for the original data.
    Matrix<Complex> data = dataCol.getSlice (rownr, dataSlicer);
    int npol = data.shape()(0);
    Slice sliceFreq(0, nrchan);
    // Convert the DComplex results to a Complex data array.
    Matrix<Complex> tmp(IPosition(2,1,nrchan));
    convertArray (tmp,
      itsExpr[blindex]->getResult11().getValue().getDComplexMatrix());
    Matrix<Complex> tmp0 (data(Slice(0,1), sliceFreq));
    tmp0 -= tmp;
    if (4 == npol) {
      convertArray (tmp,
	itsExpr[blindex]->getResult12().getValue().getDComplexMatrix());
      Matrix<Complex> tmp1 (data(Slice(1,1), sliceFreq));
      tmp1 -= tmp;
      convertArray (tmp,
	itsExpr[blindex]->getResult21().getValue().getDComplexMatrix());
      Matrix<Complex> tmp2 (data(Slice(2,1), sliceFreq));
      tmp2 -= tmp;
      convertArray (tmp,
	itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
      Matrix<Complex> tmp3 (data(Slice(3,1), sliceFreq));
      tmp3 -= tmp;
    } else if (2 == npol) {
      convertArray (tmp,
	itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
      Matrix<Complex> tmp1 (data(Slice(1,1), sliceFreq));
      tmp1 -= tmp;
    } else if (1 != npol) {
      throw AipsError("Number of polarizations should be 1, 2, or 4");
    }
    ///    if (MeqPointDFT::doshow) cout << "result: " << data << endl;
    rescol.putSlice (rownr, dataSlicer, data);
  }
  itsMS.flush();
  itsMS.unlock();
  // From now on the residual column name has to be used in the solve.
  itsSolveColName = itsResColName;
}

//----------------------------------------------------------------------
//
// ~addParm
//
// Add the result of a parameter for the current domain to a GlishRecord
// for the purpose of passing the information back to a glish script.
//
//----------------------------------------------------------------------
void MeqCalibrater::addParm(const MeqParm& parm, bool denormalize,
			    GlishRecord& rec)
{
  GlishRecord parmRec;

  MeqMatrix m;
  m = MeqMatrix (double(), itsNrScid, 1);
  
  parmRec.add("parmid", Int(parm.getParmId()));

  try {
    parm.getCurrentValue(m, denormalize);
    GlishArray ga(m.getDoubleMatrix());
    parmRec.add("value",  ga);
  } catch (...) {
    parmRec.add("value", "<?>");
  }
  
  rec.add(parm.getName(), parmRec);
}

//----------------------------------------------------------------------
//
// ~getParms
//
// Get a description of the parameters whose name matches the
// parmPatterns pattern. The description shows the result of the
// evaluation of the parameter on the current time domain.
//
//----------------------------------------------------------------------
GlishRecord MeqCalibrater::getParms(Vector<String>& parmPatterns,
				    Vector<String>& excludePatterns,
				    int isSolvable, bool denormalize)
{
  cdebug(1) << "getParms: " << endl;
  if (itsCurRows.nelements() == 0) {
    throw AipsError("nextInterval needs to be done before getParms");
  }
  GlishRecord rec;
  //  vector<MeqParm*> parmVector;

  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  // Convert patterns to regexes.
  vector<Regex> parmRegex;
  for (unsigned int i=0; i<parmPatterns.nelements(); i++) {
    parmRegex.push_back (Regex::fromPattern(parmPatterns[i]));
  }
  vector<Regex> excludeRegex;
  for (unsigned int i=0; i<excludePatterns.nelements(); i++) {
    excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
  }
  //
  // Find all parms matching the parmPatterns
  // Exclude them if matching an excludePattern
  //
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    bool ok = true;
    if (isSolvable == 0) {
      ok = !((*iter)->isSolvable());
    } else if (isSolvable > 0) {
      ok = ((*iter)->isSolvable());
    }
    if (ok) {
      String parmName ((*iter)->getName());

      for (vector<Regex>::const_iterator incIter = parmRegex.begin();
	   incIter != parmRegex.end();
	   incIter++)
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
	    addParm (**iter, denormalize, rec);
	  }
	  break;
	}
      }
    }
  }
  return rec;
}

//----------------------------------------------------------------------
//
// ~getParmNames
//
// Get the names of the parameters whose name matches the parmPatterns.
// Exclude the names that match the excludePatterns.
// E.g. getParmNames("*") returns all parameter names.
//
//----------------------------------------------------------------------
GlishArray MeqCalibrater::getParmNames(Vector<String>& parmPatterns,
				       Vector<String>& excludePatterns)
{
  const int PARMNAMES_CHUNKSIZE = 100;
  int maxlen = PARMNAMES_CHUNKSIZE;
  int current=0;
  Vector<String> parmNameVector(maxlen);
  //  vector<MeqParm*> parmVector;

  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cdebug(1) << "getParmNames: " << endl;

  // Convert patterns to regexes.
  vector<Regex> parmRegex;
  for (unsigned int i=0; i<parmPatterns.nelements(); i++) {
    parmRegex.push_back (Regex::fromPattern(parmPatterns[i]));
  }
  vector<Regex> excludeRegex;
  for (unsigned int i=0; i<excludePatterns.nelements(); i++) {
    excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
  }
  //
  // Find all parms matching the parmPatterns
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
	    if (current >= maxlen)
	    {
	      maxlen += PARMNAMES_CHUNKSIZE;
	      parmNameVector.resize(maxlen, True);
	    }
	    parmNameVector[current++] = parmName;
	  }
	  break;
	}
      }
    }
  }

  parmNameVector.resize(current, True);
  GlishArray arr(parmNameVector);

  return arr;
}

//----------------------------------------------------------------------
//
// ~getSolveDomain
//
// Store the current solve domain in a GlishRecord for the purpose
// of passing it back to the calling glish script.
//
//----------------------------------------------------------------------
GlishRecord MeqCalibrater::getSolveDomain()
{
  GlishRecord rec;
  rec.add("startx", itsSolveDomain.startX());
  rec.add("endx",   itsSolveDomain.endX());
  rec.add("starty", itsSolveDomain.startY());
  rec.add("endy",   itsSolveDomain.endY());
  return rec;
}

//----------------------------------------------------------------------
//
// ~select
//
// Create a subset of the MS using the given selection string
// (which must be the WHERE part of a TaQL command).
// Also the channels to be used can be specified.
// This selection has to be done before the loop over domains.
//
//----------------------------------------------------------------------
Int MeqCalibrater::select(const String& where, int firstChan, int lastChan)
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
  cdebug(1) << "select " << where << "  channels=[" << itsFirstChan
	    << ',' << itsLastChan << ']' << endl;
  Assert (itsFirstChan <= itsLastChan);
  if (where.empty()  ||  where == "*") {
    itsIter = TableIterator (itsSelMS, "TIME");
  } else {
    Table selms = tableCommand ("select from $1 where " + where, itsSelMS);
    AssertMsg (selms.nrow() > 0, "No matching rows found in select");
    itsIter = TableIterator (selms, "TIME");
  }  
  resetIterator();
  return itsCurRows.nelements();
}

//----------------------------------------------------------------------
//
// ~select
//
// Create a subset of the MS using the given selection string
// (which must be the WHERE part of a TaQL command).
// Also the channels to be used can be specified.
// This selection can be done before each solve.
//
//----------------------------------------------------------------------
Int MeqCalibrater::solveselect(const String& where)
{
  cdebug(1) << "solveselect " << where << endl;
  if (where.empty()  ||  where == "*") {
    itsSolveRows.reference (itsCurRows);
  } else {
    Table curms (itsMS(itsCurRows));
    Table selms = tableCommand ("select from $1 where " + where, curms);
    AssertMsg (selms.nrow() > 0, "No matching rows found in solveselect");
    Vector<uInt> rownrs (selms.rowNumbers(itsMS));
    itsSolveRows.reference (rownrs);
  }  
  itsMS.unlock();
  return itsSolveRows.nelements();
}

//----------------------------------------------------------------------
//
// ~fillUVW
//
// Calculate the station UVW coordinates from the MS.
//
//----------------------------------------------------------------------
void MeqCalibrater::fillUVW()
{
  cdebug(1) << "get UVW coordinates from MS" << endl;
  int nant = itsStatUVW.size();
  vector<bool> statFnd (nant);
  vector<bool> statDone (nant);
  vector<double> statuvw(3*nant);
  // Step time by time through the MS.
  TableIterator iter(itsSelMS, "TIME");
  while (!iter.pastEnd()) {
    Table tab = iter.table();
    ROScalarColumn<double> timeCol (tab, "TIME");
    ROScalarColumn<int>    ant1Col (tab, "ANTENNA1");
    ROScalarColumn<int>    ant2Col (tab, "ANTENNA2");
    ROArrayColumn<double>  uvwCol  (tab, "UVW");
    double time = timeCol(0);
    statFnd.assign (statFnd.size(), false);
    int nfnd = 0;
    Vector<int> ant1d = ant1Col.getColumn();
    Vector<int> ant2d = ant2Col.getColumn();
    int* ant1 = &(ant1d(0));
    int* ant2 = &(ant2d(0));
    for (unsigned int i=0; i<ant1d.nelements(); i++) {
      if (!statFnd[ant1[i]]) {
	nfnd++;
	statFnd[ant1[i]] = true;
      }
      if (!statFnd[ant2[i]]) {
	nfnd++;
	statFnd[ant2[i]] = true;
      }
    }
    // Set UVW of first station to 0 (UVW coordinates are relative!).
    Matrix<double> uvwd = uvwCol.getColumn();
    double* uvw = &(uvwd(0,0));
    statDone.assign (statDone.size(), false);
    statuvw[3*ant1[0]]   = 0;
    statuvw[3*ant1[0]+1] = 0;
    statuvw[3*ant1[0]+2] = 0;
    statDone[ant1[0]] = true;
    itsStatUVW[ant1[0]]->set (time, 0, 0, 0);
    int ndone = 1;
    // Loop until all found stations are handled.
    while (ndone < nfnd) {
      int nd = 0;
      for (unsigned int i=0; i<ant1d.nelements(); i++) {
	int a1 = ant1[i];
	int a2 = ant2[i];
	if (!statDone[a2]) {
	  if (statDone[a1]) {
	    statuvw[3*a2]   = uvw[3*i]   - statuvw[3*a1];
	    statuvw[3*a2+1] = uvw[3*i+1] - statuvw[3*a1+1];
	    statuvw[3*a2+2] = uvw[3*i+2] - statuvw[3*a1+2];
	    statDone[a2] = true;
	    itsStatUVW[a2]->set (time, statuvw[3*a2], statuvw[3*a2+1],
				 statuvw[3*a2+2]);
	    ndone++;
	    nd++;
	  }
	} else if (!statDone[a1]) {
	  if (statDone[a2]) {
	    statuvw[3*a1]   = statuvw[3*a2]   - uvw[3*i];
	    statuvw[3*a1+1] = statuvw[3*a2+1] - uvw[3*i+1];
	    statuvw[3*a1+2] = statuvw[3*a2+2] - uvw[3*i+2];
	    statDone[a1] = true;
	    itsStatUVW[a1]->set (time, statuvw[3*a1], statuvw[3*a1+1],
				 statuvw[3*a1+2]);
	    ndone++;
	    nd++;
	  }
	}
	if (ndone == nfnd) {
	  break;
	}
      }
      Assert (nd > 0);
    }
    iter++;
  }
}

//----------------------------------------------------------------------
//
// ~peel
//
// Define the source numbers to use in a peel step.
//
//----------------------------------------------------------------------
Bool MeqCalibrater::peel(const Vector<Int>& peelSourceNrs,
			 const Vector<Int>& extraSourceNrs)
{
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
  cdebug(1) << "peel: sources " << peelSourceNrs << " predicting sources "
	    << sourceNrs << endl;
  Assert (peelSourceNrs.nelements() > 0);
  vector<int> src(sourceNrs.nelements());
  for (unsigned int i=0; i<src.size(); i++) {
    src[i] = sourceNrs[i];
  }
  itsSources.setSelected (src);
  itsPeelSourceNrs.reference (tmpPeel);
  return True;
}

//----------------------------------------------------------------------
//
// ~getResidualData
//
// Get a description of the parameters whose name matches the
// parmPatterns pattern. The description shows the result of the
// evaluation of the parameter on the current time domain.
//
//----------------------------------------------------------------------
GlishRecord MeqCalibrater::getResidualData()
{
  cout << "getResidualData, data in '";
  cout << itsSolveColName << "'" << endl;

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
  Slicer dataSlicer (IPosition(2,0,itsFirstChan),
		     IPosition(2,Slicer::MimicSource,nrchan));

  ROArrayColumn<Complex> dataCol(itsMS, itsSolveColName);

  // Size the result.
  IPosition dataShp = dataCol.shape(itsSolveRows[0]);
  dataShp(1) = nrchan;
  Array<Complex> result(IPosition(3,dataShp(0),nrchan,itsSolveRows.nelements()));
  Vector<Int> ant1Res(itsSolveRows.nelements());
  Vector<Int> ant2Res(itsSolveRows.nelements());
  Vector<Double> timeRes(itsSolveRows.nelements());
  IPosition blcRes(3,0,0,0);
  IPosition trcRes = result.shape() - 1;
  // Loop through all rows in the current solve domain.
  for (unsigned int i=0; i<itsSolveRows.nelements(); i++) {
    int rownr = itsSolveRows(i);
    ///    cout << "Processing row " << rownr << " out of "
      ///	 << itsSolveRows.nelements() << endl;
    ///    MeqPointDFT::doshow = rownr==44;
    uInt ant1 = itsMSCol.antenna1()(rownr);
    uInt ant2 = itsMSCol.antenna2()(rownr);
    Assert (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
	    &&  itsBLIndex(ant1,ant2) >= 0);
    int blindex = itsBLIndex(ant1,ant2);
    ///    if (MeqPointDFT::doshow) {
      ///      cout << "Info: " << ant1 << ' ' << ant2 << ' ' << blindex << endl;
      ///    }
    double time = itsMSCol.time()(rownr);
    double step = itsMSCol.interval()(rownr);
    ant1Res[i] = ant1;
    ant2Res[i] = ant2;
    timeRes[i] = time;
    MeqDomain domain(time-step/2, time+step/2, startFreq, endFreq);
    MeqRequest request(domain, 1, nrchan, itsNrScid);
    // Predict.
    itsExpr[blindex]->calcResult (request);
    // Read the appropriate slice of data and convert to a 2D array.
    blcRes(2) = i;
    trcRes(2) = i;
    Array<Complex> subRes = result(blcRes, trcRes);
    Matrix<Complex> data = subRes.reform (dataShp);
    dataCol.getSlice (rownr, dataSlicer, data);
    int npol = data.shape()(0);
    // Convert the DComplex results to a Complex data array.
    // Subtract predicted from data.
    // Use the same polarizations as for the original data.
    Matrix<Complex> tmp(IPosition(2,1,nrchan));
    convertArray (tmp,
      itsExpr[blindex]->getResult11().getValue().getDComplexMatrix());
    Matrix<Complex> tmp0 (data(Slice(0,1), Slice(0,nrchan)));
    tmp0 -= tmp;
    if (4 == npol) {
      convertArray (tmp,
	itsExpr[blindex]->getResult12().getValue().getDComplexMatrix());
      Matrix<Complex> tmp1 (data(Slice(1,1), Slice(0,nrchan)));
      tmp1 -= tmp;
      convertArray (tmp,
	itsExpr[blindex]->getResult21().getValue().getDComplexMatrix());
      Matrix<Complex> tmp2 (data(Slice(2,1), Slice(0,nrchan)));
      tmp2 -= tmp;
      convertArray (tmp,
	itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
      Matrix<Complex> tmp3 (data(Slice(3,1), Slice(0,nrchan)));
      tmp3 -= tmp;
    } else if (2 == npol) {
      convertArray (tmp,
	itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
      Matrix<Complex> tmp1 (data(Slice(1,1), Slice(0,nrchan)));
      tmp1 -= tmp;
    } else if (1 != npol) {
      throw AipsError("Number of polarizations should be 1, 2, or 4");
    }
  }
  itsMS.unlock();
  GlishRecord rec;
  rec.add ("residuals", GlishArray(result));
  rec.add ("ant1", GlishArray(ant1Res));
  rec.add ("ant2", GlishArray(ant2Res));
  rec.add ("time", GlishArray(timeRes));
  return rec;
}

//----------------------------------------------------------------------
//
// ~getStatistics
//
// Get a description of the parameters whose name matches the
// parmPatterns pattern. The description shows the result of the
// evaluation of the parameter on the current time domain.
//
//----------------------------------------------------------------------
GlishRecord MeqCalibrater::getStatistics (bool detailed, bool clear)
{
  GlishRecord rec;

  cdebug(1) << "getStatistics: " << endl;

  // Get the total counts.
  rec.add ("timecellstotal", MeqHist::merge (itsCelltHist));
  rec.add ("freqcellstotal", MeqHist::merge (itsCellfHist));

  if (detailed) {
    // Get the counts per station.
    int nrant = itsBLIndex.nrow();
    for (int ant2=0; ant2<nrant; ant2++) {
      String str2 = String::toString(ant2);
      for (int ant1=0; ant1<nrant; ant1++) {
	int blindex = itsBLIndex(ant1,ant2);
	if (blindex >= 0) {
	  String str = String::toString(ant1) + '_' + str2;
	  rec.add ("timecells_" + str, itsCelltHist[blindex].get());
	  rec.add ("freqcells_" + str, itsCellfHist[blindex].get());
	}
      }
    }
  }
  if (clear) {
    int nrant = itsBLIndex.nrow();
    for (int ant2=0; ant2<nrant; ant2++) {
      String str2 = String::toString(ant2);
      for (int ant1=0; ant1<nrant; ant1++) {
	int blindex = itsBLIndex(ant1,ant2);
	if (blindex >= 0) {
	  itsCelltHist[blindex].clear();
	  itsCellfHist[blindex].clear();
	}
      }
    }
  }
  return rec;
}

//----------------------------------------------------------------------
//
// ~className
//
// Return the name of this DO class.
//
//----------------------------------------------------------------------
String MeqCalibrater::className() const
{
  return "meqcalibrater";
}

//----------------------------------------------------------------------
//
// ~methods
//
// Return a vector of method names for the DO.
//
//----------------------------------------------------------------------
Vector<String> MeqCalibrater::methods() const
{
  Vector<String> method(17);

  method(0)  = "settimeinterval";
  method(1)  = "resetiterator";
  method(2)  = "nextinterval";
  method(3)  = "clearsolvableparms";
  method(4)  = "setsolvableparms";
  method(5)  = "predict";
  method(6)  = "solve";
  method(7)  = "saveparms";
  method(8)  = "saveresidualdata";
  method(9)  = "getparms";
  method(10) = "getsolvedomain";
  method(11) = "select";
  method(12) = "solveselect";
  method(13) = "peel";
  method(14) = "getresidualdata";
  method(15) = "getstatistics";
  method(16) = "getparmnames";

  return method;
}

//----------------------------------------------------------------------
//
// ~noTraceMethods
//
// ?
//
//----------------------------------------------------------------------
Vector<String> MeqCalibrater::noTraceMethods() const
{
  return methods();
}

//----------------------------------------------------------------------
//
// ~runMethod
//
// Call a method from glish.
//
//----------------------------------------------------------------------
MethodResult MeqCalibrater::runMethod(uInt which,
				      ParameterSet& inputRecord,
				      Bool runMethod)
{
  switch (which) 
  {
  case 0: // settimeinterval
    {
      Parameter<Int> secondsInterval(inputRecord, "secondsinterval",
				     ParameterSet::In);

      if (runMethod) setTimeInterval(secondsInterval());
    }
    break;

  case 1: // resetiterator
    {
      if (runMethod) resetIterator();
    }
    break;

  case 2: // nextinterval
    {
      Parameter<Bool> returnval(inputRecord, "returnval",
				ParameterSet::Out);
      if (runMethod) returnval() = nextInterval();
    }
    break;

  case 3: // clearsolvableparms
    {
      if (runMethod) clearSolvableParms();
    }
    break;

  case 4: // setsolvableparms
    {
      Parameter<Vector<String> > parmPatterns(inputRecord, "parmpatterns",
					     ParameterSet::In);
      Parameter<Vector<String> > excludePatterns(inputRecord, "excludepatterns",
						 ParameterSet::In);
      Parameter<Bool> isSolvable(inputRecord, "issolvable",
				 ParameterSet::In);

      if (runMethod) setSolvableParms(parmPatterns(), excludePatterns(), isSolvable());
    }
    break;

  case 5: // predict
    {
      Parameter<String> modelDataColName(inputRecord, "modeldatacolname",
					 ParameterSet::In);

      if (runMethod) predict(modelDataColName());
    }
    break;

  case 6: // solve
    {
      Parameter<Bool> realsol(inputRecord, "realsol",
			      ParameterSet::In);
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = solve (realsol());
    }
    break;

  case 7: // saveparms
    {
      if (runMethod) saveParms();
    }
    break;

  case 8: // saveresidualdata
    {
      if (runMethod) saveResidualData();
    }
    break;

  case 9: // getparms
    {
      Parameter<Vector<String> > parmPatterns(inputRecord, "parmpatterns",
					      ParameterSet::In);
      Parameter<Vector<String> > excludePatterns(inputRecord, "excludepatterns",
						 ParameterSet::In);
      Parameter<int> isSolvable(inputRecord, "issolvable",
				ParameterSet::In);
      Parameter<bool> denormalize(inputRecord, "denormalize",
				  ParameterSet::In);
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getParms(parmPatterns(), excludePatterns(),
					    isSolvable(), denormalize());
    }
    break;

  case 10: // getsolvedomain
    {
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getSolveDomain();
    }
    break;

  case 11: // select
    {
      Parameter<String> where(inputRecord, "where",
			      ParameterSet::In);
      Parameter<Int> firstChan(inputRecord, "firstchan",
			       ParameterSet::In);
      Parameter<Int> lastChan(inputRecord, "lastchan",
			      ParameterSet::In);
      Parameter<Int> returnval(inputRecord, "returnval",
			       ParameterSet::Out);

      if (runMethod) returnval() = select(where(), firstChan(), lastChan());
    }
    break;

  case 12: // solveselect
    {
      Parameter<String> where(inputRecord, "where",
			      ParameterSet::In);
      Parameter<Int> returnval(inputRecord, "returnval",
			       ParameterSet::Out);

      if (runMethod) returnval() = solveselect(where());
    }
    break;

  case 13: // peel
    {
      Parameter<Vector<Int> > peelSourceNrs(inputRecord, "peelsourcenrs",
					    ParameterSet::In);
      Parameter<Vector<Int> > extraSourceNrs(inputRecord, "extrasourcenrs",
					     ParameterSet::In);

      if (runMethod) peel(peelSourceNrs(), extraSourceNrs());
    }
    break;

  case 14: // getresidualdata
    {
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getResidualData ();
    }
    break;

  case 15: // getstatistics
    {
      Parameter<Bool> detailed(inputRecord, "detailed",
			       ParameterSet::In);
      Parameter<Bool> clear(inputRecord, "clear",
			    ParameterSet::In);
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getStatistics (detailed(), clear());
    }
    break;

  case 16: // getparmnames
    {
      Parameter<Vector<String> > parmPatterns(inputRecord, "parmpatterns",
					      ParameterSet::In);
      Parameter<Vector<String> > excludePatterns(inputRecord, "excludepatterns",
						 ParameterSet::In);
      Parameter<GlishArray> returnval(inputRecord, "returnval",
				      ParameterSet::Out);

      if (runMethod) returnval() = getParmNames(parmPatterns(), excludePatterns());
    }
    break;

  default:
    return error("No such method");
  }

  return ok();
}

//----------------------------------------------------------------------
//
// ~make
//
// Create an instance of the MeqCalibrater class.
//
//----------------------------------------------------------------------
MethodResult MeqCalibraterFactory::make(ApplicationObject*& newObject,
					const String& whichConstructor,
					ParameterSet& inputRecord,
					Bool runConstructor)
{
  MethodResult retval;
  newObject = 0;

  if (whichConstructor == "meqcalibrater")
    {
      Parameter<String>   msName(inputRecord, "msname",   ParameterSet::In);
      Parameter<String> meqModel(inputRecord, "meqmodel", ParameterSet::In);
      Parameter<String> skyModel(inputRecord, "skymodel", ParameterSet::In);
      Parameter<String> dbType  (inputRecord, "dbtype",   ParameterSet::In);
      Parameter<String> dbName  (inputRecord, "dbname",   ParameterSet::In);
      Parameter<String> dbPwd   (inputRecord, "dbpwd",    ParameterSet::In);
      Parameter<Int>        ddid(inputRecord, "ddid",     ParameterSet::In);
      Parameter<Vector<Int> > ant1(inputRecord, "ant1",   ParameterSet::In);
      Parameter<Vector<Int> > ant2(inputRecord, "ant2",   ParameterSet::In);
      Parameter<String> modelType(inputRecord, "modeltype", ParameterSet::In);
      Parameter<Bool>     calcUVW(inputRecord, "calcuvw", ParameterSet::In);
      Parameter<String> dataColName(inputRecord, "datacolname",
				    ParameterSet::In);
      Parameter<String> resColName(inputRecord, "residualcolname",
				    ParameterSet::In);

      if (runConstructor)
	{
	  newObject = new MeqCalibrater(msName(), meqModel(), skyModel(),
					dbType(), dbName(), dbPwd(),
					ddid(), ant1(), ant2(),
					modelType(), calcUVW(),
					dataColName(), resColName());
	}
    }
  else
    {
      retval = String("Unknown constructor") + whichConstructor;
    }

  if (retval.ok() && runConstructor && !newObject)
    {
      retval = "Memory allocation error";
    }

  return retval;
}
