//# MSWriterCasa.cc: implementation for filling a MeasurementSet
//#
//#  Copyright (C) 2001
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
//#  $Id: MSWriterCasa.cc 11891 2008-10-14 13:43:51Z gels $

// \file
// Casaementation for filling a MeasurementSet

#include <lofar_config.h>

#if defined HAVE_AIPSPP

#include <AMCBase/Epoch.h>
#include <Common/LofarLogger.h>
#include <Common/Timer.h>

#include <Storage/MSWriterCasa.h>
#include <Interface/CorrelatedData.h>

#include <ms/MeasurementSets.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <tables/Tables/TiledStManAccessor.h> 
//# include <tables/Tables/TiledShapeStMan.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/Muvw.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVEpoch.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/MVBaseline.h>
#include <casa/OS/Time.h>
#include <casa/OS/SymLink.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Slicer.h>

#if defined HAVE_MPI
#include <mpi.h>
#endif




namespace LOFAR
{
  namespace RTCP
  {

    Mutex MSWriterCasa::sharedMutex;

    static NSTimer MSsetupTimer("MSWriterCasa::write (setup)", true, true);
    static NSTimer MSwriteTimer("MSWriterCasa::write (write)", true, true);


    using namespace casa;

    MSWriterCasa::MSWriterCasa (const char* msName, double startTime, double timeStep,
                                int nfreq, int ncorr, int nantennas, const vector<double>& antPos,
				const vector<string>& storageStationNames, float weightFactor)
      : itsNrBand   (0),
        itsNrField  (0),
        itsNrAnt    (nantennas),
        itsNrFreq   (nfreq),
        itsNrCorr   (ncorr),
	itsNrTimes  (0),
	itsWeightFactor(weightFactor),
	itsNVisibilities(0),
        itsTimeStep (timeStep),
	itsStartTime(0),
	itsField    (),
        itsNrPol    (0),
        itsNrChan   (0),
        itsPolnr    (0),
        itsBaselines(0),
        itsFrame    (0),
        itsMS       (0),
        itsMSCol    (0)	
    {
      sharedMutex.lock();
      AlwaysAssert (nantennas >= 0, AipsError);

      // Allocate buffers
      int nrbasel = itsNrAnt*(itsNrAnt+1)/2;
      itsNVisibilities = nrbasel * itsNrFreq * itsNrCorr;
      itsFlagsBuffers   = new bool[itsNVisibilities];
      itsWeightsBuffers = new float[nrbasel * itsNrFreq];

      // Convert the startTime from seconds since 1 Jan 1970 (UTC) to Modified Julian Day.
      AMC::Epoch epoch;
      epoch.utc(startTime);
      itsStartTime = MVEpoch(epoch.mjd()).getTime().getValue("s");
      
      // Get the position of Westerbork.
      MPosition arrayPosTmp;
      AlwaysAssert (MeasTable::Observatory(arrayPosTmp, "WSRT"), AipsError);
      itsArrayPos = new MPosition;
      MPosition *obsLofar   = new MPosition;

      //Get list of all observatories in LOFAR network. 
//    LOG_DEBUG_STR("List of all observatories in LOFAR network:" << MeasTable::Observatories());

      //Todo: get the Observatory for LOFAR!!
      AlwaysAssert (MeasTable::Observatory(*itsArrayPos, "WSRT"), AipsError);
//       LOG_DEBUG_STR("Position observatory WSRT: " << *itsArrayPos);
      AlwaysAssert (MeasTable::Observatory(*obsLofar, "LOFAR"), AipsError);
//       LOG_DEBUG_STR("Position observatory LOFAR: " << *obsLofar);

      delete obsLofar;
      
      Block<MPosition> antMPos(nantennas);

      // Keep the antenna positions in ITRF coordinates as X,Y,Z in (m,m,m).
      try {
        for (int i=0; i<nantennas; i++) {

          antMPos[i] = MPosition(MVPosition(antPos[3*i], 
	      	                            antPos[3*i+1], 
			                    antPos[3*i+2]),
	                                    MPosition::ITRF);
        }
      } catch (AipsError& e) {
	LOG_FATAL_STR("AipsError: " << e.what());
      }

      // Create the MS.
      try {
	createMS (msName, antMPos, storageStationNames);
      } catch (AipsError x) {
	LOG_FATAL_STR("AIPS/Casa error in createMS(): " << x.getMesg());
	exit(0);
      } catch (...) {
	LOG_FATAL("Unknown error in createMS()");
      }

      itsNrPol  = new Block<Int>;
      itsNrChan = new Block<Int>;
      itsPolnr  = new Block<Int>;
      // Find the baselines in X,Y,Z.
      itsBaselines = new casa::Cube<double> (3, nantennas, nantennas);
      casa::Cube<double>& basel = *itsBaselines;
      for (int i=0; i<nantennas; i++) {
        const casa::Vector<Double>& pos1 = antMPos[i].getValue().getValue();
        for (int j=0; j<nantennas; j++) {
	  // Generate a 3-vector of x,y,z in m
          const casa::Vector<Double>& pos2 = antMPos[j].getValue().getValue();
          basel(0,i,j) = pos2(0) - pos1(0);
          basel(1,i,j) = pos2(1) - pos1(1);
          basel(2,i,j) = pos2(2) - pos1(2);
        }
      }
      // Make a frame for the calculation of apparent coordinates.
      // Store position of array center in it.
      itsFrame = new MeasFrame(*itsArrayPos);
      sharedMutex.unlock();
    }

