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
#include <MNS/MeqMatrix.h>
#include <MNS/MeqMatrixTmp.h>
#include <MNS/MeqParm.h>
#include <MNS/MeqParmSingle.h>
#include <MNS/MeqPointDFT.h>
#include <MNS/MeqPointSource.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqStation.h>
#include <MNS/MeqStoredParmPolc.h>
#include <MNS/MeqUVWPolc.h>
#include <MNS/MeqWsrtInt.h>
#include <MNS/MeqWsrtPoint.h>
#include <MNS/ParmTable.h>
#include <GSM/SkyModel.h>

#include <Common/Debug.h>

#include <aips/Arrays/ArrayIO.h>
#include <aips/Arrays/ArrayMath.h>
#include <aips/Arrays/ArrayLogical.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/Vector.h>
#include <aips/Exceptions/Error.h>
#include <aips/Functionals/Polynomial.h>
#include <aips/Glish/GlishArray.h>
#include <aips/Glish/GlishRecord.h>
#include <aips/Glish/GlishValue.h>
#include <aips/Mathematics/AutoDiff.h>
#include <aips/Mathematics/Constants.h>
#include <aips/MeasurementSets/MeasurementSet.h>
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
#include <aips/Tables/SetupNewTab.h>
#include <aips/Tables/Table.h>
#include <aips/Tables/TableIter.h>
#include <aips/Utilities/Regex.h>
#include <trial/Fitting/LinearFit.h>
#include <trial/Tasking/MethodResult.h>
#include <trial/Tasking/Parameter.h>

#include <stdexcept>
#include <stdio.h>

//----------------------------------------------------------------------
//
// Local function prototypes
//
//----------------------------------------------------------------------
static MDirection         getPhaseRef  (const MeasurementSet& ms);
static void               getFreq      (const MeasurementSet& ms,
				        double& start, double& end, int& nf);
static vector<MeqStation> fillStations (const MeasurementSet& ms);
static vector<MVBaseline> fillBaselines(Matrix<int>& index,
					const Vector<int>& ant1,
					const Vector<int>& ant2,
					const vector<MeqStation>& stations);
static MeqJonesExpr*      makeWSRTExpr (const MDirection& phaseRef, MeqUVWPolc* uvw,
					ParmTable&, GSM::SkyModel& gsm);
static void               addParm      (MeqParm* parm, GlishRecord* rec);

//----------------------------------------------------------------------
//
// ~getPhaseRef
//
// Get the phase reference of the first field.
//
//----------------------------------------------------------------------
static MDirection getPhaseRef (const MeasurementSet& ms)
{
  MSField mssub(ms.field());
  ROMSFieldColumns mssubc(mssub);
  MDirection dir = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));

  return MDirection::Convert (dir, MDirection::J2000)();
}

//----------------------------------------------------------------------
//
// ~getFreq
//
// Get the frequency domain of the first spectral window.
// So far, only equal frequency spacings are possible.
//
//----------------------------------------------------------------------
static void getFreq (const MeasurementSet& ms, double& start, double& end, int& nf)
{
  MSDataDescription mssub1(ms.dataDescription());
  ROMSDataDescColumns mssub1c(mssub1);
  int spw = mssub1c.spectralWindowId()(0);
  MSSpectralWindow mssub(ms.spectralWindow());
  ROMSSpWindowColumns mssubc(mssub);
  Vector<double> chanFreq = mssubc.chanFreq()(spw);
  Vector<double> chanWidth = mssubc.chanWidth()(spw);
  AssertMsg (allEQ (chanWidth, chanWidth(0)),
	     "Channels must have equal spacings");
  nf = chanWidth.nelements();
  start = chanFreq(0) - chanWidth(0)/2;
  end = start + nf*chanWidth(0);
}

