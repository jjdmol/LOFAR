//# MSWriterDAL: an implementation of MSWriter using the DAL to weite HDF5
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

#include <Storage/MSWriter.h>
#include <Storage/MSWriterDAL.h>

#ifdef USE_DAL
#include <data_hl/BF_RootGroup.h>
#include <data_common/CommonAttributes.h>

#include <iostream>

#include <boost/format.hpp>
using namespace DAL;
using boost::format;

namespace LOFAR 
{

  namespace RTCP
  {

    template <typename T,unsigned DIM> MSWriterDAL<T,DIM>::MSWriterDAL (const char *filename, const Parset &parset, OutputType outputType, unsigned fileno, bool isBigEndian)
    :
      itsTransposeLogic( parset ),
      itsNrChannels(parset.nrChannelsPerSubband() * itsTransposeLogic.nrSubbands(fileno))
    {
      unsigned sapNr = 0;
      unsigned beamNr;
      unsigned stokesNr;
      Stokes::Component stokes;

      switch (outputType) {
        case COHERENT_STOKES: {
          // assume stokes are either I or IQUV
          const Stokes::Component stokesVars[] = { Stokes::I, Stokes::Q, Stokes::U, Stokes::V };
          stokesNr = fileno % parset.nrCoherentStokes();
          beamNr = fileno / parset.nrCoherentStokes() / parset.nrPartsPerStokes();

          itsDatatype = isBigEndian ? H5T_IEEE_F32BE : H5T_IEEE_F32LE;
          stokes = stokesVars[stokesNr];

          itsNrSamples = parset.CNintegrationSteps() / parset.coherentStokesTimeIntegrationFactor();
          break;
        }

        case BEAM_FORMED_DATA: {
          const Stokes::Component stokesVars2[] = { Stokes::X, Stokes::Y };
          const Stokes::Component stokesVars4[] = { Stokes::X, Stokes::X, Stokes::Y, Stokes::Y };
          stokesNr = fileno % parset.nrCoherentStokes();
          beamNr = fileno / parset.nrCoherentStokes() / parset.nrPartsPerStokes();

          // emulate fcomplex with a 64-bit bitfield
          itsDatatype = isBigEndian ? H5T_STD_B64BE : H5T_STD_B64LE;
          stokes = parset.nrCoherentStokes() == 2 ? stokesVars2[stokesNr] : stokesVars4[stokesNr];

          itsNrSamples = parset.CNintegrationSteps();
          break;
        } 

        default:
          THROW(StorageException, "MSWriterDAL can only handle Coherent Stokes and Beam-formed Data");
      }

      // set atttributes
      LOG_DEBUG_STR("MSWriterDAL: opening " << filename);

      CommonAttributes ca;
      /*
      CommonAttributesProject project;

      project.setProjectID( "" );
      project.setProjectTitle( parset.getString( "Observation.Campaign.title" ) );
      project.setProjectPI( parset.getString( "Observation.Campaign.PI" ) );
      project.setProjectCoI( parset.getString( "Observation.Campaign.CO_I" ) );
      project.setProjectContact( parset.getString( "Observation.Campaign.contact" ) );

      ca.setAttributesProject( project );
      */

      Filename fn(str(format("%u") % parset.observationID()), "test", Filename::bf, Filename::h5, "");

      ca.setFilename( fn );
#if 0      
      ca.setFiledate( "YYYY-MM-DDThh:mm:ss.s" ); // file creation date (now)
      ca.setTelescope( "LOFAR" );
      ca.setObserver( "" );
      ca.setObservationStart( "", "", "" );
      ca.setObservationEnd( "", "", "" );

      ca.setClockFrequency( parset.clockSpeed() / 1e6 );
      ca.setClockFrequencyUnit( "MHz" );

      ca.setAntennaSet( parset.antennaSet() );
      ca.setFilterSelection( parset.bandFilter() );

      ca.setTarget( "" );
      ca.setSystemVersion( "" );
      ca.setPipelineVersion( "" );

      ca.setNotes( "" );
#endif
      BF_RootGroup rootGroup( ca, DAL::IO_Mode(DAL::IO_Mode::Create) );

      rootGroup.openBeam( sapNr, beamNr );
      rootGroup.openStokesDataset( sapNr, beamNr, stokesNr, itsNrSamples, parset.nrSubbands(), parset.nrChannelsPerSubband(), stokes, itsDatatype );

      std::string name = DAL::BF_SubArrayPointing::getName(sapNr)
        + "/" + DAL::BF_BeamGroup::getName(beamNr) 
        + "/" + DAL::BF_StokesDataset::getName(stokesNr);

      itsStokesDataset = DAL::BF_StokesDataset(rootGroup.locationID(), name);
    }

    template <typename T,unsigned DIM> MSWriterDAL<T,DIM>::~MSWriterDAL()
    {
    }

    template <typename T,unsigned DIM> void MSWriterDAL<T,DIM>::write(StreamableData *data)
    {
      //ASSERT( dynamic_cast<SampleData<T,DIM> *>(data) );
      SampleData<T,DIM> *sdata = static_cast<SampleData<T,DIM> *>(data);

      vector<int> start(2);
      start[0] = data->sequenceNumber() * itsNrSamples;
      start[1] = 0;

      vector<int> block(2);
      block[0] = itsNrSamples;
      block[1] = itsNrChannels;

      LOG_DEBUG_STR( "HDF5: writing block " << data->sequenceNumber() << " of size " << block[0] << " x " << block[1] << " to coordinate " << start[0] << " x " << start[1]);

      itsStokesDataset.writeData( sdata->samples.origin(), start, block );
    }

    // specialisation for StokesData
    template class MSWriterDAL<float,3>;

    // specialisation for BeamFormedData
    template class MSWriterDAL<float,4>;

  } // namespace RTCP
} // namespace LOFAR

#else // no USE_DAL

namespace LOFAR 
{

  namespace RTCP
  {

    template <typename T,unsigned DIM> MSWriterDAL<T,DIM>::MSWriterDAL (const char *filename, const Parset&, OutputType, unsigned, bool)
    {
      LOG_ERROR_STR( "Using the DAL writer is not supported (file: " << filename << ")" );
    }

    template <typename T,unsigned DIM> MSWriterDAL<T,DIM>::~MSWriterDAL()
    {
    }

    template <typename T,unsigned DIM> void MSWriterDAL<T,DIM>::write(StreamableData *)
    {
    }

  } // namespace RTCP
} // namespace LOFAR

#endif
