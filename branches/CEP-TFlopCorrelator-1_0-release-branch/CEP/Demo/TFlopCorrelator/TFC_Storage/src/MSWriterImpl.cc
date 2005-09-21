// MSWriterImpl.cc: implementation for filling a MeasurementSet
//
//  Copyright (C) 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <TFC_Storage/MSWriterImpl.h>
# include <ms/MeasurementSets.h>
# include <tables/Tables/IncrementalStMan.h>
# include <tables/Tables/StandardStMan.h>
# include <tables/Tables/TiledShapeStMan.h>
# include <tables/Tables/SetupNewTab.h>
# include <tables/Tables/TableDesc.h>
# include <tables/Tables/TableRecord.h>
# include <casa/Arrays/Vector.h>
# include <casa/Arrays/Cube.h>
# include <casa/Arrays/Matrix.h>
# include <casa/Arrays/ArrayMath.h>
# include <casa/Containers/Block.h>
# include <measures/Measures/MPosition.h>
# include <measures/Measures/MBaseline.h>
# include <measures/Measures/Muvw.h>
# include <measures/Measures/MeasTable.h>
# include <measures/Measures/Stokes.h>
# include <measures/Measures/MeasConvert.h>
# include <casa/Quanta/MVEpoch.h>
# include <casa/Quanta/MVDirection.h>
# include <casa/Quanta/MVPosition.h>
# include <casa/Quanta/MVBaseline.h>
# include <casa/OS/Time.h>
# include <casa/OS/Time.h>
# include <casa/BasicSL/Constants.h>
# include <casa/Utilities/Assert.h>
# include <casa/Exceptions/Error.h>
#include <casa/Arrays/Slicer.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace casa;

MSWriterImpl::MSWriterImpl (const char* msName, double startTime, double timeStep,
			    int nantennas, const vector<double>& antPos)
: itsNrBand   (0),
  itsNrField  (0),
  itsNrAnt    (nantennas),
  itsNrTimes  (0),
  itsTimeStep (timeStep),
  itsStartTime(MVEpoch(startTime).getTime().getValue("s")),
  itsNrPol    (0),
  itsNrChan   (0),
  itsPolnr    (0),
  itsBaselines(0),
  itsFrame    (0),
  itsMS       (0),
  itsMSCol    (0)
{
  AlwaysAssert (nantennas >= 0, AipsError);
  // Get the position of Westerbork in WGS84.
  MPosition arrayPosTmp;
  AlwaysAssert (MeasTable::Observatory(arrayPosTmp, "WSRT"), AipsError);
  itsArrayPos = new MPosition;
  *itsArrayPos = MPosition::Convert (arrayPosTmp, MPosition::WGS84) ();
  const MVPosition& arrayMVPos = itsArrayPos->getValue();
  itsArrayLon = arrayMVPos.getLong();
  double arrayLat = arrayMVPos.getLat();
  double arrayAlt = arrayMVPos.getLength("m").getValue();
  // Convert the relative antenna distances (in m) to long/lat.
  // Note that 4.e7 meters = 360 degrees.
  // Keep the antenna positions in ITRF coordinates.
  Block<MPosition> antMPos(nantennas);
  for (int i=0; i<nantennas; i++) {
    double lat = arrayLat + antPos[3*i] / 4.e7 * 2 * C::pi;
    double lon = itsArrayLon + antPos[3*i+1] / (std::cos(lat)*4.e7) * 2 * C::pi;
    double alt = arrayAlt + antPos[3*i+2];
    MPosition mpos (MVPosition(alt, lon, lat), MPosition::WGS84);
    antMPos[i] = MPosition::Convert (mpos, MPosition::ITRF) ();
  }
  // Create the MS.
  createMS (msName, antMPos);
  itsNrPol  = new Block<Int>;
  itsNrChan = new Block<Int>;
  itsPolnr  = new Block<Int>;
  // Find the baselines in X,Y,Z.
  itsBaselines = new Cube<double> (3, nantennas, nantennas);
  Cube<double>& basel = *itsBaselines;
  for (int i=0; i<nantennas; i++) {
    const Vector<Double>& pos1 = antMPos[i].getValue().getValue();
    for (int j=0; j<nantennas; j++) {
      const Vector<Double>& pos2 = antMPos[j].getValue().getValue();
      basel(0,i,j) = pos2(0) - pos1(0);
      basel(1,i,j) = pos2(1) - pos1(1);
      basel(2,i,j) = pos2(2) - pos1(2);
    }
  }
  // Make a frame for the calculation of apparent coordinates.
  // Store position of array center in it.
  itsFrame = new MeasFrame(*itsArrayPos);
}

