//# MSWriterLDA: an implementation of MSWriter using the LDA to write HDF5
//#
//#  Copyright (C) 2011
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
//#  $Id: $

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>

#include <Storage/MSWriter.h>
#include <Storage/MSWriterLDA.h>

#include <lofar/BF_File.h>

using namespace LDA;
using namespace std;

#include <Common/Thread/Mutex.h>
#include <Interface/StreamableData.h>
#include <iostream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <measures/Measures.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MEpoch.h>

#include <boost/format.hpp>
using boost::format;

static string timeStr( double time )
{
  time_t timeSec = static_cast<time_t>(floor(time));
  unsigned long timeNSec = static_cast<unsigned long>(round( (time-floor(time))*1e9 ));

  char utcstr[50];
  if (strftime( utcstr, sizeof utcstr, "%Y-%m-%dT%H:%M:%S", gmtime(&timeSec) ) == 0)
    return "";

  return str(format("%s.%09lu") % utcstr % timeNSec);
}

static double toMJD( double time )
{
  // 40587 modify Julian day number = 00:00:00 January 1, 1970, GMT
  return 40587.0 + time / (24*60*60);
}

static double fromMJD( double time )
{
  // 40587 modify Julian day number = 00:00:00 January 1, 1970, GMT
  return (time - 40587.0) * (24*60*60);
}

static string toTAI( double time )
{
  using namespace casa;

  double UTC_MJD  = toMJD(time);
  double TAI_MJD  = MEpoch::Convert(MEpoch(MVEpoch(Quantity(UTC_MJD, "d")), MEpoch::Ref(MEpoch::UTC)), MEpoch::Ref(MEpoch::TAI))().getValue().get();
  double TAI_UNIX = fromMJD(TAI_MJD);

  return timeStr(TAI_UNIX);
}

static string stripextension( const string filename )
{
  return filename.substr(0,filename.rfind('.'));
}

static string forceextension( const string filename, const string extension )
{
  return stripextension(filename) + extension;
}

namespace LOFAR 
{

  namespace RTCP
  {
    // Prevent concurrent access to HDF5, which may not be compiled thread-safe. The Thread-safe version
    // uses global locks too anyway.
    static Mutex HDF5Mutex;