//----------------------------------------------------------------------
//
// ~fillStations
//
// Get all stations.
// Fill their positions as parameters.
//
//----------------------------------------------------------------------
static vector<MeqStation> fillStations (const MeasurementSet& ms)
{
  vector<MeqStation> stations;
  MSAntenna          mssub(ms.antenna());
  ROMSAntennaColumns mssubc(mssub);
  char               str[8];

  for (uInt i=0; i<mssub.nrow(); i++) {
    Vector<Double> antpos = mssubc.position()(i);

    snprintf (str, 8, "%d", i);
    MeqParmSingle* px = new MeqParmSingle ("AntPosX_"+String(str), antpos(0));
    MeqParmSingle* py = new MeqParmSingle ("AntPosY_"+String(str), antpos(1));
    MeqParmSingle* pz = new MeqParmSingle ("AntPosZ_"+String(str), antpos(2));

    stations.push_back (MeqStation(px, py, pz));
  }

  return stations;
}

//----------------------------------------------------------------------
//
// ~fillBaselines
//
// Get all baselines from the MS.
// Create an MVBaseline object for each of them.
//
//----------------------------------------------------------------------
static vector<MVBaseline> fillBaselines (Matrix<int>& index,
				  const Vector<int>& ant1,
				  const Vector<int>& ant2,
				  const vector<MeqStation>& stations)
{
  vector<MVBaseline> baselines;
  MeqDomain          domain;
  MeqRequest         req(domain, 1, 1);

  baselines.reserve (ant1.nelements());
  index.resize (stations.size(), stations.size());
  index = -1;

  for (unsigned int i=0; i<ant1.nelements(); i++)
  {
    uInt a1 = ant1(i);
    uInt a2 = ant2(i);

    Assert (a1 < stations.size()  &&  a2 < stations.size());
    index(a1,a2) = baselines.size();
    MVPosition pos1
      (stations[a1].getPosX()->getResult(req).getValue().getDouble(),
       stations[a1].getPosY()->getResult(req).getValue().getDouble(),
       stations[a1].getPosZ()->getResult(req).getValue().getDouble());
    MVPosition pos2
      (stations[a2].getPosX()->getResult(req).getValue().getDouble(),
       stations[a2].getPosY()->getResult(req).getValue().getDouble(),
       stations[a2].getPosZ()->getResult(req).getValue().getDouble());

    baselines.push_back (MVBaseline (pos1, pos2));
  }

  return baselines;
}

//----------------------------------------------------------------------
//
// ~makeWSRTExpr
//
// Make the expression tree for a single baseline of the WSRT.
//
//----------------------------------------------------------------------
static MeqJonesExpr* makeWSRTExpr (const MDirection& phaseRef, MeqUVWPolc* uvw,
				   ParmTable&, GSM::SkyModel& gsm)
{
  // Represent the stations by constant parms.
  MeqExpr* stat1_11 = new MeqParmSingle ("Station.RT_0.11",
					 double(1));
  MeqExpr* stat1_12 = new MeqParmSingle ("Station.RT_0.12",
					 double(0));
  MeqExpr* stat1_21 = new MeqParmSingle ("Station.RT_0.21",
					 double(0));
  MeqExpr* stat1_22 = new MeqParmSingle ("Station.RT_0.22",
					 double(1));
  MeqJonesNode* stat1 = new MeqJonesNode (stat1_11, stat1_12,
					  stat1_21, stat1_22);
  MeqExpr* stat2_11 = new MeqParmSingle ("Station.RT_1.11",
					 double(1));
  MeqExpr* stat2_12 = new MeqParmSingle ("Station.RT_1.12",
					 double(0));
  MeqExpr* stat2_21 = new MeqParmSingle ("Station.RT_1.21",
					 double(0));
  MeqExpr* stat2_22 = new MeqParmSingle ("Station.RT_1.22",
					 double(1));
  MeqJonesNode* stat2 = new MeqJonesNode (stat2_11, stat2_12,
					  stat2_21, stat2_22);

  // Get the point sources from the GSM.
  vector<MeqPointSource> src;
  gsm.getPointSources (src);

  // Create the DFT kernel.
  MeqPointDFT* dft = new MeqPointDFT (src, phaseRef, uvw);
  MeqWsrtPoint* pnt = new MeqWsrtPoint (src, dft);

  return new MeqWsrtInt (pnt, stat1, stat2);
}