MSWriterImpl::~MSWriterImpl()
{
  if (itsMS != 0) {
    updateTimes();
  }
  delete itsNrPol;
  delete itsNrChan;
  delete itsPolnr;
  delete itsBaselines;
  delete itsArrayPos;
  delete itsFrame;
  delete itsMSCol;
  delete itsMS;
}

int MSWriterImpl::nrPolarizations() const
{
  return itsMS->polarization().nrow();
}


void MSWriterImpl::createMS (const char* msName,
			     const Block<MPosition>& antPos)
{
  // Get the MS main default table description..
  TableDesc td = MS::requiredTableDesc();
  // Add the data column and its unit.
  MS::addColumnToDesc(td, MS::DATA, 2);
  td.rwColumnDesc(MS::columnName(MS::DATA)).rwKeywordSet().
                                                      define("UNIT","Jy");
  // Store the data and flags using the TiledStMan.
  Vector<String> tsmNames(2);
  tsmNames(0) = MS::columnName(MS::DATA);
  tsmNames(1) = MS::columnName(MS::FLAG);
  td.defineHypercolumn("TiledData", 3, tsmNames);
  // Setup the new table.
  // Most columns use the IncrStMan; some use others.
  SetupNewTable newTab(msName, td, Table::New);
  IncrementalStMan incrStMan("ISMData");
  StandardStMan    stanStMan;
  TiledShapeStMan  tiledStMan("TiledData", IPosition(3,4,16,512));
  newTab.bindAll (incrStMan);
  newTab.bindColumn(MS::columnName(MS::ANTENNA1),stanStMan);
  newTab.bindColumn(MS::columnName(MS::ANTENNA2),stanStMan);
  newTab.bindColumn(MS::columnName(MS::DATA),tiledStMan);
  newTab.bindColumn(MS::columnName(MS::FLAG),tiledStMan);
  newTab.bindColumn(MS::columnName(MS::UVW),stanStMan);
  // Create the MS and its subtables.
  // Get access to its columns.
  itsMS = new MeasurementSet(newTab);
  itsMSCol = new MSMainColumns(*itsMS);
  // Create all subtables.
  // Do this after the creation of optional subtables,
  // so the MS will know about those optional sutables.
  itsMS->createDefaultSubtables (Table::New);
  // Fill various subtables.
  fillAntenna (antPos);
  fillFeed();
  fillProcessor();
  fillObservation();
  fillState();
}

int MSWriterImpl::addBand (int npolarizations, int nchannels,
			   double refFreq, double chanWidth)
{
  AlwaysAssert (nchannels > 0, AipsError);
  Vector<double> chanWidths(nchannels);
  chanWidths = chanWidth;
  Vector<double> chanFreqs(nchannels);
  indgen (chanFreqs, refFreq - (nchannels-1)*chanWidth/2., chanWidth);
  return addBand (npolarizations, nchannels, refFreq, chanFreqs, chanWidths);
}

int MSWriterImpl::addBand (int npolarizations, int nchannels,
			   double refFreq, const double* chanFreqs,
			   const double* chanWidths)
{
  AlwaysAssert (nchannels > 0, AipsError);
  IPosition shape(1, nchannels);
  Vector<double> freqs (shape, const_cast<double*>(chanFreqs), SHARE);
  Vector<double> widths(shape, const_cast<double*>(chanWidths), SHARE);
  return addBand (npolarizations, nchannels, refFreq, freqs, widths);
}