    MSWriterCasa::~MSWriterCasa()
    {
      sharedMutex.lock();
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
      
      delete [] itsFlagsBuffers;   itsFlagsBuffers   = 0;
      delete [] itsWeightsBuffers; itsWeightsBuffers = 0;
      sharedMutex.unlock();
    }

    int MSWriterCasa::nrPolarizations() const
    {
      return itsMS->polarization().nrow();
    }


    void MSWriterCasa::createMS (const char* msName,
                                 const Block<MPosition>& antPos,
				 const vector<string>& storageStationNames)
    {
      // Get the MS main default table description..
      TableDesc td = MS::requiredTableDesc();
      // Add the data column and its unit.
      MS::addColumnToDesc(td, MS::DATA, 2);
      td.rwColumnDesc(MS::columnName(MS::DATA)).rwKeywordSet().
        define("UNIT","Jy");
      MS::addColumnToDesc(td, MS::WEIGHT_SPECTRUM, 1);
	
      //   // Store the data and flags using the TiledStMan.
      //   Vector<String> tsmNames(2);
      //   tsmNames(0) = MS::columnName(MS::DATA);
      //   tsmNames(1) = MS::columnName(MS::FLAG);
      //   td.defineHypercolumn("TiledData", 3, tsmNames);

      // Store the data and flags in two separate files using TiledColumnStMan.
      // Also store UVW with TiledColumnStMan.
      casa::Vector<String> tsmNames(1);
      tsmNames[0] = MS::columnName(MS::DATA);
      td.rwColumnDesc(tsmNames[0]).setShape (IPosition(2,itsNrCorr,itsNrFreq));
      td.defineHypercolumn("TiledData", 3, tsmNames);
      tsmNames[0] = MS::columnName(MS::FLAG);
      td.rwColumnDesc(tsmNames[0]).setShape (IPosition(2,itsNrCorr,itsNrFreq));
      td.defineHypercolumn("TiledFlag", 3, tsmNames);
      tsmNames[0] = MS::columnName(MS::UVW);
      td.defineHypercolumn("TiledUVW", 2, tsmNames);


      // Setup the new table.
      // Most columns use the IncrStMan; some use others.
      SetupNewTable newTab(msName, td, Table::New);
      IncrementalStMan incrStMan("ISMData");
      StandardStMan    stanStMan("SSMData",32768);


      //   TiledShapeStMan  tiledStMan("TiledData", IPosition(3,4,16,512));
      //   newTab.bindAll (incrStMan);
      //   newTab.bindColumn(MS::columnName(MS::ANTENNA1),stanStMan);
      //   newTab.bindColumn(MS::columnName(MS::ANTENNA2),stanStMan);
      //   newTab.bindColumn(MS::columnName(MS::DATA),tiledStMan);
      //   newTab.bindColumn(MS::columnName(MS::FLAG),tiledStMan);
      //   newTab.bindColumn(MS::columnName(MS::UVW),stanStMan);

      // Use a TiledColumnStMan for the data, flags and UVW.
      // Store all pol and freq in a single tile.
      // In this way the data appear in separate files that can be mmapped.
      // Flags are stored as bits, so take care each tile has multiple of 8 flags.
      TiledColumnStMan tiledData("TiledData", IPosition(3,itsNrCorr,itsNrFreq,(itsNrAnt+1)/2));
      TiledColumnStMan tiledFlag("TiledFlag", IPosition(3,itsNrCorr,itsNrFreq,(itsNrAnt+1)/2*8));
      TiledColumnStMan tiledUVW("TiledUVW", IPosition(2,3,(itsNrAnt+1)/2*itsNrAnt));
      newTab.bindAll (incrStMan);
      newTab.bindColumn(MS::columnName(MS::ANTENNA1),stanStMan);
      newTab.bindColumn(MS::columnName(MS::ANTENNA2),stanStMan);
      newTab.bindColumn(MS::columnName(MS::DATA),tiledData);
      newTab.bindColumn(MS::columnName(MS::FLAG),tiledFlag);
      newTab.bindColumn(MS::columnName(MS::UVW),tiledUVW);


      // Create the MS and its subtables.
      // Get access to its columns.
      itsMS = new MeasurementSet(newTab);
      itsMSCol = new MSMainColumns(*itsMS);
      itsMSCol->uvwMeas().setDescRefCode (Muvw::J2000);
      
      // Create all subtables.
      // Do this after the creation of optional subtables,
      // so the MS will know about those optional sutables.
      itsMS->createDefaultSubtables (Table::New);
      // Fill various subtables.
      fillAntenna (antPos, storageStationNames);
      fillFeed();
      fillProcessor();
      fillObservation();
      fillState();
            
      // Add weight_spectrum description

    }