    template <typename T,unsigned DIM> MSWriterLDA<T,DIM>::MSWriterLDA (const string &filename, const Parset &parset, unsigned fileno, bool isBigEndian)
    :
      MSWriterFile(forceextension(string(filename),".raw"),false),
      itsTransposeLogic(parset.transposeLogic()),
      itsInfo(itsTransposeLogic.streamInfo[fileno]),
      itsNrChannels(itsInfo.nrChannels * itsInfo.subbands.size()),
      itsNrSamples(itsInfo.nrSamples),
      itsNextSeqNr(0),
      itsBlockSize(itsNrSamples * itsNrChannels)
    {
      string h5filename = forceextension(string(filename),".h5");
      string rawfilename = forceextension(string(filename),".raw");

      ScopedLock sl(HDF5Mutex);

#if 0
      // install our own error handler
      H5Eset_auto_stack(H5E_DEFAULT, my_hdf5_error_handler, NULL);
#endif

      unsigned sapNr, beamNr, stokesNr, partNr;

      itsTransposeLogic.decompose( fileno, sapNr, beamNr, stokesNr, partNr );

      unsigned nrBlocks = floor((parset.stopTime() - parset.startTime()) / parset.CNintegrationTime());
      unsigned nrSubbands = itsInfo.subbands.size();
      const vector<unsigned> &subbandIndices = itsInfo.subbands;
      const vector<unsigned> allSubbands = parset.subbandList();

      vector<unsigned> subbands(nrSubbands, 0); // actual subbands written in this file

      for (unsigned sb = 0; sb < nrSubbands; sb++)
        subbands[sb] = allSubbands[subbandIndices[sb]];

      vector<string> stokesVars;

      switch (itsInfo.stokesType) {
        case STOKES_I:
          stokesVars.push_back("I");
          break;

        case STOKES_IQUV:
          stokesVars.push_back("I");
          stokesVars.push_back("Q");
          stokesVars.push_back("U");
          stokesVars.push_back("V");
          break;

        case STOKES_XXYY:
          stokesVars.push_back("Xr");
          stokesVars.push_back("Xi");
          stokesVars.push_back("Yr");
          stokesVars.push_back("Yi");
          break;

        case INVALID_STOKES:
          LOG_ERROR("MSWriterLDA asked to write INVALID_STOKES");
          return;
      }    

      LOG_DEBUG_STR("MSWriterLDA: opening " << filename);

      // create the top structure
      BF_File file(h5filename, BF_File::CREATE);

      // Common Attributes
      file.groupType().set("Root");
      file.fileName() .set(LOFAR::basename(h5filename));

      char now_str[50];
      time_t now = time(NULL);
      if (strftime( now_str, sizeof now_str, "%Y-%m-%dT%H:%M:%S.0", gmtime(&now) ) > 0 )
        file.fileDate().set(now_str);

      file.fileType() .set("bf");
      file.telescope().set("LOFAR");
      file.observer() .set("unknown");

      file.projectID()     .set(parset.getString("Observation.Campaign.name"));
      file.projectTitle()  .set(parset.getString("Observation.Campaign.title"));
      file.projectPI()     .set(parset.getString("Observation.Campaign.PI"));
      file.projectCOI()    .set(parset.getString("Observation.Campaign.CO_I")); // TODO: actually a vector, so pretty print
      file.projectContact().set(parset.getString("Observation.Campaign.contact"));

      file.observationID() .set(str(format("%s") % parset.observationID()));

      file.observationStartUTC().set(timeStr(parset.startTime()));
      file.observationStartMJD().set(toMJD(parset.startTime()));
      file.observationStartTAI().set(toTAI(parset.startTime()));

      // because we process in blocks, the stop time can be a bit further than the one
      // actually specified.
      double stopTime = parset.startTime() + nrBlocks * parset.CNintegrationTime();

      file.observationEndUTC().set(timeStr(stopTime));
      file.observationEndMJD().set(toMJD(stopTime));
      file.observationEndTAI().set(toTAI(stopTime));

      file.observationNofStations().set(parset.nrStations()); // TODO: SS beamformer?
      file.observationStationsList().set(parset.allStationNames()); // TODO: SS beamformer?

      const vector<double> subbandCenterFrequencies = parset.subbandToFrequencyMapping();
      double min_centerfrequency = *min_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
      double max_centerfrequency = *max_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
      double sum_centerfrequencies = accumulate( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end(), 0.0 );

      double subbandBandwidth = parset.sampleRate();
      double channelBandwidth = parset.channelWidth();

      file.observationFrequencyMin()   .set((min_centerfrequency - subbandBandwidth / 2) / 1e6);
      file.observationFrequencyCenter().set(sum_centerfrequencies / subbandCenterFrequencies.size());
      file.observationFrequencyMax()   .set((max_centerfrequency + subbandBandwidth / 2) / 1e6);
      file.observationFrequencyUnit()  .set("MHz");

      file.observationNofBitsPerSample().set(parset.nrBitsPerSample());
      file.clockFrequency()             .set(parset.clockSpeed() / 1e6);
      file.clockFrequencyUnit()         .set("MHz");

      file.antennaSet()     .set(parset.antennaSet());
      file.filterSelection().set(parset.getString("Observation.bandFilter"));

      file.ICDNumber() .set("3");
      file.ICDVersion().set("2.04.10");

      // BF_File specific root group parameters

      file.createOfflineOnline().set("Online");

      file.expTimeStartUTC().set(timeStr(parset.startTime()));
      file.expTimeStartMJD().set(toMJD(parset.startTime()));
      file.expTimeStartTAI().set(toTAI(parset.startTime()));

      file.expTimeEndUTC().set(timeStr(stopTime));
      file.expTimeEndMJD().set(toMJD(stopTime));
      file.expTimeEndTAI().set(toTAI(stopTime));

      file.totalIntegrationTime().set(nrBlocks * parset.CNintegrationTime());
      file.bandwidth()           .set(parset.nrSubbands() * subbandBandwidth / 1e6);

      file.nofSubArrayPointings().set(parset.nrBeams());

      // SysLog group -- empty for now
      file.sysLog().create();

      // information about the station beam (SAP)
      BF_SubArrayPointing sap = file.subArrayPointing(sapNr);
      sap.create();
      sap.groupType()   .set("SubArrayPointing");

      sap.nofStations() .set(parset.nrStations()); // TODO: SS beamformer?
      sap.stationsList().set(parset.allStationNames());

      // TODO: non-J2000 pointings
      if( parset.getBeamDirectionType(sapNr) != "J2000" )
        LOG_WARN("HDF5 writer does not record positions of non-J2000 observations yet.");

      vector<double> beamDir = parset.getBeamDirection(sapNr);
      sap.pointRA() .set(beamDir[0] * 180.0 / M_PI);
      sap.pointDEC().set(beamDir[1] * 180.0 / M_PI);

      sap.clockRate()         .set(parset.clockSpeed());
      sap.clockRateUnit()     .set("Hz");

      sap.nofSamples()        .set(itsNrSamples * nrBlocks);
      sap.samplingRate()      .set(parset.sampleRate());
      sap.samplingRateUnit()  .set("Hz");
      sap.samplingTime()      .set(1.0 / parset.sampleRate());
      sap.samplingTimeUnit()  .set("s");

      sap.channelsPerSubband().set(parset.nrChannelsPerSubband());
      sap.subbandWidth()      .set(subbandBandwidth);
      sap.subbandWidthUnit()  .set("Hz");
      sap.channelWidth()      .set(channelBandwidth);
      sap.channelWidthUnit()  .set("Hz");

      sap.nofBeams()          .set(parset.nrPencilBeams(sapNr));

      // information about the pencil beam

      BF_BeamGroup beam = sap.beam(beamNr);
      beam.create();
      beam.groupType()   .set("Beam");

      vector<string> beamStationList = parset.pencilBeamStationList(sapNr, beamNr);
      beam.nofStations() .set(beamStationList.size());
      beam.stationsList().set(beamStationList);

      //const char *trackingTypes[] = { "J2000", "LMN", "TBD" };
      //writeAttribute(         beam, "TRACKING",      "J2000" ); // TODO: support non-tracking
      // TODO: non-J2000 pointings
      //ASSERT( parset.getBeamDirectionType() == "J2000" );
      BeamCoordinates pbeamDirs = parset.pencilBeams(sapNr);
      BeamCoord3D pbeamDir = pbeamDirs[beamNr];
      beam.pointRA()       .set((beamDir[0] + pbeamDir[0]) * 180.0 / M_PI);
      beam.pointDEC()      .set((beamDir[1] + pbeamDir[1]) * 180.0 / M_PI);
      beam.pointOffsetRA() .set(pbeamDir[0] * 180.0 / M_PI);
      beam.pointOffsetDEC().set(pbeamDir[1] * 180.0 / M_PI);

      vector<double> beamCenterFrequencies(nrSubbands, 0.0);

      for (unsigned sb = 0; sb < nrSubbands; sb++)
        beamCenterFrequencies[sb] = subbandCenterFrequencies[subbandIndices[sb]];

      double beamCenterFrequencySum = accumulate(beamCenterFrequencies.begin(), beamCenterFrequencies.end(), 0.0);

      beam.beamFrequencyCenter().set(beamCenterFrequencySum / nrSubbands);

      double DM = parset.dispersionMeasure(sapNr, beamNr);

      beam.foldedData()             .set(false);
      beam.dedispersion()           .set(DM == 0.0 ? "NONE" : "COHERENT");
      beam.dedispersionMeasure()    .set(DM);
      beam.dedispersionMeasureUnit().set("pc/cm^3");

      //beam.baryCenter()             .set(false);

      beam.nofStokes()              .set(itsInfo.nrStokes);
      beam.stokesComponents()       .set(stokesVars);
      beam.complexVoltages()        .set(itsInfo.stokesType == STOKES_XXYY);
      beam.signalSum()              .set(itsInfo.coherent ? "COHERENT" : "INCOHERENT");

      CoordinatesGroup coordinates = beam.coordinates();
      coordinates.create();
      coordinates.groupType().set("Coordinates");

      coordinates.refLocationValue().set(parset.getRefPhaseCentre());
      coordinates.refLocationUnit().set(vector<string>(3,"m"));
      coordinates.refLocationFrame().set("ITRF");

      coordinates.refTimeValue().set(toMJD(parset.startTime()));
      coordinates.refTimeUnit().set("d");
      coordinates.refTimeFrame().set("MJD");

      coordinates.nofCoordinates().set(2);
      coordinates.nofAxes()       .set(2);

      vector<string> coordinateTypes(2);
      coordinateTypes[0] = "Time"; // or TimeCoord ?
      coordinateTypes[1] = "Spectral"; // or SpectralCoord ?
      coordinates.coordinateTypes().set(coordinateTypes);

      SmartPtr<TimeCoordinate> timeCoordinate = dynamic_cast<TimeCoordinate*>(coordinates.coordinate(0));
      timeCoordinate.get()->create();
      timeCoordinate.get()->groupType()     .set("TimeCoord");

      timeCoordinate.get()->coordinateType().set("Time");
      timeCoordinate.get()->storageType()   .set(vector<string>(1,"Linear"));
      timeCoordinate.get()->nofAxes()       .set(1);
      timeCoordinate.get()->axisNames()     .set(vector<string>(1,"Time"));
      timeCoordinate.get()->axisUnits()     .set(vector<string>(1,"us"));

      // linear coordinates:
      //   referenceValue = offset from starting time, in axisUnits
      //   referencePixel = offset from first sample
      //   increment      = time increment for each sample
      //   pc             = scaling factor (?)

      timeCoordinate.get()->referenceValue().set(0);
      timeCoordinate.get()->referencePixel().set(0);
      timeCoordinate.get()->increment()     .set(parset.sampleDuration() * itsInfo.timeIntFactor);

      SmartPtr<SpectralCoordinate> spectralCoordinate = dynamic_cast<SpectralCoordinate*>(coordinates.coordinate(1));
      spectralCoordinate.get()->create();
      spectralCoordinate.get()->groupType()     .set("SpectralCoord");

      spectralCoordinate.get()->coordinateType().set("Spectral");
      spectralCoordinate.get()->storageType()   .set(vector<string>(1,"Tabular"));
      spectralCoordinate.get()->nofAxes()       .set(1);
      spectralCoordinate.get()->axisNames()     .set(vector<string>(1,"Frequency"));
      spectralCoordinate.get()->axisUnits()     .set(vector<string>(1,"MHz"));

      // tabular coordinates:
      //   axisValuePixel = data indices
      //   axisValueWorld = corresponding (central) frequencies

      vector<unsigned> spectralPixels;
      vector<double> spectralWorld;

      for(unsigned sb = 0; sb < nrSubbands; sb++) {
        const double subbandBeginFreq = beamCenterFrequencies[sb] - 0.5 * subbandBandwidth;

        for(unsigned ch = 0; ch < itsInfo.nrChannels; ch++) {
          spectralPixels.push_back(spectralPixels.size());
          spectralWorld .push_back(subbandBeginFreq + ch * channelBandwidth);
        }
      }

      spectralCoordinate.get()->axisValuesPixel().set(spectralPixels);
      spectralCoordinate.get()->axisValuesWorld().set(spectralWorld);

      BF_StokesDataset stokesDS = beam.stokes(stokesNr);

      vector<ssize_t> dims(2), maxdims(2);

      dims[0] = itsNrSamples * nrBlocks;
      dims[1] = itsNrChannels;

      maxdims[0] = -1;
      maxdims[1] = itsNrChannels;

      stokesDS.create(dims, maxdims, LOFAR::basename(rawfilename), isBigEndian ? BF_StokesDataset::BIG : BF_StokesDataset::LITTLE);

      stokesDS.stokesComponent().set(stokesVars[stokesNr]);
      stokesDS.nofChannels()    .set(vector<unsigned>(nrSubbands, itsInfo.nrChannels));
      stokesDS.nofSamples()     .set(dims[0]);
    }

    template <typename T,unsigned DIM> MSWriterLDA<T,DIM>::~MSWriterLDA()
    {
    }

    template <typename T,unsigned DIM> void MSWriterLDA<T,DIM>::write(StreamableData *data)
    {
      SampleData<T,DIM> *sdata = dynamic_cast<SampleData<T,DIM> *>(data);

      ASSERT( data );
      ASSERT( sdata );
      ASSERTSTR( sdata->samples.num_elements() >= itsBlockSize, "A block is at least " << itsBlockSize << " elements, but provided sdata only has " << sdata->samples.num_elements() << " elements" );

      unsigned seqNr = data->sequenceNumber();
      unsigned bytesPerBlock = itsBlockSize * sizeof(T);

      // fill in zeroes for lost blocks
      if (itsNextSeqNr < seqNr)
        itsFile.skip((seqNr - itsNextSeqNr) * bytesPerBlock);

      // make sure we skip |2 in the highest dimension
      itsFile.write(sdata->samples.origin(), bytesPerBlock);

      itsNextSeqNr = seqNr + 1;
    }

    // specialisation for FinalBeamFormedData
    template class MSWriterLDA<float,3>;

  } // namespace RTCP
} // namespace LOFAR