int MSWriterImpl::addBand (int npolarizations, int nchannels,
			   double refFreq, const Vector<double>& chanFreqs,
			   const Vector<double>& chanWidths)
{
  AlwaysAssert (npolarizations==1 || npolarizations==2 || npolarizations==4,
		AipsError);
  AlwaysAssert (nchannels > 0, AipsError);
  // Find out if this nr of polarizations has already been given.
  Int polnr = -1;
  for (Int i=0; i<itsNrBand; i++) {
    if (npolarizations == (*itsNrPol)[i]) {
      polnr = (*itsPolnr)[i];
      break;
    }
  }
  // If not, add an entry to the POLARIZATION subtable.
  if (polnr < 0) {
    polnr = addPolarization (npolarizations);
  }
  // Add a row to the DATADESCRIPTION subtable.
  MSDataDescription msdd = itsMS->dataDescription();
  MSDataDescColumns msddCol(msdd);
  uInt rownr = msdd.nrow();
  msdd.addRow();
  msddCol.spectralWindowId().put (rownr, rownr);
  msddCol.polarizationId().put (rownr, polnr);
  msddCol.flagRow().put (rownr, False);
  // Add a row to the SPECTRALWINDOW subtable.
  // Find the total bandwidth from the minimum and maximum.
  Vector<double> stFreqs = chanFreqs - chanWidths/2.;
  Vector<double> endFreqs = chanFreqs + chanWidths/2.;
  double totalBW = max(endFreqs) - min(stFreqs);
  MSSpectralWindow msspw = itsMS->spectralWindow();
  MSSpWindowColumns msspwCol(msspw);
  msspw.addRow();
  msspwCol.numChan().put (rownr, nchannels);
  msspwCol.name().put (rownr, "");
  msspwCol.refFrequency().put (rownr, refFreq);
  msspwCol.chanFreq().put (rownr, chanFreqs);
  msspwCol.chanWidth().put (rownr, chanWidths);
  msspwCol.measFreqRef().put (rownr, MFrequency::TOPO);
  msspwCol.effectiveBW().put (rownr, chanWidths);
  msspwCol.resolution().put (rownr, chanWidths);
  msspwCol.totalBandwidth().put (rownr, totalBW);
  msspwCol.netSideband().put (rownr, 0);
  msspwCol.ifConvChain().put (rownr, 0);
  msspwCol.freqGroup().put (rownr, 0);
  msspwCol.freqGroupName().put (rownr, "");
  msspwCol.flagRow().put (rownr, False);
  // Now add the band to the internal blocks.
  itsNrBand++;
  itsNrPol->resize (itsNrBand);
  itsNrChan->resize (itsNrBand);
  itsPolnr->resize (itsNrBand);
  (*itsNrPol)[itsNrBand-1] = npolarizations;
  (*itsNrChan)[itsNrBand-1] = nchannels;
  (*itsPolnr)[itsNrBand-1] = polnr;
  msspw.flush();
  msdd.flush();
  return itsNrBand-1;
}

int MSWriterImpl::addPolarization (int npolarizations)
{
  MSPolarization mspol = itsMS->polarization();
  MSPolarizationColumns mspolCol(mspol);
  uInt rownr = mspol.nrow();
  Vector<Int> corrType(npolarizations);
  corrType(0) = Stokes::XX;
  if (npolarizations == 2) {
    corrType(1) = Stokes::YY;
  } else if (npolarizations == 4) {
    corrType(1) = Stokes::XY;
    corrType(2) = Stokes::YX;
    corrType(3) = Stokes::YY;
  }
  Matrix<Int> corrProduct(2, npolarizations);
  for (Int i=0; i<npolarizations; i++) {
    corrProduct(0,i) = Stokes::receptor1(Stokes::type(corrType(i)));
    corrProduct(1,i) = Stokes::receptor2(Stokes::type(corrType(i)));
  }
  // Fill the columns.
  mspol.addRow();
  mspolCol.numCorr().put (rownr, npolarizations);
  mspolCol.corrType().put (rownr, corrType);
  mspolCol.corrProduct().put (rownr, corrProduct);
  mspolCol.flagRow().put (rownr, False);
  mspol.flush();
  return rownr;
}