    int MSWriterCasa::addBand (int npolarizations, int nchannels,
                               double refFreq, double chanWidth)
    {
      sharedMutex.lock();
      AlwaysAssert (nchannels > 0, AipsError);
      casa::Vector<double> chanWidths(nchannels);
      chanWidths = chanWidth;
      casa::Vector<double> chanFreqs(nchannels);
      indgen (chanFreqs, refFreq - (nchannels-1)*chanWidth/2., chanWidth);
      int retval;
      try {
	retval = addBand (npolarizations, nchannels, refFreq, chanFreqs, chanWidths);
      } catch (AipsError x) {
	LOG_FATAL_STR("AIPS/Casa error in MSWriterCasa::addBand(): " << x.getMesg());
	exit(0);
      } catch (...) {
	LOG_FATAL("Unexpected error in MSWriterCasa::addBand() ");
	exit(0);
      }
      sharedMutex.unlock();
      return retval;
    }

    int MSWriterCasa::addBand (int npolarizations, int nchannels,
                               double refFreq, const double* chanFreqs,
                               const double* chanWidths)
    {
      sharedMutex.lock();
      AlwaysAssert (nchannels > 0, AipsError);
      IPosition shape(1, nchannels);
      casa::Vector<double> freqs (shape, const_cast<double*>(chanFreqs), SHARE);
      casa::Vector<double> widths(shape, const_cast<double*>(chanWidths), SHARE);
      int retval;
      try {
	retval = addBand (npolarizations, nchannels, refFreq, freqs, widths);
      } catch (AipsError x) {
	LOG_FATAL_STR("AIPS/Casa error in MSWriterCasa::addBand(): " << x.getMesg());
	exit(0);
      } catch (...) {
        LOG_FATAL("Unexpected error in MSWriterCasa::addBand() ");
	exit(0);
      }
      sharedMutex.unlock();
      return retval;
    }

    int MSWriterCasa::addBand (int npolarizations, int nchannels,
                               double refFreq, const casa::Vector<double>& chanFreqs,
                               const casa::Vector<double>& chanWidths)
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
      casa::Vector<double> stFreqs = chanFreqs - chanWidths/2.;
      casa::Vector<double> endFreqs = chanFreqs + chanWidths/2.;
      double totalBW = max(endFreqs) - min(stFreqs);
      MSSpectralWindow msspw = itsMS->spectralWindow();
      MSSpWindowColumns msspwCol(msspw);
      msspw.addRow();
      msspwCol.numChan().put (rownr, nchannels);
#if defined HAVE_MPI
      int rank;
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      int nrSubband = rank + rownr; // FIXME: is this correct???
      msspwCol.name().put (rownr, "SB-" + String::toString(nrSubband));
#else
      msspwCol.name().put (rownr, "SB-0");     
#endif      
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