//----------------------------------------------------------------------
//
// ~calcUVWPolc
//
// Calculate the polynomial for the UVW coordinates for the current
// MS.
//
//----------------------------------------------------------------------
void MeqCalibrater::calcUVWPolc()
{
  // We only handle field 0 and spectral window 0 (for the time being).
  Table selMS = itsMS(itsMS.col("FIELD_ID")==0 && itsMS.col("DATA_DESC_ID")==0);
  Block<String> keys(2);
  keys[0] = "ANTENNA1";
  keys[1] = "ANTENNA2";
  selMS = selMS.sort (keys);
  cout << itsMS.nrow() << ' ' << selMS.nrow() << endl;

  // Iterate through the MS per baseline.
  TableIterator iter (selMS, keys, TableIterator::Ascending,
		      TableIterator::NoSort);

  // calculate the uvwpolc for the whole domain of the measurementset
  while (!iter.pastEnd())
  {
    Table tab = iter.table();
    ROArrayColumn<double> uvwcol(tab, "UVW");
    ROScalarColumn<double> timcol(tab, "TIME");

    Vector<double> dt = timcol.getColumn();
    Matrix<double> uvws = uvwcol.getColumn();

    // should be a loop over one hour periods of data
    // for (..)
    // {

    itsUVWPolc.calcCoeff (dt, uvws);

    // }

    iter++;
  }
}

//----------------------------------------------------------------------
//
// ~MeqCalibrater
//
// Constructor. Initialize a MeqCalibrater object.
//
// Add MODEL_DATA column to MS if not yet present.
// Calculate UVW polc.
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the WSRT.
//
//----------------------------------------------------------------------
MeqCalibrater::MeqCalibrater(const String& msName,
			     const String& meqModel,
			     const String& skyModel,
			     const uInt    spw)
  :
  itsMS(msName, Table::Update),
  itsMEP(meqModel + ".MEP"),
  itsGSMTable(skyModel + ".GSM"),
  itsGSM(itsGSMTable),
  itsRequest(MeqDomain(),0,0),
  itsFitValue(10.0)
{
  cout << "MeqCalibrater constructor (";
  cout << "'" << msName   << "', ";
  cout << "'" << meqModel << "', ";
  cout << "'" << skyModel << "', ";
  cout << spw << ")" << endl;

  calcUVWPolc();

  // We only handle field 0 and spectral window 0 (for the time being).
  // Get all baselines by doing a unique sort.
  Table selMS = itsMS(itsMS.col("FIELD_ID")==0 && itsMS.col("DATA_DESC_ID")==0);
  Block<String> keys(2);
  keys[0] = "ANTENNA1";
  keys[1] = "ANTENNA2";
  selMS = selMS.sort (keys);
  Table sselMS = selMS.sort (keys, Sort::Ascending,
			     Sort::NoDuplicates + Sort::InsSort);
  cout << itsMS.nrow() << ' ' << selMS.nrow() << ' ' << sselMS.nrow() << endl;

  // Now generate the baseline objects for them.
  // If we ever want to solve for station positions, we cannot use the
  // fixed MVBaseline objects, but instead they should be recalculated
  // after each solve iteration.
  ROScalarColumn<int> ant1(sselMS, "ANTENNA1");
  ROScalarColumn<int> ant2(sselMS, "ANTENNA2");
  Matrix<int> index;
  itsStations  = fillStations(itsMS);
  itsBaselines = fillBaselines (index, ant1.getColumn(), ant2.getColumn(), itsStations);

#if 0
  Block<String> timeKeys(1);
  timeKeys[0] = "TIME";
#endif

  // Iterate through the MS per baseline.
  itsIter = TableIterator(selMS, keys, TableIterator::Ascending,
			  TableIterator::InsSort);

  // Get phase reference (for field 0), frequencies (for spw 0)
  // and all station positions from it.
  MDirection phaseRef = getPhaseRef(itsMS);

  // Set up the expression tree for a single baseline.
  itsExprTree = makeWSRTExpr (phaseRef, &itsUVWPolc, itsMEP, itsGSM);

  cout << "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
       << ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor << endl;
  cout << "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
       << ' ' << MeqResultRep::nctor + MeqResultRep::ndtor << endl;

  //
  // Initialize the domain for iteration
  //

  //
  // calculate frequency domain
  //
  getFreq (itsMS, itsStartFreq, itsEndFreq, itsNrChan);
  cout << "Freq: " << itsStartFreq << ' ' << itsEndFreq << " (" <<
    itsEndFreq - itsStartFreq << " Hz) " << itsNrChan << endl;

  //
  // calculate first time domain
  //
  ROScalarColumn<double> timcol(itsIter.table(), "TIME");
  Vector<double>         dt = timcol.getColumn();
  ROScalarColumn<double> intcol(itsIter.table(), "INTERVAL");

  double step      = intcol(0);
  int    nrTime    = dt.nelements();
  double startTime = dt(0) - (step / 2);
  double endTime   = dt(nrTime - 1) + (step / 2);

  itsStartTime = startTime;

  Assert (near(endTime - startTime, nrTime * step));

  //itsDomain.setDomain(startTime, endTime, itsStartFreq, itsEndFreq);
  itsRequest.setDomain(MeqDomain(startTime, endTime, itsStartFreq, itsEndFreq),
		       nrTime, itsNrChan);
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
  itsMS.flush();

  cout << "MeqCalibrater destructor" << endl;
}