int MSWriterImpl::addField (double azimuth, double elevation)
{
  // Convert AZEL to J2000 RADEC.
  Quantity qtime(itsStartTime, "s");
  itsFrame->set (MEpoch(qtime, MEpoch::UTC));
  MVDirection azel (Quantity(azimuth,"deg"), Quantity(elevation,"deg"));
  MDirection::Ref inref(MDirection::AZEL, *itsFrame);
  MDirection indir(azel, inref);
  Vector<MDirection> outdir(1);
  outdir(0) = MDirection::Convert (indir, MDirection::J2000) ();
  // Put the direction into the FIELD subtable.
  {
    MSField msfield = itsMS->field();
    MSFieldColumns msfieldCol(msfield);
    uInt rownr = msfield.nrow();
    msfield.addRow();
    msfieldCol.name().put (rownr, "BEAM_" + String::toString(rownr));
    msfieldCol.code().put (rownr, "");
    msfieldCol.time().put (rownr, itsStartTime);
    msfieldCol.numPoly().put (rownr, 1);
    msfieldCol.delayDirMeasCol().put (rownr, outdir);
    msfieldCol.phaseDirMeasCol().put (rownr, outdir);
    msfieldCol.referenceDirMeasCol().put (rownr, outdir);
    msfieldCol.sourceId().put (rownr, -1);
    msfieldCol.flagRow().put (rownr, False);
  }
  // Put the direction for each antenna into the POINTING subtable.
  {
    MSPointing mspointing = itsMS->pointing();
    MSPointingColumns mspointingCol(mspointing);
    uInt rownr = mspointing.nrow();
    mspointing.addRow(itsNrAnt);
    for (Int i=0; i<itsNrAnt; i++) {
      mspointingCol.antennaId().put (rownr, i);
      mspointingCol.time().put (rownr, itsStartTime);
      mspointingCol.interval().put (rownr, 0.);
      mspointingCol.name().put (rownr, "");
      mspointingCol.numPoly().put (rownr, 1);
      mspointingCol.timeOrigin().put (rownr, itsStartTime);
      mspointingCol.directionMeasCol().put (rownr, outdir);
      mspointingCol.targetMeasCol().put (rownr, outdir);
      mspointingCol.tracking().put (rownr, True);
      rownr++;
    }
  }
  itsNrField++;
  return itsNrField-1;
}

void MSWriterImpl::fillAntenna (const Block<MPosition>& antPos)
{
  // Determine constants for the ANTENNA subtable.
  Vector<Double> antOffset(3);
  antOffset = 0;
  // Fill the ANTENNA subtable.
  MSAntenna msant = itsMS->antenna();
  MSAntennaColumns msantCol(msant);
  msant.addRow (itsNrAnt);
  for (Int i=0; i<itsNrAnt; i++) {
    msantCol.name().put (i, "ST_" + String::toString(i));
    msantCol.station().put (i, "LOFAR");
    msantCol.type().put (i, "GROUND-BASED");
    msantCol.mount().put (i, "ALT-AZ");
    msantCol.positionMeas().put (i, antPos[i]);
    msantCol.offset().put (i, antOffset);
    msantCol.dishDiameter().put (i, 150);
    msantCol.flagRow().put (i, False);
  }
  msant.flush();
}

void MSWriterImpl::fillFeed()
{
  // Determine constants for the FEED subtable.
  Int nRec = 2;
  Matrix<Double> feedOffset(2,nRec);
  feedOffset = 0;
  Matrix<Complex> feedResponse(nRec,nRec);
  feedResponse = Complex(0.0,0.0);
  for (Int rec=0; rec<nRec; rec++) {
    feedResponse(rec,rec) = Complex(1.0,0.0);
  }
  Vector<String> feedType(nRec);
  feedType(0) = "X";
  feedType(1) = "Y";
  Vector<Double> feedPos(3);
  feedPos = 0.0;
  Vector<Double> feedAngle(nRec);
  feedAngle = -C::pi_4;                      // 0 for parallel dipoles
  // Fill the FEED subtable.
  MSFeed msfeed = itsMS->feed();
  MSFeedColumns msfeedCol(msfeed);
  msfeed.addRow (itsNrAnt);
  for (Int i=0; i<itsNrAnt; i++) {
    msfeedCol.antennaId().put (i, i);
    msfeedCol.feedId().put (i, 0);
    msfeedCol.spectralWindowId().put (i, -1);
    msfeedCol.time().put (i, itsStartTime + itsNrTimes*itsTimeStep/2.);
    msfeedCol.interval().put (i, itsNrTimes*itsTimeStep);
    msfeedCol.beamId().put (i, -1);
    msfeedCol.beamOffset().put (i, feedOffset);
    msfeedCol.polarizationType().put (i, feedType);
    msfeedCol.polResponse().put (i, feedResponse);
    msfeedCol.position().put (i, feedPos);
    msfeedCol.receptorAngle().put (i, feedAngle);
  }
  msfeed.flush();
}

