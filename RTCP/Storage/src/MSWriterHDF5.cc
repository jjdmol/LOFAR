//# MSWriterHDF5: an implementation of MSWriter using the DAL to weite HDF5
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
#include <Storage/MSWriterHDF5.h>

#ifdef HAVE_HDF5
#include <hdf5.h>

#if 0
#include <H5Epublic.h>
#endif

#include <Storage/HDF5Attributes.h>
#include <Common/Thread/Mutex.h>
#include <Interface/StreamableData.h>
#include <iostream>
#include <ctime>
#include <cmath>
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


#if 0
static herr_t errorwalker( unsigned n, const H5E_error2_t *err_desc, void *clientdata )
{
  (void)clientdata;

  H5E_msg_t *maj_ptr = static_cast<H5E_msg_t *>(H5Iobject_verify(err_desc->maj_num, H5I_ERROR_MSG));
  H5E_msg_t *min_ptr = static_cast<H5E_msg_t *>(H5Iobject_verify(err_desc->min_num, H5I_ERROR_MSG));

  LOG_ERROR_STR( "HDF5 trace #" << n
    << ": " << (maj_ptr && maj_ptr->msg ? maj_ptr->msg : "unknown")
    << " (" << (min_ptr && min_ptr->msg ? min_ptr->msg : "unknown") << ")" 
    << " in " << err_desc->func_name
    << " at " << err_desc->file_name << ":" << err_desc->line );
}

static herr_t errorhandler( hid_t stackid, void *clientdata )
{
  return H2Ewalk2( stackid, H5E_WALK_DOWNWARD, errorwalker, clientdata ); 
}
#endif

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

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::MSWriterHDF5 (const string &filename, const Parset &parset, OutputType outputType, unsigned fileno, bool isBigEndian)
    :
      MSWriterFile(forceextension(string(filename),".raw").c_str(),false),
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
      const char *stokes;

      itsTransposeLogic.decompose( fileno, sapNr, beamNr, stokesNr, partNr );

      unsigned nrBlocks = ceil((parset.stopTime() - parset.startTime()) / parset.CNintegrationTime());
      unsigned nrValuesPerStokes;


      switch (outputType) {
        case COHERENT_STOKES: {
          // assume stokes are either I or IQUV
          const char *stokesVars[] = { "I", "Q", "U", "V" };

          stokes = stokesVars[stokesNr];
          nrValuesPerStokes = 1;

          itsNrSamples = parset.CNintegrationSteps() / parset.coherentStokesTimeIntegrationFactor();
          break;
        }

        case BEAM_FORMED_DATA: {
          const char *stokesVars4[] = { "Xr", "Xi", "Yr", "Yi" };
          const char *stokesVars2[] = { "X", "Y" };

          stokes = parset.nrCoherentStokes() == 4 ? stokesVars4[stokesNr] : stokesVars2[stokesNr];
          nrValuesPerStokes = 4 / parset.nrCoherentStokes();

          itsNrSamples = parset.CNintegrationSteps();
          break;
        }

        default:
          THROW(StorageException, "MSWriterHDF5 can only handle Coherent Stokes and Beam-formed Data");
      }

      itsZeroBlock.resize( itsNrSamples * itsNrChannels * nrValuesPerStokes );

      LOG_DEBUG_STR("MSWriterHDF5: opening " << filename);

      hid_t ret;

      // create the top structure
      h5auto file(H5Fcreate( h5filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT ), H5Fclose);
      if (file <= 0)
        THROW( StorageException, "Could not open " << h5filename << " for writing" );

      writeAttribute( file, "GROUPTYPE", "Root" );
      writeAttribute( file, "FILENAME",  LOFAR::basename(h5filename).c_str() );

      char now_str[50];
      time_t now = time(NULL);
      if (strftime( now_str, sizeof now_str, "%Y-%m-%dT%H:%M:%S.0", gmtime(&now) ) > 0 )
        writeAttribute<const char*>( file, "FILEDATE",  now_str );

      writeAttribute( file, "FILETYPE",  "bf" );
      writeAttribute( file, "TELESCOPE", "LOFAR" );
      writeAttribute( file, "OBSERVER",  "unknown" );

      writeAttribute( file, "PROJECT_ID",      parset.getString("Observation.Campaign.name") );
      writeAttribute( file, "PROJECT_TITLE",   parset.getString("Observation.Campaign.title") );
      writeAttribute( file, "PROJECT_PI",      parset.getString("Observation.Campaign.PI") );
      writeAttribute( file, "PROJECT_CO_I",    parset.getString("Observation.Campaign.CO_I") ); // TODO: actually a vector, so pretty print a bit more
      writeAttribute( file, "PROJECT_CONTACT", parset.getString("Observation.Campaign.contact") );

      writeAttribute( file, "OBSERVATION_ID",  str(format("%s") % parset.observationID()).c_str() );

      writeAttribute(         file, "OBSERVATION_START_UTC",  timeStr(parset.startTime()) );
      writeAttribute<double>( file, "OBSERVATION_START_MJD",  toMJD(parset.startTime()) );
      writeAttribute(         file, "OBSERVATION_START_TAI",  toTAI(parset.startTime()) );

      writeAttribute(         file, "OBSERVATION_END_UTC",    timeStr(parset.stopTime()) );
      writeAttribute<double>( file, "OBSERVATION_END_MJD",    toMJD(parset.stopTime()) );
      writeAttribute(         file, "OBSERVATION_END_TAI",    toTAI(parset.stopTime()) );

      writeAttribute<int>( file, "OBSERVATION_NOF_STATIONS",  parset.nrStations() ); // TODO: SS beamformer?
      writeAttributeV( file, "OBSERVATION_STATIONS_LIST", parset.allStationNames() ); // TODO: SS beamformer?
