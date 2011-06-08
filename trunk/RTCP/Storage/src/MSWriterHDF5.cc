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
#include <iostream>

#include <boost/format.hpp>
using boost::format;

// C->HDF5 translations of native types (Storage endianness)
template<typename T> hid_t h5nativeType();

template<> hid_t h5nativeType<float>()    { return H5T_NATIVE_FLOAT; }
template<> hid_t h5nativeType<unsigned>() { return H5T_NATIVE_UINT;  }
template<> hid_t h5nativeType<int>()      { return H5T_NATIVE_INT;   }

// C->HDF5 translations of types to use in header
template<typename T> hid_t h5writeType();

template<> hid_t h5writeType<float>()    { return H5T_IEEE_F32LE; }
template<> hid_t h5writeType<unsigned>() { return H5T_STD_U32LE;  }
template<> hid_t h5writeType<int>()      { return H5T_STD_I32LE;  }

// C->HDF5 translations of types to use for data (CNProc endianness)
template<typename T> hid_t h5dataType( bool bigEndian );

template<> hid_t h5dataType<float>( bool bigEndian ) {
  return bigEndian ? H5T_IEEE_F32BE : H5T_IEEE_F32LE;
}

template<> hid_t h5dataType<LOFAR::fcomplex>( bool bigEndian ) {
  // emulate fcomplex with a 64-bit bitfield
  return bigEndian ? H5T_STD_B64BE : H5T_STD_B64LE;
}

template<typename T> void writeAttribute( hid_t loc, const char *name, T value )
{
  hid_t ret;

  hid_t dataspace;
  dataspace = H5Screate( H5S_SCALAR );
  ASSERT( dataspace > 0 );

  hid_t attr;
  attr = H5Acreate2( loc, name, h5writeType<T>(), dataspace,  H5P_DEFAULT,  H5P_DEFAULT );
  ASSERT( attr > 0 );

  ret = H5Awrite( attr, h5nativeType<T>(), &value );
  ASSERT( ret >= 0 );

  H5Aclose( attr );

  H5Sclose( dataspace );
}

template<> void writeAttribute<char const *>( hid_t loc, const char *name, char const *value )
{
  hid_t ret;

  hid_t datatype;
  datatype = H5Tcopy( H5T_C_S1 );
  ASSERT( datatype > 0 );
  ret = H5Tset_size( datatype, H5T_VARIABLE );
  ASSERT( ret >= 0 );

  hid_t dataspace;
  dataspace = H5Screate( H5S_SCALAR );
  ASSERT( dataspace > 0 );

  hid_t attr;
  attr = H5Acreate2( loc, name, datatype, dataspace,  H5P_DEFAULT,  H5P_DEFAULT );
  ASSERT( attr > 0 );

  ret = H5Awrite( attr, datatype, &value );
  ASSERT( ret >= 0 );

  H5Aclose( attr );

  H5Tclose( datatype );
  H5Sclose( dataspace );
}

struct H5Group
{
  hid_t loc;

  H5Group( hid_t parent, const std::string &name ) {
    loc = H5Gcreate2( parent, name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
    ASSERT( loc > 0 );
  }

  ~H5Group() {
    hid_t ret = H5Gclose( loc );

    ASSERT( ret >= 0 );
  }
};

namespace LOFAR 
{

