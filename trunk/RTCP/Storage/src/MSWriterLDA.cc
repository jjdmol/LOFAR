//# MSWriterLDA: an implementation of MSWriter using the LDA to write HDF5
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
//#  $Id: $

#include <lofar_config.h>

#include <AMCBase/Epoch.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>

#include <Storage/MSWriter.h>
#include <Storage/MSWriterLDA.h>

#ifdef USE_LDA
#include <BF_File.h>

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

static std::string timeStr( double time )
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

static std::string toTAI( double time )
{
  using namespace casa;

  double UTC_MJD  = toMJD(time);
  double TAI_MJD  = MEpoch::Convert(MEpoch(MVEpoch(Quantity(UTC_MJD, "d")), MEpoch::Ref(MEpoch::UTC)), MEpoch::Ref(MEpoch::TAI))().getValue().get();
  double TAI_UNIX = fromMJD(TAI_MJD);

  return timeStr(TAI_UNIX);
}

static std::string stripextension( const std::string filename )
{
  return filename.substr(0,filename.rfind('.'));
}

static std::string forceextension( const std::string filename, const std::string extension )
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

    template <typename T,unsigned DIM> MSWriterLDA<T,DIM>::MSWriterLDA (const string &filename, const Parset &parset, OutputType outputType, unsigned fileno, bool isBigEndian)
    :
      MSWriterFile(forceextension(string(filename),".raw"),false),
      itsTransposeLogic(parset),
      itsNrChannels(parset.nrChannelsPerSubband() * itsTransposeLogic.nrSubbands(fileno)),
      itsNextSeqNr(0)
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

      unsigned nrBlocks = ceil((parset.stopTime() - parset.startTime()) / parset.CNintegrationTime());
      unsigned nrSubbands = itsTransposeLogic.nrSubbands(fileno);

      unsigned nrValuesPerStokes;
      std::vector<std::string> stokesVars;

      switch (outputType) {
        case INCOHERENT_STOKES:  {
          // assume stokes are either I or IQUV
          stokesVars.push_back("I");

          if (parset.nrIncoherentStokes() > 1) {
            stokesVars.push_back("Q");
            stokesVars.push_back("U");
            stokesVars.push_back("V");
          }

          nrValuesPerStokes = 1;

          itsNrSamples = parset.CNintegrationSteps() / parset.incoherentStokesTimeIntegrationFactor();
          break;
        }

        case COHERENT_STOKES: {
          // assume stokes are either I or IQUV
          stokesVars.push_back("I");

          if (parset.nrIncoherentStokes() > 1) {
            stokesVars.push_back("Q");
            stokesVars.push_back("U");
            stokesVars.push_back("V");
          }

          nrValuesPerStokes = 1;

          itsNrSamples = parset.CNintegrationSteps() / parset.coherentStokesTimeIntegrationFactor();
          break;
        }

        case BEAM_FORMED_DATA: {
          if (parset.nrCoherentStokes() == 2) {
            stokesVars.push_back("X");
            stokesVars.push_back("Y");
          } else {
            stokesVars.push_back("Xr");
            stokesVars.push_back("Xi");
            stokesVars.push_back("Yr");
            stokesVars.push_back("Yi");
          }

          nrValuesPerStokes = 4 / parset.nrCoherentStokes();

          itsNrSamples = parset.CNintegrationSteps();
          break;
        }

        default:
          THROW(StorageException, "MSWriterLDA can only handle Coherent Stokes and Beam-formed Data");
      }

      itsZeroBlock.resize( itsNrSamples * itsNrChannels * nrValuesPerStokes );

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

#if 0
      std::vector<unsigned> subbands = parset.subbandList();
      unsigned max_subband = *std::max_element( subbands.begin(), subbands.end() );
      unsigned min_subband = *std::min_element( subbands.begin(), subbands.end() );