//----------------------------------------------------------------------
//
// ~initParms
//
// Initialize all parameters in the expression tree by calling
// its initDomain method.
//
//----------------------------------------------------------------------
void MeqCalibrater::initParms(MeqDomain& theDomain)
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  int spidIndex = 0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
      spidIndex += (*iter)->initDomain (theDomain, spidIndex);
    }
  }
}

//----------------------------------------------------------------------
//
// ~setTimeIntervalSize
//
// Set the time domain (interval) for which the solver will solve.
// The predict could be on a smaller domain (but not larger) than
// this domain.
//
//----------------------------------------------------------------------
void MeqCalibrater::setTimeIntervalSize(uInt secondsInterval)
{
  cout << "setTimeIntervalSize = " << secondsInterval << endl;
}

//----------------------------------------------------------------------
//
// ~resetTimeIterator
//
// Start iteration over time domains from the beginning.
//
//----------------------------------------------------------------------
void MeqCalibrater::resetTimeIterator()
{
  cout << "resetTimeIterator" << endl;
  Block<Int> columns(0);

  itsIter.reset();

  //
  // set the domain to the beginning
  //
  initParms(itsDomain);

#if 1
  // Dummy implementation
  itsFitValue = 10.0;
#endif
}

//----------------------------------------------------------------------
//
// ~nextTimeInterval
//
// Move to the next time interval (domain).
//
//----------------------------------------------------------------------
void MeqCalibrater::nextTimeInterval()
{
#if 0

  for (int i=0; i < itsRequest.nx(); i++)
  {
    itsIter++;
    if (itsIter.pastEnd()) break;

    // fill itsRowNumbers vector of RowNumbers
  }
  itsDataRead = false;

#else

  itsIter++;

  if (itsIter.pastEnd()) return;

#endif


  //
  // calculate first time domain
  //
  ROScalarColumn<double> timcol(itsIter.table(), "TIME");
  Vector<double>         dt = timcol.getColumn();
  ROScalarColumn<double> intcol(itsIter.table(), "INTERVAL");

  double step      = intcol(0);
  int    nrTime    = dt.nelements();
  double startTime = dt(0) - (step / 2);
  double endTime   = dt(nrTime - 1) + (step / 2);

  Assert (near(endTime - startTime, nrTime * step));
  //itsDomain.setDomain(startTime, endTime, itsStartFreq, itsEndFreq);
  itsRequest.setDomain(MeqDomain(startTime, endTime, itsStartFreq, itsEndFreq),
		       nrTime, itsNrChan);
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

  cout << "clearSolvableParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
      cout << "clearSolvable: " << (*iter)->getName() << endl;
      (*iter)->setSolvable(false);
    }
  }
}

