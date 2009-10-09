//#  MeasurementSetFormat.cc: Creates required infrastructure for 
//#  a LofarStMan MeasurementSet.
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id: $


#include <lofar_config.h>

#include <Storage/MeasurementSetFormat.h>

#include <AMCBase/Epoch.h>

#include <string>
#include <fstream>
#include <iostream>

#include <linux/limits.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
//#include <tables/Tables/StandardStMan.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Containers/BlockIO.h>
#include <casa/OS/RegularFile.h>
#include <casa/Utilities/Assert.h>
#include <casa/IO/RegularFileIO.h>
#include <casa/IO/RawIO.h>
#include <casa/IO/CanonicalIO.h>
#include <casa/OS/HostInfo.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <casa/BasicSL/Constants.h>
#include <ms/MeasurementSets.h>

#include <LofarStMan/LofarStMan.h>


using namespace casa;

namespace LOFAR { 
namespace RTCP {

MeasurementSetFormat::MeasurementSetFormat(const Parset *ps, const unsigned alignment)
  : itsPS(ps),
    itsMS(0), 
    itsAlignment(alignment)
{
 
  if (itsPS->nrTabStations() > 0) {
    itsNrAnt = itsPS->nrTabStations();
    stationNames = itsPS->getStringVector("OLAP.tiedArrayStationNames",true);
  } else {
    itsNrAnt = itsPS->nrStations();
    stationNames = itsPS->getStringVector("OLAP.storageStationNames",true);
  }

  antPos = itsPS->positions();

  AMC::Epoch epoch;
  epoch.utc(itsPS->startTime());
  itsStartTime = MVEpoch(epoch.mjd()).getTime().getValue("s");
  itsTimeStep = itsPS->IONintegrationTime();
  itsNrTimes = 29030400;  /// equates to about one year, sets valid
			  /// timerage to 1 year beyond starttime
}
  
MeasurementSetFormat::~MeasurementSetFormat()  
{}

void MeasurementSetFormat::addSubband(unsigned subband)
{
  /// First create a valid MeasurementSet with all required
  /// tables. Note that the MS is destroyed immidiately.
  createMSTables(subband);
  /// Next make a metafile which describes the raw datafile we're
  /// going to write
  createMSMetaFile(subband);
}

void MeasurementSetFormat::createMSTables(unsigned subband)
{
  try {

    TableDesc td = MS::requiredTableDesc();
    MS::addColumnToDesc(td, MS::DATA, 2);
    MS::addColumnToDesc(td, MS::WEIGHT_SPECTRUM, 2);

    string MSname =  itsPS->getMSname(subband);

    SetupNewTable newtab(MSname, td, Table::New);
    LofarStMan lofarstman;
    newtab.bindAll(lofarstman);

    itsMS = new MeasurementSet(newtab);
    itsMS->createDefaultSubtables (Table::New);

    Block<MPosition> antMPos(itsNrAnt);
    try {
      for (unsigned i=0; i<itsNrAnt; i++) {
      
	antMPos[i] = MPosition(MVPosition(antPos[3*i], 
					  antPos[3*i+1], 
					  antPos[3*i+2]),
			       MPosition::ITRF);
      }
    } catch (AipsError& e) {
      LOG_FATAL_STR("AipsError: " << e.what());
    }

    fillAntenna(antMPos);
    fillFeed();
    fillField(subband);
    fillPola();
    fillDataDesc();
    fillSpecWindow(subband);
    fillObs();

  } catch (AipsError x) {
    LOG_FATAL_STR("AIPS/CASA error: " << x.getMesg());
    exit(1);
  } catch (...) {
    LOG_FATAL_STR("NON AIPS/CASA error");
    exit(1);
  }

  // Flush the MS to make sure all tables are written
  itsMS->flush();
  // Delete the MS, since we don't need it anymore
  delete itsMS;
}


void MeasurementSetFormat::fillAntenna (const Block<MPosition>& antMPos)
{
  // Determine constants for the ANTENNA subtable.
  casa::Vector<Double> antOffset(3);
  antOffset = 0;
  // Fill the ANTENNA subtable.
  MSAntenna msant = itsMS->antenna();
  MSAntennaColumns msantCol(msant);
  msant.addRow (itsNrAnt);
      
  for (unsigned i=0; i<itsNrAnt; i++) {
    msantCol.name().put (i, stationNames[i]);
    msantCol.station().put (i, "LOFAR");
    msantCol.type().put (i, "GROUND-BASED");
    msantCol.mount().put (i, "ALT-AZ");
    msantCol.positionMeas().put (i, antMPos[i]);
    msantCol.offset().put (i, antOffset);
    msantCol.dishDiameter().put (i, 150);
    msantCol.flagRow().put (i, False);
  }
  msant.flush();
}

void MeasurementSetFormat::fillFeed()
{
  // Determine constants for the FEED subtable.
  unsigned nRec = 2;
  casa::Matrix<Double> feedOffset(2,nRec);
  feedOffset = 0;
  casa::Matrix<Complex> feedResponse(nRec,nRec);
  feedResponse = Complex(0.0,0.0);
  for (unsigned rec=0; rec<nRec; rec++) {
    feedResponse(rec,rec) = Complex(1.0,0.0);
  }
  casa::Vector<String> feedType(nRec);
  feedType(0) = "X";
  feedType(1) = "Y";
  casa::Vector<Double> feedPos(3);
  feedPos = 0.0;
  casa::Vector<Double> feedAngle(nRec);
  feedAngle = -C::pi_4;                      // 0 for parallel dipoles

  // Fill the FEED subtable.
  MSFeed msfeed = itsMS->feed();
  MSFeedColumns msfeedCol(msfeed);
  msfeed.addRow (itsNrAnt);
  for (unsigned i=0; i<itsNrAnt; i++) {
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
    msfeedCol.numReceptors().put (i, 2);
  }
  msfeed.flush();
}


void MeasurementSetFormat::fillField(unsigned subband) {
  const vector<unsigned> subbandToBeamMapping = itsPS->subbandToBeamMapping();

  MVDirection radec (Quantity(itsPS->getBeamDirection(subbandToBeamMapping[subband])[0], "rad"), 
		     Quantity(itsPS->getBeamDirection(subbandToBeamMapping[subband])[1], "rad"));
  MDirection indir(radec, MDirection::J2000);
  casa::Vector<MDirection> outdir(1);
  outdir(0) = indir;
  // Put the direction into the FIELD subtable.
  {
    MSField msfield = itsMS->field();
    MSFieldColumns msfieldCol(msfield);
    uInt rownr = msfield.nrow();
    msfield.addRow();
    msfieldCol.name().put (rownr, "BEAM_" + String::toString(rownr));
    msfieldCol.code().put (rownr, "");
    msfieldCol.time().put (rownr, itsStartTime);
    msfieldCol.numPoly().put (rownr, 0);
    msfieldCol.delayDirMeasCol().put (rownr, outdir);
    msfieldCol.phaseDirMeasCol().put (rownr, outdir);
    msfieldCol.referenceDirMeasCol().put (rownr, outdir);
    msfieldCol.sourceId().put (rownr, -1);
    msfieldCol.flagRow().put (rownr, False);
  }
}

void MeasurementSetFormat::fillPola() {
  const unsigned npolarizations = itsPS->nrCrossPolarisations();

  MSPolarization mspol = itsMS->polarization();
  MSPolarizationColumns mspolCol(mspol);
  uInt rownr = mspol.nrow();
  casa::Vector<Int> corrType(npolarizations);
  corrType(0) = Stokes::XX;
  if (npolarizations == 2) {
    corrType(1) = Stokes::YY;
  } else if (npolarizations == 4) {
    corrType(1) = Stokes::XY;
    corrType(2) = Stokes::YX;
    corrType(3) = Stokes::YY;
  }
  casa::Matrix<Int> corrProduct(2, npolarizations);
  for (unsigned i=0; i<npolarizations; i++) {
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
}

void MeasurementSetFormat::fillDataDesc() {
  MSDataDescription msdd = itsMS->dataDescription();
  MSDataDescColumns msddCol(msdd);
  
  msdd.addRow();

  msddCol.spectralWindowId().put(0, 0);
  msddCol.polarizationId().put(0, itsPS->nrCrossPolarisations());
  msddCol.flagRow().put(0, False);

  msdd.flush();
}

void MeasurementSetFormat::fillObs() {
  casa::Vector<Double> timeRange(2);
  timeRange(0) = itsStartTime;
  timeRange(1) = itsStartTime + itsNrTimes*itsTimeStep;

  casa::Vector<String> corrSchedule(1);
  corrSchedule = "corrSchedule";

  double releaseDate = timeRange(1) + 365.25*24*60*60;

  MSObservation msobs = itsMS->observation();
  MSObservationColumns msobsCol(msobs);

  msobs.addRow();

  msobsCol.telescopeName().put(0, "LOFAR");
  msobsCol.timeRange().put(0, timeRange);
  msobsCol.observer().put (0, itsPS->observerName());
  msobsCol.scheduleType().put (0, "LOFAR");
  msobsCol.schedule().put (0, corrSchedule);
  msobsCol.project().put (0, itsPS->projectName());
  msobsCol.releaseDate().put (0, releaseDate);
  msobsCol.flagRow().put(0, False);

  msobs.flush();
}

void MeasurementSetFormat::fillSpecWindow(unsigned subband) {
  const double refFreq = itsPS->subbandToFrequencyMapping()[subband];
  const double chanWidth = itsPS->channelWidth();
  casa::Vector<double> chanWidths(itsPS->nrChannelsPerSubband());
  chanWidths = chanWidth;
  casa::Vector<double> chanFreqs(itsPS->nrChannelsPerSubband());
  indgen (chanFreqs, refFreq - (itsPS->nrChannelsPerSubband()-1)*chanWidth/2., chanWidth);

  casa::Vector<double> stFreqs = chanFreqs - chanWidth/2.;
  casa::Vector<double> endFreqs = chanFreqs + chanWidth/2.;
  double totalBW = max(endFreqs) - min(stFreqs);

  MSSpectralWindow msspw = itsMS->spectralWindow();
  MSSpWindowColumns msspwCol(msspw);
    
  msspw.addRow();

  msspwCol.numChan().put (0, itsPS->nrChannelsPerSubband());
  msspwCol.name().put (0, "SB-" + String::toString(subband));
  msspwCol.refFrequency().put (0, refFreq);
  msspwCol.chanFreq().put (0, chanFreqs);

  msspwCol.chanWidth().put (0, chanWidths);
  msspwCol.measFreqRef().put (0, MFrequency::TOPO);
  msspwCol.effectiveBW().put (0, chanWidths);
  msspwCol.resolution().put (0, chanWidths);
  msspwCol.totalBandwidth().put (0, totalBW);
  msspwCol.netSideband().put (0, 0);
  msspwCol.ifConvChain().put (0, 0);
  msspwCol.freqGroup().put (0, 0);
  msspwCol.freqGroupName().put (0, "");
  msspwCol.flagRow().put (0, False);

  msspw.flush();
}

void MeasurementSetFormat::createMSMetaFile(unsigned subband)
{ 
  Block<Int> ant1(itsPS->nrBaselines());
  Block<Int> ant2(itsPS->nrBaselines());
  uInt inx=0;
  uInt nStations = 0;

  if (itsPS->nrTabStations() > 0) nStations = itsPS->nrTabStations();
  else nStations = itsPS->nrStations();

  for (uInt i = 0; i < nStations; ++i) {
    for (uInt j=0; j<=i; ++j) {
      ant1[inx] = j;
      ant2[inx] = i;
      ++inx;
    }
  }

  // data is generated on a BigEndian machine and written as is to
  // disk, regardless of endianness of writing machine.
  bool isBigEndian = true;
 
  string filename = itsPS->getMSname(subband) + "/table.f0meta";
  
  AipsIO aio(filename, ByteIO::New);
  aio.putstart ("LofarStMan", FORMATVERSION);     // version 1
  aio << ant1 << ant2
      << itsStartTime
      << itsPS->IONintegrationTime()
      << itsPS->nrChannelsPerSubband()
      << itsPS->nrCrossPolarisations()
      << itsPS->sampleRate()
      << itsAlignment
      << isBigEndian;
  aio.close();
}
 

} // namespace RTCP
} // namepsace LOFAR