    int MSWriterCasa::addPolarization (int npolarizations)
    {
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

    void MSWriterCasa::addField (double RA, double DEC, unsigned beamIndex)
    {
      sharedMutex.lock();
      MVDirection radec (Quantity(RA,"rad"), Quantity(DEC,"rad"));
      MDirection indir(radec, MDirection::J2000);
      casa::Vector<MDirection> outdir(1);
      outdir(0) = indir;
      itsField = indir;
      // Put the direction into the FIELD subtable.
      {
        MSField msfield = itsMS->field();
        MSFieldColumns msfieldCol(msfield);
        uInt rownr = msfield.nrow();
        msfield.addRow();
        msfieldCol.name().put (rownr, "BEAM_" + String::toString(beamIndex));
        msfieldCol.code().put (rownr, "");
        msfieldCol.time().put (rownr, itsStartTime);
        msfieldCol.numPoly().put (rownr, 0);
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
          mspointingCol.numPoly().put (rownr, 0);
          mspointingCol.timeOrigin().put (rownr, itsStartTime);
          mspointingCol.directionMeasCol().put (rownr, outdir);
          mspointingCol.targetMeasCol().put (rownr, outdir);
          mspointingCol.tracking().put (rownr, False);
          rownr++;
        }
      }
      itsNrField++;
      sharedMutex.unlock();
    }