//----------------------------------------------------------------------
//
// ~setSolvableParm
//
// Set the solvable flag (true or false) on all parameters whose
// name matches the parmPatterns pattern.
//
//----------------------------------------------------------------------
void MeqCalibrater::setSolvableParms (Vector<String>& parmPatterns,
				      Bool isSolvable)
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "setSolvableParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    for (int i=0; i < (int)parmPatterns.nelements(); i++)
    {
      Regex pattern(Regex::fromPattern(parmPatterns[i]));

      if (*iter)
      {
	if (String((*iter)->getName()).matches(pattern))
	{
	  cout << "setSolvable: " << (*iter)->getName() << endl;
	  (*iter)->setSolvable(isSolvable);
	}
      }
    }
  }

  cout << "isSolvable = " << isSolvable << endl;
}

//----------------------------------------------------------------------
//
// ~predict
//
// Predict visibilities for the current time domain.
//
//----------------------------------------------------------------------
void MeqCalibrater::predict(const String& modelDataColName)
{
  cout << "predict('" << modelDataColName << "')" << endl;

  cout << "timer:  ";
  Timer timer;
  itsExprTree->calcResult (itsRequest);
  timer.show();

  //      cout << expr->getResult11().getValue() << endl;
  //      cout << expr->getResult12().getValue() << endl;
  //      cout << expr->getResult21().getValue() << endl;
  //      cout << expr->getResult22().getValue() << endl;
  
  // Write the predicted data into the MODEL_DATA column.
  // They have to be converted from double to float complex.
  its_xx=itsExprTree->getResult11().getValue().getDComplexMatrix();
  its_xy=itsExprTree->getResult12().getValue().getDComplexMatrix();
  its_yx=itsExprTree->getResult21().getValue().getDComplexMatrix();
  its_yy=itsExprTree->getResult22().getValue().getDComplexMatrix();
}

//----------------------------------------------------------------------
//
// ~solve
//
// Solve for the solvable parameters on the current time domain.
//
//----------------------------------------------------------------------
Double MeqCalibrater::solve()
{
  cout << "solve" << endl;

  if (!itsDataRead)
  {
    // read data from itsIter.table()

    itsDataRead = true;
  }
    

  // invoke solver for the current domain

  // update the paramters

#if 0
  // Dummy implementation
  itsFitValue /= 2.0;
#else
  itsFitValue = 0;
#endif

  return itsFitValue;
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

  cout << "saveParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
      cout << "saveParm: " << (*iter)->getName() << endl;
      (*iter)->save();
    }
  }

  // Save the source parameters.
  // SkyModel writes all sources into the table. So when using the
  // existing table, all sources would be duplicated.
  // Therefore the table is recreated.
  // First get the table description and name.
  TableDesc td = itsGSMTable.tableDesc();
  String name  = itsGSMTable.tableName() + "_saved";
  // Close the table by assigining an empty object to it.
  itsGSMTable = Table();
  // Rrecreate the table and save the parameters.
  SetupNewTable newtab(name, td, Table::New);
  Table tab(newtab);
  itsGSM.store(tab);

#if 0
  // Make the new table the current one.
  itsGSMTable = tab;
#endif
}