#endif

      const std::vector<double> subbandCenterFrequencies = parset.subbandToFrequencyMapping();
      double min_centerfrequency = *std::min_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
      double max_centerfrequency = *std::max_element( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end() );
      double sum_centerfrequencies = std::accumulate( subbandCenterFrequencies.begin(), subbandCenterFrequencies.end(), 0.0 );

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
      file.bandwidth()           .set(parset.nrSubbands() * parset.sampleRate() / 1e6);

      // SysLog group -- empty for now
      file.sysLog().create();

      // Information about the station beam (SAP)
      file.nofSubArrayPointings().set(parset.nrBeams());
      BF_SubArrayPointing sap = file.subArrayPointing(sapNr);

      sap.create();
      sap.groupType()   .set("SubArrayPointing");
      sap.nofStations() .set(parset.nrStations()); // TODO: SS beamformer?
      sap.stationsList().set(parset.allStationNames());

      // TODO: non-J2000 pointings
      ASSERT( parset.getBeamDirectionType(sapNr) == "J2000" );

      std::vector<double> beamDir = parset.getBeamDirection(sapNr);
      sap.pointRA() .set(beamDir[0] * 180.0 / M_PI);
      sap.pointDEC().set(beamDir[1] * 180.0 / M_PI);

      sap.clockRate()         .set(parset.clockSpeed());
      sap.clockRateUnit()     .set("Hz");

      sap.nofSamples()        .set(itsNrSamples * nrBlocks);
      sap.samplingRate()      .set(1.0 * itsNrSamples / parset.CNintegrationTime());
      sap.samplingRateUnit()  .set("Hz");

      sap.channelsPerSubband().set(parset.nrChannelsPerSubband());
      sap.subbandWidth()      .set(subbandBandwidth);
      sap.subbandWidthUnit()  .set("Hz");
      sap.channelWidth()      .set(channelBandwidth);
      sap.channelWidthUnit()  .set("Hz");

      sap.nofBeams()          .set(parset.nrPencilBeams(sapNr));

      // Information about the pencil beam

      BF_BeamGroup beam = sap.beam(beamNr);

      beam.create();
      beam.groupType()   .set("Beam");
      beam.nofStations() .set(parset.nrStations());
      beam.stationsList().set(parset.allStationNames()); // TODO: SS beamformer, support subsets of allStations

      //const char *trackingTypes[] = { "J2000", "LMN", "TBD" };
      //writeAttribute(         beam, "TRACKING",      "J2000" ); // TODO: support non-tracking
      // TODO: non-J2000 pointings
      //ASSERT( parset.getBeamDirectionType() == "J2000" );
      BeamCoordinates pbeamDirs = parset.pencilBeams(sapNr);
      BeamCoord3D pbeamDir = pbeamDirs[beamNr];
      beam.pointRA()       .set((beamDir[0] + pbeamDir[0]) * 180.0 / M_PI);
      beam.pointDEC()      .set((beamDir[0] + pbeamDir[0]) * 180.0 / M_PI);
      beam.pointOffsetRA() .set(pbeamDir[0] * 180.0 / M_PI);
      beam.pointOffsetDEC().set(pbeamDir[0] * 180.0 / M_PI);

      const std::vector<unsigned> sapMapping = parset.subbandToSAPmapping();
      double beamCenterFrequencySum = 0.0;

      for (unsigned i = 0; i < sapMapping.size(); i++)
        if (sapMapping[i] == sapNr)
          beamCenterFrequencySum += subbandCenterFrequencies[i];

      //beam.beamFrequencyCenter().set(beamCenterFrequencySum / nrSubbands);

      double DM = parset.dispersionMeasure(sapNr, beamNr);

      beam.foldedData()             .set(false);
      beam.dedispersion()           .set(DM == 0.0 ? "NONE" : "COHERENT");
      beam.dedispersionMeasure()    .set(DM);
      beam.dedispersionMeasureUnit().set("pc/cm^3");

      //beam.baryCenter()             .set(false);

      beam.nofStokes()              .set(stokesVars.size());
      beam.stokesComponents()       .set(stokesVars);
      beam.complexVoltages()        .set(outputType == BEAM_FORMED_DATA);
      beam.signalSum()              .set(outputType == INCOHERENT_STOKES ? "INCOHERENT" : "COHERENT");

      CoordinatesGroup coordinates = beam.coordinates();

      coordinates.refLocationValue().set(parset.getRefPhaseCentre());
      coordinates.refLocationUnit().set(std::vector<std::string>(3,"m"));
      coordinates.refLocationFrame().set("ITRF");

      coordinates.refTimeValue().set(toMJD(parset.startTime()));
      coordinates.refTimeUnit().set("d");
      coordinates.refTimeFrame().set("MJD");

      BF_StokesDataset stokesDS = beam.stokes(stokesNr);

      vector<ssize_t> dims(2), maxdims(2);

      dims[0] = itsNrSamples * nrBlocks;
      dims[1] = itsNrChannels;

      maxdims[0] = -1;
      maxdims[1] = itsNrChannels;

      stokesDS.create(dims, maxdims, LOFAR::basename(rawfilename), isBigEndian ? BF_StokesDataset::BIG : BF_StokesDataset::LITTLE);

      stokesDS.stokesComponent().set(stokesVars[stokesNr]);
      stokesDS.nofChannels()    .set(vector<unsigned>(nrSubbands, parset.nrChannelsPerSubband()));
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
      ASSERTSTR( sdata->samples.num_elements() >= itsZeroBlock.size(), "A block is at least " << itsZeroBlock.size() << " elements, but provided sdata only has " << sdata->samples.num_elements() << " elements" );

      unsigned seqNr = data->sequenceNumber();
      unsigned bytesPerBlock = itsZeroBlock.size() * sizeof(T);

      // fill in zeroes for lost blocks
      if (itsNextSeqNr < seqNr)
        itsFile.skip((seqNr - itsNextSeqNr) * bytesPerBlock);

      // make sure we skip |2 in the highest dimension
      itsFile.write(sdata->samples.origin(), bytesPerBlock);

      itsNextSeqNr = seqNr + 1;
    }

    // specialisation for StokesData
    template class MSWriterLDA<float,3>;

    // specialisation for FinalBeamFormedData
    template class MSWriterLDA<float,4>;
  } // namespace RTCP
} // namespace LOFAR

#else // no USE_LDA

namespace LOFAR 
{

  namespace RTCP
  {

    template <typename T,unsigned DIM> MSWriterLDA<T,DIM>::MSWriterLDA (const string &filename, const Parset &parset, OutputType outputType, unsigned fileno, bool isBigEndian)
    :
      MSWriterFile(filename,false)
    {
      LOG_ERROR_STR( "Using the LDA writer is not supported (file: " << filename << ")" );
    }

    template <typename T,unsigned DIM> MSWriterLDA<T,DIM>::~MSWriterLDA()
    {
    }

  } // namespace RTCP
} // namespace LOFAR

#endif