  namespace RTCP
  {

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::MSWriterHDF5 (const char *filename, const Parset &parset, OutputType outputType, unsigned fileno, bool isBigEndian)
    :
      MSWriterFile(str(format("%s.dat") % filename).c_str(),false),
      itsNrChannels(parset.nrChannelsPerSubband() * parset.nrSubbands()),
      itsNextSeqNr(0),
      itsDatatype(h5dataType<T>(isBigEndian))
    {
      unsigned sapNr = 0;
      unsigned beamNr;
      unsigned stokesNr;
      const char *stokes;

      switch (outputType) {
        case COHERENT_STOKES: {
          // assume stokes are either I or IQUV
          const char *stokesVars[] = { "I", "Q", "U", "V" };
          stokesNr = fileno % parset.nrCoherentStokes();
          beamNr = fileno / parset.nrCoherentStokes() / parset.nrPartsPerStokes();

          stokes = stokesVars[stokesNr];

          itsNrSamples = parset.CNintegrationSteps() / parset.coherentStokesTimeIntegrationFactor();
          break;
        }

        case BEAM_FORMED_DATA: {
          const char *stokesVars[] = { "X", "Y" };
          stokesNr = fileno % NR_POLARIZATIONS;
          beamNr = fileno / NR_POLARIZATIONS / parset.nrPartsPerStokes();

          stokes = stokesVars[stokesNr];

          itsNrSamples = parset.CNintegrationSteps();
          break;
        } 

        default:
          THROW(StorageException, "MSWriterHDF5 can only handle Coherent Stokes and Beam-formed Data");
      }

      itsZeroBlock.resize( itsNrSamples * itsNrChannels );

      LOG_DEBUG_STR("MSWriterHDF5: opening " << filename);

      hid_t ret;

      // create the top structure
      hid_t file = H5Fcreate( filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
      ASSERT( file > 0 );

      hid_t sap = H5Gcreate2( file, str(format("SubArrayPointing%03u") % sapNr).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      ASSERT( sap > 0 );

      hid_t beam = H5Gcreate2( sap, str(format("Beam%03u") % sapNr).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      ASSERT( beam > 0 );

      // define the dimensions
      const int rank = 2;
      unsigned nrBlocks = ceil((parset.stopTime() - parset.startTime()) / parset.CNintegrationTime());
      hsize_t dims[rank] = { itsNrSamples * nrBlocks, itsNrChannels };
      hsize_t maxdims[rank] = { H5S_UNLIMITED, itsNrChannels };

      hid_t filespace = H5Screate_simple( rank, dims, maxdims );
      ASSERT( filespace > 0 );

      // define the file writing strategy
      hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
      ASSERT( dcpl > 0 );
      ret = H5Pset_layout(dcpl, H5D_CONTIGUOUS);
      ASSERT( ret >= 0 );
      ret = H5Pset_external(dcpl, LOFAR::basename(str(format("%s.dat") % filename)).c_str(), 0, H5F_UNLIMITED);
      ASSERT( ret >= 0 );

      // create the dataset
      itsDataset = H5Dcreate2( beam, str(format("Stokes%u") % stokesNr).c_str(), itsDatatype, filespace, H5P_DEFAULT, dcpl,  H5P_DEFAULT );
      ASSERT( itsDataset > 0 );

      ret = H5Pclose(dcpl);
      ASSERT( ret >= 0 );

      ret = H5Sclose(filespace);
      ASSERT( ret >= 0 );

      writeAttribute( itsDataset, "STOKES_COMPONENT", stokes );
      writeAttribute<unsigned>( itsDataset, "NOF_CHANNELS", parset.nrChannelsPerSubband() );
      writeAttribute<unsigned>( itsDataset, "NOF_SUBBANDS", parset.nrSubbands() );

      ret = H5Dclose(itsDataset);
      ASSERT( ret >= 0 );

      ret = H5Gclose(beam);
      ASSERT( ret >= 0 );

      ret = H5Gclose(sap);
      ASSERT( ret >= 0 );

      ret = H5Fclose(file);
      ASSERT( ret >= 0 );
    }

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::~MSWriterHDF5()
    {
    }

    template <typename T,unsigned DIM> void MSWriterHDF5<T,DIM>::write(StreamableData *data)
    {
      unsigned seqNr = data->byteSwappedSequenceNumber();

      // fill in zeroes for lost blocks
      for (unsigned i = itsNextSeqNr; i < seqNr; i++)
        itsFile.write( &itsZeroBlock[0], itsZeroBlock.size() );

      data->write(&itsFile, false);
      itsNextSeqNr = seqNr + 1;
    }

    // specialisation for StokesData
    template class MSWriterHDF5<float,3>;

    // specialisation for BeamFormedData
    template class MSWriterHDF5<fcomplex,3>;

  } // namespace RTCP
} // namespace LOFAR

#else // no HAVE_HDF5

namespace LOFAR 
{

  namespace RTCP
  {

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::MSWriterHDF5 (const char *filename, const Parset&, OutputType, unsigned, bool)
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