//----------------------------------------------------------------------
//
// ~savePredictedData
//
// Save the predicted data for the current time domain in column
// with name modelDataColName.
//
//----------------------------------------------------------------------
void MeqCalibrater::savePredictedData(const String& modelDataColName)
{
  cout << "savePredictedData('" << modelDataColName << "')" << endl;

  // Add the dataColName column if not existing yet.
  if (! itsMS.tableDesc().isColumn(modelDataColName)) {
    ArrayColumnDesc<Complex> mdcol(modelDataColName);
    itsMS.addColumn (mdcol);
    cout << "Added column " << modelDataColName << " to MS" << endl;
  }

  ArrayColumn<Complex> mdcol(itsIter.table(), modelDataColName);
  ROArrayColumn<Complex> dcol(itsIter.table(), "DATA");
  int npol = dcol(0).shape()(0);

  Matrix<complex<float> > data(npol, itsRequest.ny());
  for (int j=0; j < itsRequest.nx(); j++)
  {
    for (int i=0; i<itsRequest.ny(); i++)
    {
      const DComplex& its_xxji = its_xx(j,i);
      data(0,i) = complex<float> (its_xxji.real(), its_xxji.imag());
      //	  if (abs(data(0,i)) < 0.4) {
      //	    cout << cnt << ' ' << j << ' ' << i << ' ' << abs(data(0,i)) << endl;
      //	  }
      if (npol > 2)
      {
	const DComplex& its_xyji = its_xy(j,i);
	data(1,i) = complex<float> (its_xyji.real(), its_xyji.imag());
	const DComplex& its_yxji = its_yx(j,i);
	data(2,i) = complex<float> (its_yxji.real(), its_yxji.imag());
      }

      const DComplex& its_yyji = its_yy(j,i);
      data(npol-1,i) = complex<float> (its_yyji.real(), its_yyji.imag());
    }
    
    mdcol.put (j, data);
  }
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
  itsMS.reopenRW();
  // Create the column if it does not exist yet.
  // Make it an indirect variable shaped column.
  if (! itsMS.tableDesc().isColumn (residualColName)) {
    ArrayColumnDesc<Complex> cdesc(residualColName);
    itsMS.addColumn (cdesc);
    itsMS.flush();
  }

  // Loop through all rows in the MS and store the result.
  ROArrayColumn<Complex> colA(itsMS, colAName);
  ROArrayColumn<Complex> colB(itsMS, colBName);
  ArrayColumn<Complex> colR(itsMS, residualColName);
  for (uInt i=0; i<itsMS.nrow(); i++) {
    colR.put (i, colA(i) - colB(i));
  }
  itsMS.flush();

  cout << "saveResidualData('" << colAName << "', '" << colBName << "', ";
  cout << "'" << residualColName << "')" << endl;
}