void MSWriterImpl::fillObservation()
{
  MSObservation msobs = itsMS->observation();
  MSObservationColumns msobsCol(msobs);
  Vector<String> corrSchedule(1);
  corrSchedule = "corrSchedule";
  Vector<Double> timeRange(2);
  timeRange(0) = itsStartTime;
  timeRange(1) = itsStartTime + itsNrTimes*itsTimeStep;
  // Data is public one year after end of observation.
  Double releaseDate = timeRange(1) + 365.25*24*60*60;
  // Fill the columns
  msobs.addRow();
  msobsCol.telescopeName().put (0, "LOFAR");
  msobsCol.timeRange().put (0, timeRange);
  msobsCol.observer().put (0, "LofarSim");
  msobsCol.scheduleType().put (0, "LOFAR");
  msobsCol.schedule().put (0, corrSchedule);
  msobsCol.project().put (0, "LofarSim");
  msobsCol.releaseDate().put (0, releaseDate);
  msobsCol.flagRow().put (0, False);
  msobs.flush();
}

void MSWriterImpl::fillProcessor()
{
  MSProcessor msproc = itsMS->processor();
  MSProcessorColumns msprocCol(msproc);
  // Fill the columns
  msproc.addRow();
  msprocCol.type().put (0, "CORRELATOR");
  msprocCol.subType().put (0, "");
  msprocCol.typeId().put (0, -1);
  msprocCol.modeId().put (0, -1);
  msprocCol.flagRow().put (0, False);
  msproc.flush();
}

void MSWriterImpl::fillState()
{
  MSState msstate = itsMS->state();
  MSStateColumns msstateCol(msstate);
  // Fill the columns
  msstate.addRow();
  msstateCol.sig().put (0, True);
  msstateCol.ref().put (0, False);
  msstateCol.cal().put (0, 0.);
  msstateCol.load().put (0, 0.);
  msstateCol.subScan().put (0, 0);
  msstateCol.obsMode().put (0, "");
  msstateCol.flagRow().put (0, False);
  msstate.flush();
}

void MSWriterImpl::updateTimes()
{
  // Calculate the interval, end, and central time.
  Double interval = itsNrTimes*itsTimeStep;
  Double endTime = itsStartTime + interval;
  Double midTime = (itsStartTime + endTime) / 2;
  // Update all rows in FEED subtable.
  {
    MSFeed mssub = itsMS->feed();
    MSFeedColumns mssubCol(mssub);
    Vector<Double> val(mssub.nrow());
    val = midTime;
    mssubCol.time().putColumn (val);
    val = interval;
    mssubCol.interval().putColumn (val);
  }
  // Update all rows in POINTING subtable.
  {
    MSPointing mssub = itsMS->pointing();
    MSPointingColumns mssubCol(mssub);
    Vector<Double> val(mssub.nrow());
    val = midTime;
    mssubCol.time().putColumn (val);
    val = interval;
    mssubCol.interval().putColumn (val);
  }
  // Update all rows in OBSERVATION subtable.
  {
    MSObservation msobs = itsMS->observation();
    MSObservationColumns msobsCol(msobs);
    Vector<Double> timeRange(2);
    timeRange(0) = itsStartTime;
    timeRange(1) = itsStartTime + itsNrTimes*itsTimeStep;
    for (uInt i=0; i<msobs.nrow(); i++) {
      msobsCol.timeRange().put (i, timeRange);
    }
  }
}