    void MSWriterCasa::fillAntenna (const Block<MPosition>& antPos,
                                    const vector<string>& storageStationNames)
    {
      // Determine constants for the ANTENNA subtable.
      casa::Vector<Double> antOffset(3);
      antOffset = 0;
      // Fill the ANTENNA subtable.
      MSAntenna msant = itsMS->antenna();
      MSAntennaColumns msantCol(msant);
      msant.addRow (itsNrAnt);
      
      for (Int i=0; i<itsNrAnt; i++) {
        msantCol.name().put (i, storageStationNames[i]);
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

    void MSWriterCasa::fillFeed()
    {
      // Determine constants for the FEED subtable.
      Int nRec = 2;
      casa::Matrix<Double> feedOffset(2,nRec);
      feedOffset = 0;
      casa::Matrix<Complex> feedResponse(nRec,nRec);
      feedResponse = Complex(0.0,0.0);
      for (Int rec=0; rec<nRec; rec++) {
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
        msfeedCol.numReceptors().put (i, 2);
      }
      msfeed.flush();
    }

    void MSWriterCasa::fillObservation()
    {
      MSObservation msobs = itsMS->observation();
      MSObservationColumns msobsCol(msobs);
      casa::Vector<String> corrSchedule(1);
      corrSchedule = "corrSchedule";
      casa::Vector<Double> timeRange(2);
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

    void MSWriterCasa::fillProcessor()
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

    void MSWriterCasa::fillState()
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

    void MSWriterCasa::updateTimes()
    {
      // Calculate the interval, end, and central time.
      Double interval = itsNrTimes*itsTimeStep;
      Double endTime = itsStartTime + interval;
      Double midTime = (itsStartTime + endTime) / 2;

      // Update all rows in FEED subtable.
      {
        MSFeed mssub = itsMS->feed();
        MSFeedColumns mssubCol(mssub);
        casa::Vector<Double> val(mssub.nrow());
        val = midTime;
        mssubCol.time().putColumn (val);
        val = interval;
        mssubCol.interval().putColumn (val);
      }
      // Update all rows in POINTING subtable.
      {
        MSPointing mssub = itsMS->pointing();
        MSPointingColumns mssubCol(mssub);
        casa::Vector<Double> val(mssub.nrow());
        val = midTime;
        mssubCol.time().putColumn (val);
        val = interval;
        mssubCol.interval().putColumn (val);
      }
      // Update all rows in OBSERVATION subtable.
      {
        MSObservation msobs = itsMS->observation();
        MSObservationColumns msobsCol(msobs);
        casa::Vector<Double> timeRange(2);
        timeRange(0) = itsStartTime;
	timeRange(1) = itsStartTime + (itsNrTimes*itsTimeStep);
        for (uInt i=0; i<msobs.nrow(); i++) {
          msobsCol.timeRange().put (i, timeRange);
        }
      }
    }

    void MSWriterCasa::write (int bandId, int channelId, int nrChannels, 
                              StreamableData *data)
    {
      sharedMutex.lock();
      CorrelatedData *correlatedData = dynamic_cast<CorrelatedData*>(data);
      const fcomplex* visibilityData = correlatedData->visibilities.origin();
  
      ASSERT(bandId >= 0  &&  bandId < itsNrBand);
      ASSERT(visibilityData != 0);

      //LOG_DEBUG("write");

      MSsetupTimer.start();

      try {
        int nrbasel = itsNrAnt*(itsNrAnt+1)/2;
      
        // Write one set of visibilities of size
        // fcomplex[nrbasel][nrChannels][npol][npol]
        unsigned short *valSamples = correlatedData->nrValidSamples.origin();
      
        for (int i = 0; i < nrbasel * nrChannels; i ++) {
          itsWeightsBuffers[i] = itsWeightFactor * valSamples[i];
          bool flagged = valSamples[i] == 0;
          itsFlagsBuffers[4 * i    ] = flagged;
          itsFlagsBuffers[4 * i + 1] = flagged;
          itsFlagsBuffers[4 * i + 2] = flagged;
          itsFlagsBuffers[4 * i + 3] = flagged;
        }
        
	const float* weights = itsWeightsBuffers;
	const bool* flags = itsFlagsBuffers;
      
	if ((int)correlatedData->sequenceNumber >= itsNrTimes)
	  itsNrTimes = correlatedData->sequenceNumber + 1;
	
	Double time = itsStartTime + (itsNrTimes - .5) * itsTimeStep;
	
	// Find the shape of the data array in each table row.
	IPosition shape(2, (*itsNrPol)[bandId], (*itsNrChan)[bandId]);
	Int nrel = shape[0];       // == number of polarisations/correlations
	ASSERTSTR ((int)itsNVisibilities == nrel*nrChannels*itsNrAnt*(itsNrAnt+1)/2, 
		   "incorrect nr of data points for this band; should be " +  
		   String::toString(nrel*nrChannels*itsNrAnt*(itsNrAnt+1)/2));
	Array<Bool> defFlags(shape);
			
	// Add the number of rows needed.
	int rowNumber = itsMS->nrow();
	itsMS->addRow (nrbasel);
	
	// If first time, set the cache size for the tiled data and flags.
	ROTiledStManAccessor accData(*itsMS, "TiledData");
	accData.setCacheSize (0, itsNrAnt*(itsNrAnt+1)/2);
	////    accData.setCacheSize (0, 1);
	ROTiledStManAccessor accFlag(*itsMS, "TiledFlag");
	accFlag.setCacheSize (0, itsNrAnt*(itsNrAnt+1)/2);
	////    accFlag.setCacheSize (0, 1);
	
	defFlags = False;
	Array<Float> sigma(IPosition(1, shape(0)));
	sigma = 1;
	Array<Float> weight(IPosition(1, shape(0)));
	
	// Calculate the apparent HA and DEC for the array center.
	// First store time in frame.
	Quantity qtime(time, "s");
	itsFrame->set (MEpoch(qtime, MEpoch::UTC));
	
	itsFrame->set (itsField);
	MVDirection MVd = itsField.getValue();
	casa::Vector<Double> uvw(3);
	const casa::Cube<Double>& basel = *itsBaselines;
	
	IPosition dShape(2, shape[0], nrChannels); // Shape of data field
	IPosition wShape(1, nrChannels);           // Shape of weight_spectrum field
	
	MBaseline  MB_ITRF;
	MBaseline  MB_J2000;
	MVBaseline MBV_J2000;
	
	MSsetupTimer.stop();
	MSwriteTimer.start();
	
	for (int ant2=0; ant2<itsNrAnt; ant2++) {
	  for (int ant1=0; ant1<=ant2; ant1++) {
	    MB_ITRF = MBaseline(MVBaseline(basel(0,ant1,ant2), basel(1,ant1,ant2), basel(2,ant1,ant2)), 
	                      MBaseline::ITRF);
	    MB_J2000 = MBaseline::Convert (MB_ITRF, MBaseline::Ref (MBaseline::J2000, *itsFrame)) ();
	    MBV_J2000 = MB_J2000.getValue();
	    uvw = MVuvw(MBV_J2000, MVd).getValue();
	    
	    weight = ((Float*)weights)[1];
	  
	    itsMSCol->data().setShape(rowNumber, shape);

	    itsMSCol->flagRow().put (rowNumber, False);
	    itsMSCol->time().put (rowNumber, time);
	    itsMSCol->antenna1().put (rowNumber, ant1);
	    itsMSCol->antenna2().put (rowNumber, ant2);
	    itsMSCol->feed1().put (rowNumber, 0);
	    itsMSCol->feed2().put (rowNumber, 0);
	    itsMSCol->dataDescId().put (rowNumber, bandId);
	    itsMSCol->processorId().put (rowNumber, 0);
	    itsMSCol->fieldId().put (rowNumber, 0);
	    itsMSCol->interval().put (rowNumber, itsTimeStep);
	    itsMSCol->exposure().put (rowNumber, itsTimeStep);
	    itsMSCol->timeCentroid().put (rowNumber, time);
	    itsMSCol->scanNumber().put (rowNumber, 0);
	    itsMSCol->arrayId().put (rowNumber, 0);
	    itsMSCol->observationId().put (rowNumber, 0);
	    itsMSCol->stateId().put (rowNumber, 0);
	    itsMSCol->uvw().put (rowNumber, uvw);
	    itsMSCol->weight().put (rowNumber, weight);
	    itsMSCol->sigma().put (rowNumber, sigma);

          try
	    {
	      // Write all polarisations and nrChannels for each baseline.
	      // The input data array has shape nrant,nrant,nchan(subs),npol.
	      // So we can form an AIPS++ array for each baseline.
	      Array<Complex> dataArray(dShape, (Complex*)visibilityData, SHARE);
	      IPosition start(2, 0, channelId);
	      IPosition leng(2, shape[0], nrChannels);
	      dataArray.apply(std::conj); // Temporary fix, necessary to prevent flipping of the sky, since
	      // the UVW coordinates are reversed (20070515)
	      itsMSCol->data().putSlice(rowNumber, Slicer(start, leng), dataArray);
	    }
          catch (AipsError& e)
	    {
	      LOG_FATAL_STR("AipsError in writing data: " <<  e.what());
	    }
          // Write flags
          if (flags == 0) {
            itsMSCol->flag().put(rowNumber, defFlags);
          } else {
            Array<Bool> flagArray(dShape, const_cast<Bool*>(flags), SHARE);
            IPosition start(2, 0, channelId);   // Start position
            IPosition leng(2, shape[0], nrChannels);     // Length: ncorr, nchan
            itsMSCol->flag().putSlice(rowNumber, Slicer(start, leng), flagArray);
          }
          visibilityData += nrel*nrChannels;  // Go to next baseline data
          if (flags != 0) {
            flags += nrel*nrChannels;
          }
#if 0
          // Write weights
          Array<float> weightArray(wShape, (Float*)weights, SHARE);
          itsMSCol->weightSpectrum().put(rowNumber, weightArray);
#endif
          weights += nrChannels;

          rowNumber++;
	  }
	}
      
	itsMS->flush();

	MSwriteTimer.stop();

      } catch (AipsError x) {
	LOG_FATAL_STR("AIPS/Casa error in MSWriterCasa::write(): " << x.getMesg());
	exit(0);
      } catch (std::exception &ex){ 
	LOG_FATAL_STR("std::exception error in MSWriterCasa::write(): " << ex.what());
	exit(0);
      } catch (...) {
        LOG_FATAL("Unknown error in MSWriterCasa::write(): ");
	exit(0);
      } /// try
      sharedMutex.unlock();
    }
    
  } // namespace RTCP

} // namespace LOFAR

#endif // defined HAVE_AIPSPP
