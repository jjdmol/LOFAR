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

#include <Common/Debug.h>

#include <aips/Arrays/ArrayIO.h>
#include <aips/Arrays/ArrayMath.h>
#include <aips/Arrays/ArrayLogical.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/Slice.h>
#include <aips/Arrays/Slicer.h>
#include <aips/Arrays/Vector.h>
#include <aips/Exceptions/Error.h>
#include <aips/Functionals/Polynomial.h>
#include <aips/Glish/GlishArray.h>
#include <aips/Glish/GlishRecord.h>
#include <aips/Glish/GlishValue.h>
#include <aips/Mathematics/AutoDiff.h>
#include <aips/Mathematics/Constants.h>
#include <aips/MeasurementSets/MSAntenna.h>
#include <aips/MeasurementSets/MSAntennaColumns.h>
#include <aips/MeasurementSets/MSDataDescription.h>
#include <aips/MeasurementSets/MSDataDescColumns.h>
#include <aips/MeasurementSets/MSField.h>
#include <aips/MeasurementSets/MSFieldColumns.h>
#include <aips/MeasurementSets/MSSpectralWindow.h>
#include <aips/MeasurementSets/MSSpWindowColumns.h>
#include <aips/Measures/MDirection.h>
#include <aips/Measures/MeasConvert.h>
#include <aips/OS/Timer.h>
#include <aips/Quanta/MVBaseline.h>
#include <aips/Quanta/MVPosition.h>
#include <aips/Tables/ArrColDesc.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/ColumnDesc.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/ExprNodeSet.h>
#include <aips/Tables/SetupNewTab.h>
#include <aips/Tables/TableParse.h>
#include <aips/Utilities/Regex.h>
#include <trial/Fitting/LinearFit.h>
#include <trial/Tasking/MethodResult.h>
#include <trial/Tasking/Parameter.h>

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
  for (uInt i=0; i<ant1.nelements(); i++) {
    for (int j=0; j<2; j++) {
      int ant = ant1(i);
      if (j==1) ant = ant2(i);
      Assert (ant < nrant);
      if (itsStations[ant] == 0) {
	// Store each position as a constant parameter.
	// Use the antenna name as the parameter name.
	Vector<Double> antpos = mssubc.position()(ant);
	String name = mssubc.name()(ant);
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
  // Get the point sources from the GSM.
  itsGSM.getPointSources (itsSources);
  for (unsigned int i=0; i<itsSources.size(); i++) {
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
      // Expression to calculate UVW per station
      itsStatUVW[i] = new MeqStatUVW (itsStations[i], &itsPhaseRef);
      // Expression to calculate contribution per station per source.
      itsStatSrc[i] = new MeqStatSources (itsStatUVW[i], &itsSources);
      // Expression representing station parameters.
      MeqExpr* frot = new MeqStoredParmPolc ("frot." +
					     itsStations[i]->getName(),
					     &itsMEP);
      MeqExpr* drot = new MeqStoredParmPolc ("drot." +
					     itsStations[i]->getName(),
					     &itsMEP);
      MeqExpr* dell = new MeqStoredParmPolc ("dell." +
					     itsStations[i]->getName(),
					     &itsMEP);
      MeqExpr* gain11 = new MeqStoredParmPolc ("gain.11." +
					       itsStations[i]->getName(),
					       &itsMEP);
      MeqExpr* gain22 = new MeqStoredParmPolc ("gain.22." +
					       itsStations[i]->getName(),
					       &itsMEP);
      itsStatExpr[i] = new MeqStatExpr (frot, drot, dell, gain11, gain22);
    }
  }    

  // Make an expression for each baseline.
  itsExpr.resize (itsBaselines.size());
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
	MeqWsrtPoint* pnt = new MeqWsrtPoint (itsSources, dft,
					      &itsCelltHist[blindex],
					      &itsCellfHist[blindex]);
	itsExpr[blindex] = new MeqWsrtInt (pnt, itsStatExpr[ant1],
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
			           uInt    ddid,
			     const Vector<Int>& ant1,
			     const Vector<Int>& ant2)
  :
  itsMS       (msName, Table::Update),
  itsMSCol    (itsMS),
  itsMEP      (meqModel + ".MEP"),
  itsGSMTable (skyModel + ".GSM"),
  itsGSM      (itsGSMTable),
  itsSolver   (1, LSQBase::REAL)
{
  cdebug(1) << "MeqCalibrater constructor (";
  cdebug(1) << "'" << msName   << "', ";
  cdebug(1) << "'" << meqModel << "', ";
  cdebug(1) << "'" << skyModel << "', ";
  cdebug(1) << ddid << ")" << endl;

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
  makeWSRTExpr();

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
      itsSolution = MeqMatrix (complex<double>(), itsNrScid, 1);
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
    itsSolver.set (itsNrScid, 1, 0);
  }
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
  itsSolveDomain = MeqDomain(timeStart, timeStart + nrtim*timeStep,
			     itsStartFreq + itsFirstChan*itsStepFreq,
			     itsStartFreq + (itsLastChan+1)*itsStepFreq);
  initParms (itsSolveDomain);
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

  cdebug(1) << "setSolvableParms" << endl;
  cdebug(1) << "isSolvable = " << isSolvable << endl;

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
GlishRecord MeqCalibrater::solve (const String& colName)
{
  cdebug(1) << "solve using column " << colName << endl;

  if (itsCurRows.nelements() == 0) {
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

  ROArrayColumn<Complex> dataCol (itsMS, colName);
  // Complex values are separated in real and imaginary.
  // Loop through all rows in the current solve domain.
  for (unsigned int rowinx=0; rowinx<itsCurRows.nelements(); rowinx++) {
    ///for (unsigned int rowinx=0; rowinx<5; rowinx++) {
    int rownr = itsCurRows(rowinx);
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
    MeqJonesExpr& expr = *(itsExpr[blindex]);
    expr.calcResult (request);
    // Form the equations for this row.
    // Make a default derivative vector with values 0.
    MeqMatrix defaultDeriv (Matrix<DComplex> (1, nrchan, DComplex()));
    const complex<double>* defaultDerivPtr = defaultDeriv.dcomplexStorage();
    // Get the data of this row for the given channels.
    Matrix<Complex> data = dataCol.getSlice (rownr, dataSlicer);
    int npol = data.shape()(0);
    // Calculate the derivatives and get pointers to them.
    // Use the default if no perturbed value defined.
    vector<const complex<double>*> derivs(npol*itsNrScid);
    bool foundDeriv = false;
    for (int scinx=0; scinx<itsNrScid; scinx++) {
      MeqMatrix val;
      if (expr.getResult11().isDefined(scinx)) {
	val = expr.getResult11().getPerturbedValue(scinx);
	///cout << "Pert  " << val << endl;
	val -= expr.getResult11().getValue();
	///cout << "Diff  " << val << endl;
	val /= expr.getResult11().getPerturbation(scinx);
	///cout << "Deriv " << val << endl;
	derivs[scinx] = val.dcomplexStorage();
	foundDeriv = true;
      } else {
	derivs[scinx] = defaultDerivPtr;
      }
      if (npol == 4) {
	if (expr.getResult12().isDefined(scinx)) {
	  val = expr.getResult12().getPerturbedValue(scinx);
	  val -= expr.getResult12().getValue();
	  val /= expr.getResult12().getPerturbation(scinx);
	  derivs[scinx + itsNrScid] = val.dcomplexStorage();
	  foundDeriv = true;
	} else {
	  derivs[scinx + itsNrScid] = defaultDerivPtr;
	}
	if (expr.getResult21().isDefined(scinx)) {
	  val = expr.getResult21().getPerturbedValue(scinx);
	  val -= expr.getResult21().getValue();
	  val /= expr.getResult21().getPerturbation(scinx);
	  derivs[scinx + 2*itsNrScid] = val.dcomplexStorage();
	  foundDeriv = true;
	} else {
	  derivs[scinx + 2*itsNrScid] = defaultDerivPtr;
	}
      }
      if (npol > 1) {
	if (expr.getResult22().isDefined(scinx)) {
	  val = expr.getResult22().getPerturbedValue(scinx);
	  val -= expr.getResult22().getValue();
	  val /= expr.getResult22().getPerturbation(scinx);
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
      vector<double> derivReal(itsNrScid);
      vector<double> derivImag(itsNrScid);
      // Fill in all equations.
      if (npol == 1) {
	const MeqMatrix& xx = expr.getResult11().getValue();
	for (int i=0; i<nrchan; i++) {
	  for (int j=0; j<itsNrScid; j++) {
	    derivReal[j] = derivs[j][i].real();
	    derivImag[j] = derivs[j][i].imag();
	  }
	  DComplex diff (dataPtr[i].real(), dataPtr[i].imag());
	  diff -= xx.getDComplex(0,i);
	  double val = diff.real();
	  itsSolver.makeNorm (&(derivReal[0]), 1., &val);
	  val = diff.imag();
	  itsSolver.makeNorm (&(derivImag[0]), 1., &val);
	  nrpoint++;
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
	    itsSolver.makeNorm (&(derivReal[0]), 1., &val);
	    val = diff.imag();
	    itsSolver.makeNorm (&(derivImag[0]), 1., &val);
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
	    itsSolver.makeNorm (&(derivReal[0]), 1., &val);
	    val = diff.imag();
	    itsSolver.makeNorm (&(derivImag[0]), 1., &val);
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
	    }
	    ///cout << endl;
	    DComplex diff (dataPtr[i*2].real(), dataPtr[i*2].imag());
	    ///cout << "Value " << diff << ' ' << xx.getDComplex(0,i) << endl;
	    diff -= xx.getDComplex(0,i);
	    double val = diff.real();
	    itsSolver.makeNorm (&(derivReal[0]), 1., &val);
	    val = diff.imag();
	    itsSolver.makeNorm (&(derivImag[0]), 1., &val);
	    nrpoint++;
	  }
	}
	{
	  const MeqMatrix& xy = expr.getResult12().getValue();
	  for (int i=0; i<nrchan; i++) {
	    for (int j=0; j<itsNrScid; j++) {
	      derivReal[j] = derivs[j+itsNrScid][i].real();
	      derivImag[j] = derivs[j+itsNrScid][i].imag();
	    }
	    DComplex diff (dataPtr[i*2+1].real(), dataPtr[i*2+1].imag());
	    diff -= xy.getDComplex(0,i);
	    double val = diff.real();
	    itsSolver.makeNorm (&(derivReal[0]), 1., &val);
	    val = diff.imag();
	    itsSolver.makeNorm (&(derivImag[0]), 1., &val);
	    nrpoint++;
	  }
	}
	{
	  const MeqMatrix& yx = expr.getResult21().getValue();
	  for (int i=0; i<nrchan; i++) {
	    for (int j=0; j<itsNrScid; j++) {
	      derivReal[j] = derivs[j+2*itsNrScid][i].real();
	      derivImag[j] = derivs[j+2*itsNrScid][i].imag();
	    }
	    DComplex diff (dataPtr[i*2+2].real(), dataPtr[i*2+2].imag());
	    diff -= yx.getDComplex(0,i);
	    double val = diff.real();
	    itsSolver.makeNorm (&(derivReal[0]), 1., &val);
	    val = diff.imag();
	    itsSolver.makeNorm (&(derivImag[0]), 1., &val);
	    nrpoint++;
	  }
	}
	{
	  const MeqMatrix& yy = expr.getResult22().getValue();
	  for (int i=0; i<nrchan; i++) {
	    for (int j=0; j<itsNrScid; j++) {
	      derivReal[j] = derivs[j+3*itsNrScid][i].real();
	      derivImag[j] = derivs[j+3*itsNrScid][i].imag();
	    }
	    DComplex diff (dataPtr[i*2+3].real(), dataPtr[i*2+3].imag());
	    diff -= yy.getDComplex(0,i);
	    double val = diff.real();
	    itsSolver.makeNorm (&(derivReal[0]), 1., &val);
	    val = diff.imag();
	    itsSolver.makeNorm (&(derivImag[0]), 1., &val);
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
  double stddev;
  double mu;
  cdebug(1) << "Solution before: " << itsSolution << endl;
  int nrs = itsSolution.nelements();
  Vector<double> sol(nrs);
  complex<double>* solData = itsSolution.dcomplexStorage();
  // It looks as if LSQ has a bug so that solveLoop and getCovariance
  // interact badly (maybe both doing an invert).
  // So make a copy to separate them.
  Matrix<double> covar;
  FitLSQ tmpSolver = itsSolver;
  tmpSolver.getCovariance (covar);
  for (int i=0; i<nrs; i++) {
    sol[i] = solData[i].real();
  }
  Assert (itsSolver.solveLoop (fit, rank, sol, stddev, mu));
  if (Debug(1)) timer.show("solve");
  for (int i=0; i<nrs; i++) {
    solData[i] = complex<double>(sol[i], 0);
  }
  cdebug(1) << "Solution after:  " << itsSolution << endl;
  
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
  rec.add ("sol", GlishArray(sol));
  rec.add ("rank", Int(rank));
  rec.add ("fit", fit);
  rec.add ("diag", GlishArray(covar.diagonal()));
  rec.add ("covar", GlishArray(covar));
  rec.add ("mu", mu);
  rec.add ("stddev", stddev);
  rec.add ("chi", itsSolver.getChi());

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
      (*iter)->update (itsSolution);
    }
    i++;
  }

  // Save the source parameters.
  // SkyModel writes all sources into the table. So when using the
  // existing table, all sources would be duplicated.
  // Therefore the table is recreated.
  // First get the table description and name.
  TableDesc td = itsGSMTable.tableDesc();
  String name  = itsGSMTable.tableName();
  // Close the table by assigning an empty object to it.
  itsGSMTable = Table();
  // Recreate the table and save the parameters.
  SetupNewTable newtab(name+"_new", td, Table::New);
  Table tab(newtab);
  itsGSM.store(tab);
  // Make the new table the current one and rename it to the original name.
  tab.rename (name, Table::New);
  itsGSMTable = tab;
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
void MeqCalibrater::saveResidualData(const String& colAName,
				     const String& colBName,
				     const String& residualColName)
{
  if (colAName == residualColName)
  {
    throw(AipsError("residualcolname can not be the same as colaname"));
  }
  if (colBName == residualColName)
  {
    throw(AipsError("residualcolname can not be the same as colbname"));
  }
  if (residualColName == "DATA")
  {
    throw(AipsError("residualcolname can not be the DATA column"));
  }

  // Make sure the MS is writable.
  itsSelMS.reopenRW();
  // Create the column if it does not exist yet.
  // Make it an indirect variable shaped column.
  if (! itsMS.tableDesc().isColumn (residualColName)) {
    ArrayColumnDesc<Complex> cdesc(residualColName);
    itsMS.addColumn (cdesc);
    itsMS.flush();
  }

  // Loop through all rows in the MS and store the result.
  ROArrayColumn<Complex> colA(itsSelMS, colAName);
  ROArrayColumn<Complex> colB(itsSelMS, colBName);
  ArrayColumn<Complex> colR(itsSelMS, residualColName);
  for (uInt i=0; i<itsSelMS.nrow(); i++) {
    colR.put (i, colA(i) - colB(i));
  }
  itsSelMS.flush();

  cdebug(1) << "saveResidualData('" << colAName << "', '" << colBName << "', ";
  cdebug(1) << "'" << residualColName << "')" << endl;
}

//----------------------------------------------------------------------
//
// ~addParm
//
// Add the result of a parameter for the current domain to a GlishRecord
// for the purpose of passing the information back to a glish script.
//
//----------------------------------------------------------------------
void MeqCalibrater::addParm(const MeqParm& parm, GlishRecord& rec)
{
  GlishRecord parmRec;

  MeqMatrix m;
  m = MeqMatrix (complex<double>(), itsNrScid, 1);
  
  parmRec.add("parmid", Int(parm.getParmId()));

  try
  {
    parm.getCurrentValue(m);
    Matrix<DComplex> coefs(m.nx(), m.ny());
    
    for (int i=0; i < m.nx(); i++)
    {
      for (int j=0; j < m.ny(); j++)
      {
	coefs(i,j) = m.getDComplex(i,j);
      }
    }

    GlishArray ga(coefs);
    parmRec.add("value",  ga);
  }
  catch (...)
  {
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
				    Vector<String>& excludePatterns)
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
	    addParm (**iter, rec);
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
//
//----------------------------------------------------------------------
Bool MeqCalibrater::select(const String& where, int firstChan, int lastChan)
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
  if (! where.empty()) {
    Table selms = tableCommand ("select from $1 where " + where, itsSelMS);
    Assert (selms.nrow() > 0);
    itsIter = TableIterator (selms, "TIME");
  } else {
    itsIter = TableIterator (itsSelMS, "TIME");
  }  
  resetIterator();
  return True;
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
  Vector<String> method(14);

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
  method(12) = "getstatistics";
  method(13) = "getparmnames";

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
      Parameter<String> colName(inputRecord, "datacolname",
				ParameterSet::In);
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = solve (colName());
    }
    break;

  case 7: // saveparms
    {
      if (runMethod) saveParms();
    }
    break;

  case 8: // saveresidualdata
    {
      Parameter<String> colAName(inputRecord, "colaname",
				 ParameterSet::In);
      Parameter<String> colBName(inputRecord, "colbname",
				 ParameterSet::In);
      Parameter<String> residualColName(inputRecord, "residualcolname",
					ParameterSet::In);

      if (runMethod) saveResidualData(colAName(),
				      colBName(),
				      residualColName());
    }
    break;

  case 9: // getparms
    {
      Parameter<Vector<String> > parmPatterns(inputRecord, "parmpatterns",
					      ParameterSet::In);
      Parameter<Vector<String> > excludePatterns(inputRecord, "excludepatterns",
						 ParameterSet::In);
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getParms(parmPatterns(), excludePatterns());
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

      if (runMethod) select(where(), firstChan(), lastChan());
    }
    break;

  case 12: // getstatistics
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

  case 13: // getparmnames
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
      Parameter<Int>        ddid(inputRecord, "ddid",     ParameterSet::In);
      Parameter<Vector<Int> > ant1(inputRecord, "ant1",   ParameterSet::In);
      Parameter<Vector<Int> > ant2(inputRecord, "ant2",   ParameterSet::In);

      if (runConstructor)
	{
	  newObject = new MeqCalibrater(msName(), meqModel(),
					skyModel(), ddid(),
					ant1(), ant2());
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