//----------------------------------------------------------------------
//
// ~addParm
//
// Add the result of a parameter for the current domain to a GlishRecord
// for the purpose of passing the information back to a glish script.
//
//----------------------------------------------------------------------
static void addParm(MeqParm* parm, GlishRecord* rec)
{
  GlishRecord parmRec;

#if 0

  //
  // In order to fill in the result for the current domain
  // we need some specification of the current domain.
  // The domain should be set in the nextTimeInterval method.
  //

  MeqRequest mr(MeqDomain(0,1,0,1), 1, 1);
  MeqMatrix mm = parm->getResult(mr).getValue();	    
  if (mm.isDouble()) rec->add("result", mm.getDoubleMatrix());
  else               rec->add("result", mm.getDComplexMatrix());
#endif

  parmRec.add("parmid", Int(parm->getParmId()));
  
  rec->add(parm->getName(), parmRec);
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
  GlishRecord rec;
  bool parmDone = false;
  vector<MeqParm*> parmVector;

  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "getParms: " << endl;


  //
  // Find all parms matching the parmPatterns
  //
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    parmDone = false;

    for (int i=0; i < (int)parmPatterns.nelements(); i++)
    {
      Regex pattern(Regex::fromPattern(parmPatterns[i]));

      if (*iter && !parmDone)
      {
	if (String((*iter)->getName()).matches(pattern))
	{
	  parmDone = true;
	  parmVector.push_back(*iter);
	}
      }
    }
  }

  //
  // Make record of parms to return, but exclude parms
  // matching the excludePatterns
  //
  for (vector<MeqParm*>::iterator iter = parmVector.begin();
       iter != parmVector.end();
       iter++)
  {
    if (0 == excludePatterns.nelements())
    {
      if (*iter) addParm((*iter), &rec);
    }
    else
    {
      for (int i=0; i < (int)excludePatterns.nelements(); i++)
      {
	Regex pattern(Regex::fromPattern(excludePatterns[i]));
	
	if (*iter)
	{
	  if (! String((*iter)->getName()).matches(pattern))
	  {
	    addParm((*iter), &rec);
	  }
	}
      }
    }
  }

  return rec;
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
  GlishRecord      rec;
  const MeqDomain& domain = itsRequest.domain();

  rec.add("offsetx", domain.offsetX() - itsStartTime);
  rec.add("scalex",  domain.scaleX());
  rec.add("offsety", domain.offsetY());
  rec.add("scaley",  domain.scaleY());

  return rec;
}

//----------------------------------------------------------------------
//
// ~timeIteratorPastEnd
//
// Indicate whether we've reached the end of time for the current MS.
//
//----------------------------------------------------------------------
Bool MeqCalibrater::timeIteratorPastEnd()
{
  return itsIter.pastEnd();
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
  Vector<String> method(13);

  method(0)  = "settimeintervalsize";
  method(1)  = "resettimeiterator";
  method(2)  = "nexttimeinterval";
  method(3)  = "clearsolvableparms";
  method(4)  = "setsolvableparms";
  method(5)  = "predict";
  method(6)  = "solve";
  method(7)  = "saveparms";
  method(8)  = "savepredicteddata";
  method(9)  = "saveresidualdata";
  method(10) = "getparms";
  method(11) = "getsolvedomain";
  method(12) = "timeiteratorpastend";

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
  case 0: // settimeintervalsize
    {
      Parameter<Int> secondsInterval(inputRecord, "secondsinterval",
				     ParameterSet::In);

      if (runMethod) setTimeIntervalSize(secondsInterval());
    }
    break;

  case 1: // resettimeiterator
    {
      if (runMethod) resetTimeIterator();
    }
    break;

  case 2: // nexttimeinterval
    {
      if (runMethod) nextTimeInterval();
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
      Parameter<Bool> isSolvable(inputRecord, "issolvable",
				 ParameterSet::In);

      if (runMethod) setSolvableParms(parmPatterns(), isSolvable());
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
      Parameter<Double> returnval(inputRecord, "returnval",
				  ParameterSet::Out);

      if (runMethod) returnval() = solve();
    }
    break;

  case 7: // saveparms
    {
      if (runMethod) saveParms();
    }
    break;

  case 8: // savepredicteddata
    {
      Parameter<String> modelDataColName(inputRecord, "modeldatacolname",
					 ParameterSet::In);

      if (runMethod) savePredictedData(modelDataColName());
    }
    break;

  case 9: // saveresidualdata
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

  case 10: // getparms
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

  case 11: // getsolvedomain
    {
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getSolveDomain();
    }
    break;

  case 12: // timeiteratorpastend
    {
      Parameter<Bool> returnval(inputRecord, "returnval",
				ParameterSet::Out);

      if (runMethod) returnval() = timeIteratorPastEnd();
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
      Parameter<Int>         spw(inputRecord, "spw",      ParameterSet::In);

      if (runConstructor)
	{
	  newObject = new MeqCalibrater(msName(), meqModel(),
					skyModel(), spw());
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
