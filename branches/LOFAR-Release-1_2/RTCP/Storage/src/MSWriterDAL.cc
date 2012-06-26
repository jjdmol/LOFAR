//# MSWriterDAL: an implementation of MSWriter using the DAL to write HDF5
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
#include <Storage/MSWriterDAL.h>
#include <Storage/Package__Version.h>

#include <dal/lofar/BF_File.h>
#include <dal/dal_version.h>

using namespace DAL;
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

static string toUTC( double time )
{
  return timeStr(time) + "Z";
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

    template <typename T,unsigned DIM> MSWriterDAL<T,DIM>::MSWriterDAL (const string &filename, const Parset &parset, unsigned fileno, bool isBigEndian)
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
          LOG_ERROR("MSWriterDAL asked to write INVALID_STOKES");
          return;
      }    

      LOG_DEBUG_STR("MSWriterDAL: opening " << filename);

      // create the top structure
      BF_File file(h5filename, BF_File::CREATE);

      // Common Attributes
      file.groupType().value = "Root";
      file.fileName() .value = LOFAR::basename(h5filename);

      char now_str[50];
      time_t now = time(NULL);
      if (strftime( now_str, sizeof now_str, "%Y-%m-%dT%H:%M:%S.0", gmtime(&now) ) > 0 )
        file.fileDate().value = now_str;

      file.fileType() .value = "bf";
      file.telescope().value = "LOFAR";
      file.observer() .value = "unknown";

      file.projectID()     .value = parset.getString("Observation.Campaign.name");
      file.projectTitle()  .value = parset.getString("Observation.Scheduler.taskName");
      file.projectPI()     .value = parset.getString("Observation.Campaign.PI");
      file.projectCOI()    .value = parset.getString("Observation.Campaign.CO_I");
      file.projectContact().value = parset.getString("Observation.Campaign.contact");

      file.observationID() .value = str(format("%u") % parset.observationID());

      file.observationStartUTC().value = toUTC(parset.startTime());
      file.observationStartMJD().value = toMJD(parset.startTime());
      file.observationStartTAI().value = toTAI(parset.startTime());

      // because we process in blocks, the stop time can be a bit further than the one
      // actually specified.
      double stopTime = parset.startTime() + nrBlocks * parset.CNintegrationTime();

      file.observationEndUTC().value = toUTC(stopTime);
      file.observationEndMJD().value = toMJD(stopTime);
      file.observationEndTAI().value = toTAI(stopTime);

      file.observationNofStations().value = parset.nrStations(); // TODO: SS beamformer?
      file.observationStationsList().value = parset.allStationNames(); // TODO: SS beamformer?

      const vector<double> subbandCenterFrequencies = parset.subbandToFrequencyMapping();
      double min_centerfrequency = *min_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
      double max_centerfrequency = *max_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
      double sum_centerfrequencies = accumulate( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end(), 0.0 );

      double subbandBandwidth = parset.sampleRate();
      double channelBandwidth = parset.channelWidth();

      file.observationFrequencyMax()   .value = (max_centerfrequency + subbandBandwidth / 2) / 1e6;
      file.observationFrequencyMin()   .value = (min_centerfrequency - subbandBandwidth / 2) / 1e6;
      file.observationFrequencyCenter().value = sum_centerfrequencies / subbandCenterFrequencies.size() / 1e6;
      file.observationFrequencyUnit()  .value = "MHz";

      file.observationNofBitsPerSample().value = parset.nrBitsPerSample();
      file.clockFrequency()             .value = parset.clockSpeed() / 1e6;
      file.clockFrequencyUnit()         .value = "MHz";

      file.antennaSet()     .value = parset.antennaSet();
      file.filterSelection().value = parset.getString("Observation.bandFilter");

      unsigned nrSAPs = parset.nrBeams();
      vector<string> targets(nrSAPs);

      for (unsigned sap = 0; sap < nrSAPs; sap++)
        targets[sap] = parset.beamTarget(sap);

      file.targets().value = targets;

      file.systemVersion().value   = StorageVersion::getVersion(); // LOFAR version
      file.pipelineName().value    = "";
      file.pipelineVersion().value = "";

      file.docName() .value   = "ICD 3: Beam-Formed Data";
      file.docVersion().value = "2.04.27";

      file.notes().value      = "";

      // BF_File specific root group parameters

      file.createOfflineOnline().value = "Online";
      file.BFFormat().value            = "TAB";
      file.BFVersion().value           = str(format("RTCP/Storage %s r%s using DAL %s and HDF5 %s") % StorageVersion::getVersion() % StorageVersion::getRevision() % DAL::get_lib_version() % DAL::get_dal_hdf5_version());

      file.totalIntegrationTime()    .value = nrBlocks * parset.CNintegrationTime();
      file.totalIntegrationTimeUnit().value = "s";
      file.observationDataType()     .value = "";

      //file.subArrayPointingDiameter().value = 0.0;
      //file.subArrayPointingDiameterUnit().value = "arcmin";
      file.bandwidth()               .value = parset.nrSubbands() * subbandBandwidth / 1e6;
      file.bandwidthUnit()           .value = "MHz";
      //file.beamDiameter()            .value = 0.0;
      //file.beamDiameterUnit()          .value = "arcmin";

      file.observationNofSubArrayPointings().value = parset.nrBeams();
      file.nofSubArrayPointings().value            = 1;

      // SysLog group -- empty for now
      file.sysLog().create();

      // information about the station beam (SAP)
      BF_SubArrayPointing sap = file.subArrayPointing(sapNr);
      sap.create();
      sap.groupType()   .value = "SubArrayPointing";
      sap.target()      .value = targets[sapNr];

      sap.expTimeStartUTC().value = toUTC(parset.startTime());
      sap.expTimeStartMJD().value = toMJD(parset.startTime());
      sap.expTimeStartTAI().value = toTAI(parset.startTime());

      sap.expTimeEndUTC().value = toUTC(stopTime);
      sap.expTimeEndMJD().value = toMJD(stopTime);
      sap.expTimeEndTAI().value = toTAI(stopTime);

      // TODO: non-J2000 pointings
      if( parset.getBeamDirectionType(sapNr) != "J2000" )
        LOG_WARN("HDF5 writer does not record positions of non-J2000 observations yet.");

      vector<double> beamDir = parset.getBeamDirection(sapNr);
      sap.pointRA() .value = beamDir[0] * 180.0 / M_PI;
      sap.pointDEC().value = beamDir[1] * 180.0 / M_PI;

      sap.subbandWidth()      .value = subbandBandwidth;
      sap.subbandWidthUnit()  .value = "Hz";

      sap.observationNofBeams().value = parset.nrPencilBeams(sapNr);
      sap.nofBeams()           .value = 1;

      BF_ProcessingHistory sapHistory = sap.processHistory();
      sapHistory.create();

      string parsetAsString;
      parset.writeBuffer(parsetAsString);

      sapHistory.observationParset().value = parsetAsString;
      sapHistory.observationLog()   .value = "";
      sapHistory.prestoParset()     .value = "";
      sapHistory.prestoLog()        .value = "";

      // information about the pencil beam

      BF_BeamGroup beam = sap.beam(beamNr);
      beam.create();
      beam.groupType()         .value = "Beam";

      vector<string> beamStationList = parset.pencilBeamStationList(sapNr, beamNr);
      beam.nofStations() .value = beamStationList.size();
      beam.stationsList().value = beamStationList;

      beam.tracking().value     = parset.getBeamDirectionType(sapNr);

      BeamCoordinates pbeamDirs = parset.pencilBeams(sapNr);
      BeamCoord3D pbeamDir = pbeamDirs[beamNr];
      beam.pointRA()           .value = (beamDir[0] + pbeamDir[0]) * 180.0 / M_PI;
      beam.pointRAUnit()       .value = "deg";
      beam.pointDEC()          .value = (beamDir[1] + pbeamDir[1]) * 180.0 / M_PI;
      beam.pointDECUnit()      .value = "deg";
      beam.pointOffsetRA()     .value = pbeamDir[0] * 180.0 / M_PI;
      beam.pointOffsetRAUnit() .value = "deg";
      beam.pointOffsetDEC()    .value = pbeamDir[1] * 180.0 / M_PI;
      beam.pointOffsetDECUnit().value = "deg";

      beam.beamDiameterRA()     .value = 0;
      beam.beamDiameterRAUnit() .value = "arcmin";
      beam.beamDiameterDEC()    .value = 0;
      beam.beamDiameterDECUnit().value = "arcmin";

      beam.nofSamples()        .value = itsNrSamples * nrBlocks;
      beam.samplingRate()      .value = parset.sampleRate() / parset.nrChannelsPerSubband() / itsInfo.timeIntFactor;
      beam.samplingRateUnit()  .value = "Hz";
      beam.samplingTime()      .value = parset.sampleDuration() * parset.nrChannelsPerSubband() * itsInfo.timeIntFactor;
      beam.samplingTimeUnit()  .value = "s";

      beam.channelsPerSubband().value = itsInfo.nrChannels;
      beam.channelWidth()      .value = channelBandwidth * (parset.nrChannelsPerSubband() / itsInfo.nrChannels);
      beam.channelWidthUnit()  .value = "Hz";

      vector<double> beamCenterFrequencies(nrSubbands, 0.0);

      for (unsigned sb = 0; sb < nrSubbands; sb++)
        beamCenterFrequencies[sb] = subbandCenterFrequencies[subbandIndices[sb]];

      double beamCenterFrequencySum = accumulate(beamCenterFrequencies.begin(), beamCenterFrequencies.end(), 0.0);

      beam.beamFrequencyCenter()    .value = beamCenterFrequencySum / nrSubbands / 1e6;
      beam.beamFrequencyCenterUnit().value = "MHz";

      double DM = parset.dispersionMeasure(sapNr, beamNr);

      beam.foldedData()             .value = false;
      beam.foldPeriod()             .value = 0.0;
      beam.foldPeriodUnit()         .value = "s";

      beam.dedispersion()           .value = DM == 0.0 ? "NONE" : "COHERENT";
      beam.dedispersionMeasure()    .value = DM;
      beam.dedispersionMeasureUnit().value = "pc/cm^3";

      beam.barycentered()           .value = false;

      beam.observationNofStokes()   .value = itsInfo.nrStokes;
      beam.nofStokes()              .value = 1;

      vector<string> stokesComponents(1, stokesVars[stokesNr]);

      beam.stokesComponents()       .value = stokesComponents;
      beam.complexVoltages()        .value = itsInfo.stokesType == STOKES_XXYY;
      beam.signalSum()              .value = itsInfo.coherent ? "COHERENT" : "INCOHERENT";

      BF_ProcessingHistory beamHistory = beam.processHistory();
      beamHistory.create();

      beamHistory.observationParset().value = parsetAsString;
      beamHistory.observationLog()   .value = "";
      beamHistory.prestoParset()     .value = "";
      beamHistory.prestoLog()        .value = "";

      CoordinatesGroup coordinates = beam.coordinates();
      coordinates.create();
      coordinates.groupType().value = "Coordinates";

      coordinates.refLocationValue().value = parset.getRefPhaseCentre();
      coordinates.refLocationUnit().value = vector<string>(3,"m");
      coordinates.refLocationFrame().value = "ITRF";

      coordinates.refTimeValue().value = toMJD(parset.startTime());
      coordinates.refTimeUnit().value = "d";
      coordinates.refTimeFrame().value = "MJD";

      coordinates.nofCoordinates().value = 2;
      coordinates.nofAxes()       .value = 2;

      vector<string> coordinateTypes(2);
      coordinateTypes[0] = "Time"; // or TimeCoord ?
      coordinateTypes[1] = "Spectral"; // or SpectralCoord ?
      coordinates.coordinateTypes().value = coordinateTypes;

      SmartPtr<TimeCoordinate> timeCoordinate = dynamic_cast<TimeCoordinate*>(coordinates.coordinate(0));
      timeCoordinate.get()->create();
      timeCoordinate.get()->groupType()     .value = "TimeCoord";

      timeCoordinate.get()->coordinateType().value = "Time";
      timeCoordinate.get()->storageType()   .value = vector<string>(1,"Linear");
      timeCoordinate.get()->nofAxes()       .value = 1;
      timeCoordinate.get()->axisNames()     .value = vector<string>(1,"Time");
      timeCoordinate.get()->axisUnits()     .value = vector<string>(1,"us");

      // linear coordinates:
      //   referenceValue = offset from starting time, in axisUnits
      //   referencePixel = offset from first sample
      //   increment      = time increment for each sample
      //   pc             = scaling factor (?)

      timeCoordinate.get()->referenceValue().value = 0;
      timeCoordinate.get()->referencePixel().value = 0;
      timeCoordinate.get()->increment()     .value = parset.sampleDuration() * itsInfo.timeIntFactor;

      timeCoordinate.get()->axisValuesPixel().value = vector<unsigned>(1, 0); // not used
      timeCoordinate.get()->axisValuesWorld().value = vector<double>(1, 0.0); // not used

      SmartPtr<SpectralCoordinate> spectralCoordinate = dynamic_cast<SpectralCoordinate*>(coordinates.coordinate(1));
      spectralCoordinate.get()->create();
      spectralCoordinate.get()->groupType()     .value = "SpectralCoord";

      spectralCoordinate.get()->coordinateType().value = "Spectral";
      spectralCoordinate.get()->storageType()   .value = vector<string>(1,"Tabular");
      spectralCoordinate.get()->nofAxes()       .value = 1;
      spectralCoordinate.get()->axisNames()     .value = vector<string>(1,"Frequency");
      spectralCoordinate.get()->axisUnits()     .value = vector<string>(1,"MHz");

      spectralCoordinate.get()->referenceValue().value = 0; // not used
      spectralCoordinate.get()->referencePixel().value = 0; // not used
      spectralCoordinate.get()->increment()     .value = 0; // not used

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

      spectralCoordinate.get()->axisValuesPixel().value = spectralPixels;
      spectralCoordinate.get()->axisValuesWorld().value = spectralWorld;

      BF_StokesDataset stokesDS = beam.stokes(stokesNr);

      vector<ssize_t> dims(2), maxdims(2);

      dims[0] = itsNrSamples * nrBlocks;
      dims[1] = itsNrChannels;

      maxdims[0] = -1;
      maxdims[1] = itsNrChannels;

      stokesDS.create(dims, maxdims, LOFAR::basename(rawfilename), isBigEndian ? BF_StokesDataset::BIG : BF_StokesDataset::LITTLE);
      stokesDS.groupType().value = "bfData";
      stokesDS.dataType() .value = "float";

      stokesDS.stokesComponent().value = stokesVars[stokesNr];
      stokesDS.nofChannels()    .value = vector<unsigned>(nrSubbands, itsInfo.nrChannels);
      stokesDS.nofSubbands()    .value = nrSubbands;
      stokesDS.nofSamples()     .value = dims[0];
    }

    template <typename T,unsigned DIM> MSWriterDAL<T,DIM>::~MSWriterDAL()
    {
    }

    template <typename T,unsigned DIM> void MSWriterDAL<T,DIM>::write(StreamableData *data)
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
    template class MSWriterDAL<float,3>;

  } // namespace RTCP
} // namespace LOFAR