void MSWriterImpl::write (int& rowNr, int bandId, int fieldId, int channelId, 
			  int timeCounter, int nrdata, const fcomplex* data, 
			  const bool* flags)
{
  ASSERT(bandId >= 0  &&  bandId < itsNrBand);
  ASSERT(fieldId >= 0  &&  fieldId < itsNrField);
  ASSERT(data != 0);
  if (timeCounter >= itsNrTimes) {
    itsNrTimes = timeCounter+1;
  }
  // Find the shape of the data array in each table row.
  IPosition shape(2, (*itsNrPol)[bandId], (*itsNrChan)[bandId]);
  Int nrel = shape[0];       // == number of polarisations/correlations
  ASSERTSTR (nrdata == nrel*itsNrAnt*(itsNrAnt+1)/2, 
	     "incorrect nr of data points for this band; should be " +  
	     String::toString(nrel*itsNrAnt*(itsNrAnt+1)/2));
  Array<Bool> defFlags(shape);

  // Add the number of rows needed.
  int nrbasel = itsNrAnt*(itsNrAnt+1)/2;
  rowNr = itsMS->nrow();
  itsMS->addRow (nrbasel);
  defFlags = False;
  Array<Float> sigma(IPosition(1, shape(0)));
  sigma = 0;
  Array<Float> weight(IPosition(1, shape(0)));
  weight = 1;
  Double time = itsStartTime + timeCounter*itsTimeStep;
  // Calculate the apparent HA and DEC for the array center.
  // First store time in frame.
  Quantity qtime(time, "s");
  itsFrame->set (MEpoch(qtime, MEpoch::UTC));
  MDirection::Ref outref(MDirection::HADEC, *itsFrame);
  MDirection outdir = MDirection::Convert (*itsArrayPos, outref) ();
  Double ha = outdir.getAngle().getValue()(0);
  Double dec = outdir.getAngle().getValue()(1);
  Double sinha = std::sin(ha);
  Double cosha = std::cos(ha);
  Double sindec = std::sin(dec);
  Double cosdec = std::cos(dec);
  Vector<Double> uvw(3);
  const Cube<Double>& basel = *itsBaselines;
  // Write all the data.
  // The input data array has shape nrpol,nrant,nrant.
  // So we can form an AIPS++ array for each baseline.
  for (int i=0; i<itsNrAnt; i++) {
    for (int j=0; j<=i; j++) {
      uvw(0) = sinha*basel(0,i,j) + cosha*basel(1,i,j);
      uvw(1) = -sindec*cosha*basel(0,i,j) + sindec*sinha*basel(1,i,j) +
	cosdec*basel(2,i,j);
      uvw(2) = cosdec*cosha*basel(0,i,j) - cosdec*sinha*basel(1,i,j) +
	sindec*basel(2,i,j);

      Array<Complex> array(shape, (Complex*)data, SHARE);
      try
      {
	itsMSCol->data().put(rowNr, array);
      }
      catch (AipsError& e)
      {
	cout << "AipsError in put: " <<  e.what() << endl;
      }
      if (flags == 0) {
	itsMSCol->flag().put(rowNr, defFlags);
      } else {
	Array<Bool> flagArray(shape, const_cast<Bool*>(flags), SHARE);
	itsMSCol->flag().put (rowNr, flagArray);
      }

      itsMSCol->flagRow().put (rowNr, False);
      itsMSCol->time().put (rowNr, time);
      itsMSCol->antenna1().put (rowNr, i);
      itsMSCol->antenna2().put (rowNr, j);
      itsMSCol->feed1().put (rowNr, 0);
      itsMSCol->feed2().put (rowNr, 0);
      itsMSCol->dataDescId().put (rowNr, bandId);
      itsMSCol->processorId().put (rowNr, 0);
      itsMSCol->fieldId().put (rowNr, fieldId);
      itsMSCol->interval().put (rowNr, itsTimeStep);
      itsMSCol->exposure().put (rowNr, itsTimeStep);
      itsMSCol->timeCentroid().put (rowNr, time);
      itsMSCol->scanNumber().put (rowNr, 0);
      itsMSCol->arrayId().put (rowNr, 0);
      itsMSCol->observationId().put (rowNr, 0);
      itsMSCol->stateId().put (rowNr, 0);
      itsMSCol->uvw().put (rowNr, uvw);
      itsMSCol->weight().put (rowNr, weight);
      itsMSCol->sigma().put (rowNr, sigma);
      rowNr++;
      data += nrel;
      if (flags != 0) {
	flags += nrel;
      }
    }
  }
}