#if 0
      // TODO: are subbands represented by their beginning, end, or middle frequency?

      std::vector<unsigned> subbands = parset.subbandList();
      unsigned max_subband = *std::max_element( subbands.begin(), subbands.end() );
      unsigned min_subband = *std::min_element( subbands.begin(), subbands.end() );
#endif
      //writeAttribute<double>( file, "OBSERVATION_FREQUENCY_MAX",    0.0 );
      //writeAttribute<double>( file, "OBSERVATION_FREQUENCY_MIN",    0.0 );
      //writeAttribute<double>( file, "OBSERVATION_FREQUENCY_CENTER", 0.0 );
      //writeAttribute( file, "OBSERVATION_FREQUENCY_UNIT", "MHz" );
      writeAttribute<int>( file, "OBSERVATION_NOF_BITS_PER_SAMPLE", parset.nrBitsPerSample() );

      writeAttribute<double>( file, "CLOCK_FREQUENCY", parset.clockSpeed() / 1e6 );
      writeAttribute( file, "CLOCK_FREQUENCY_UNIT", "MHz" );

      writeAttribute( file, "ANTENNA_SET",      parset.antennaSet() );
      writeAttribute( file, "FILTER_SELECTION", parset.getString("Observation.bandFilter") );
      //writeAttribute( file, "TARGET",           "" );

      //writeAttribute( file, "SYSTEM_VERSION",   "" );
      //writeAttribute( file, "PIPELINE_NAME",    "" );
      //writeAttribute( file, "PIPELINE_VERSION", "" );
      writeAttribute( file, "NOTES",            "" );

      // SysLog group -- empty for now

      {
        h5auto syslog(H5Gcreate2( file, "SysLog", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ), H5Gclose);
        ASSERT( syslog > 0 );
      }  

      // Information about the station beam (SAP)

      h5auto sap(H5Gcreate2( file, str(format("SUB_ARRAY_POINTING_%03u") % sapNr).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ), H5Gclose);
      ASSERT( sap > 0 );

      writeAttribute( sap, "GROUPTYPE",     "SubArrayPointing" );
      writeAttribute( sap, "NOF_STATIONS",  parset.nrStations() );
      writeAttributeV( sap, "STATIONS_LIST", parset.allStationNames() ); // TODO: SS beamformer?

      // TODO: non-J2000 pointings
      ASSERT( parset.getBeamDirectionType(sapNr) == "J2000" );
      std::vector<double> beamDir = parset.getBeamDirection(sapNr);
      writeAttribute<double>( sap, "POINT_RA",             beamDir[0] * 180.0 / M_PI );
      writeAttribute<double>( sap, "POINT_DEC",            beamDir[1] * 180.0 / M_PI );

      //writeAttribute(         sap, "POINT_ALTITUDE",       "" );
      //writeAttribute(         sap, "POINT_AZIMUTH",        "" );

      writeAttribute<double>( sap, "CLOCK_RATE",           parset.clockSpeed() / 1e6 );
      writeAttribute(         sap, "CLOCK_RATE_UNIT",      "MHz" );

      writeAttribute<int>(    sap, "NOF_SAMPLES",          itsNrSamples * nrBlocks );
      writeAttribute<double>( sap, "SAMPLING_RATE",        itsNrSamples / parset.CNintegrationTime() / 1e6 );
      writeAttribute(         sap, "SAMPLING_RATE_UNIT",   "MHz" );

      writeAttribute<int>(    sap, "CHANNELS_PER_SUBBAND", parset.nrChannelsPerSubband() );
      writeAttribute<double>( sap, "SUBBAND_WIDTH",        parset.clockSpeed() / 1e6 / 1024 );
      writeAttribute(         sap, "SUBBAND_WIDTH_UNIT",   "MHz" );
      writeAttribute<double>( sap, "CHANNEL_WIDTH",        parset.clockSpeed() / 1e6 / 1024 / parset.nrChannelsPerSubband() );
      writeAttribute(         sap, "CHANNEL_WIDTH_UNIT",   "MHz" );

      writeAttribute<int>(    sap, "NOF_BEAMS",            parset.nrPencilBeams(sapNr) );

      // Process History group -- empty for now
      {
        h5auto prochist(H5Gcreate2( sap, "ProcessHistory", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ), H5Gclose);
        ASSERT( prochist > 0 );
      }

      // Information about the pencil beam

      h5auto beam(H5Gcreate2( sap, str(format("BEAM_%03u") % beamNr).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ), H5Gclose);
      ASSERT( beam > 0 );

      writeAttribute(         beam, "GROUPTYPE",     "Beam" );
      //writeAttribute(       beam, "TARGET",        "" ); // TODO: vector of targets
      writeAttribute(         beam, "NOF_STATIONS",  parset.nrStations() );
      writeAttributeV(        beam, "STATIONS_LIST", parset.allStationNames() ); // TODO: SS beamformer?

      //const char *trackingTypes[] = { "J2000", "LMN", "TBD" };
      //writeAttribute(         beam, "TRACKING",      "J2000" ); // TODO: support non-tracking

      // TODO: non-J2000 pointings
      //ASSERT( parset.getBeamDirectionType() == "J2000" );
      BeamCoordinates pbeamDirs = parset.pencilBeams(sapNr);
      BeamCoord3D pbeamDir = pbeamDirs[beamNr];
      writeAttribute<double>( beam, "POINT_RA",      (beamDir[0] + pbeamDir[0]) * 180.0 / M_PI );
      writeAttribute<double>( beam, "POINT_DEC",     (beamDir[1] + pbeamDir[1]) * 180.0 / M_PI );
      writeAttribute<double>( beam, "POINT_OFFSET_RA",  pbeamDir[0] * 180.0 / M_PI );
      writeAttribute<double>( beam, "POINT_OFFSET_DEC", pbeamDir[1] * 180.0 / M_PI );

      //writeAttribute<double>( beam, "BEAM_DIAMETER_RA",      0.0 );
      //writeAttribute<double>( beam, "BEAM_DIAMETER_DEC",     0.0 );
      //writeAttribute<double>( beam, "BEAM_FREQUENCY_CENTER", 0.0 );
      //writeAttribute(         beam, "BEAM_FREQUENCY_CENTER_UNIT", "MHz" );

      double DM = parset.dispersionMeasure(sapNr, beamNr);

      writeAttribute<bool>(   beam, "FOLDED_DATA",      false );
      //writeAttribute<float>(  beam, "FOLD_PERIOD",      0.0 );
      //writeAttribute<float>(  beam, "FOLD_PERIOD_UNIT", "s" );

      writeAttribute<bool>(   beam, "DEDISPERSION",              DM == 0.0 ? "NONE" : "COHERENT" );
      writeAttribute<float>(  beam, "DEDISPERSION_MEASURE",      DM );
      writeAttribute(         beam, "DEDISPERSION_MEASURE_UNIT", "pc/cm^3" );

      //writeAttribute<bool>(   beam, "BARYCENTER",       false );

      writeAttribute<int>(   beam, "NOF_STOKES",       1 ); // we always write 1 stokes per file for now
      writeAttributeV(       beam, "STOKES_COMPONENTS", std::vector<std::string>( 1, stokes ) );
      writeAttribute<bool>(  beam, "COMPLEX_VOLTAGES", outputType == BEAM_FORMED_DATA );
      writeAttribute(        beam, "SIGNAL_SUM",       "COHERENT" );

      // Process History group -- empty for now
      {
        h5auto prochist(H5Gcreate2( beam, "ProcessHistory", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ), H5Gclose);
        ASSERT( prochist > 0 );
      }

      // Coordinates group -- empty for now
      {
        h5auto coordinates(H5Gcreate2( beam, "Coordinates", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT ), H5Gclose);
        ASSERT( coordinates > 0 );
      }

      // define the dimensions
      const int rank = 2;
      hsize_t dims[rank] = { itsNrSamples * nrBlocks, itsNrChannels };
      hsize_t maxdims[rank] = { H5S_UNLIMITED, itsNrChannels };

      h5auto filespace(H5Screate_simple( rank, dims, maxdims ), H5Sclose);
      ASSERT( filespace > 0 );

      // define the file writing strategy
      h5auto dcpl(H5Pcreate(H5P_DATASET_CREATE), H5Pclose);
      ASSERT( dcpl > 0 );
      ret = H5Pset_layout(dcpl, H5D_CONTIGUOUS);
      ASSERT( ret >= 0 );
      ret = H5Pset_external(dcpl, LOFAR::basename(rawfilename).c_str(), 0, H5F_UNLIMITED);
      ASSERT( ret >= 0 );

      // create the dataset
      h5auto stokesDataset(H5Dcreate2( beam, str(format("STOKES_%u") % stokesNr).c_str(), h5dataType<T>(isBigEndian), filespace, H5P_DEFAULT, dcpl,  H5P_DEFAULT ), H5Dclose);
      ASSERT( stokesDataset > 0 );

      writeAttribute( stokesDataset, "STOKES_COMPONENT", stokes );
      std::vector<int> nofChannels( parset.nrSubbandsPerPart(), parset.nrChannelsPerSubband() );
      writeAttributeV(     stokesDataset, "NOF_CHANNELS", nofChannels );
      writeAttribute<int>( stokesDataset, "NOF_SUBBANDS", parset.nrSubbandsPerPart() );
      writeAttribute<int>( stokesDataset, "NOF_SAMPLES",  itsNrSamples * nrBlocks );
    }

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::~MSWriterHDF5()
    {
    }

    template <typename T,unsigned DIM> void MSWriterHDF5<T,DIM>::write(StreamableData *data)
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
    template class MSWriterHDF5<float,3>;

    // specialisation for FinalBeamFormedData
    template class MSWriterHDF5<float,4>;
  } // namespace RTCP
} // namespace LOFAR

#else // no HAVE_HDF5

namespace LOFAR 
{

  namespace RTCP
  {

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::MSWriterHDF5 (const string &filename, const Parset &parset, OutputType outputType, unsigned fileno, bool isBigEndian)
    :
      MSWriterFile(filename,false)
    {
      LOG_ERROR_STR( "Using the HDF5 writer is not supported (file: " << filename << ")" );
    }

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::~MSWriterHDF5()
    {
    }

  } // namespace RTCP
} // namespace LOFAR

#endif
