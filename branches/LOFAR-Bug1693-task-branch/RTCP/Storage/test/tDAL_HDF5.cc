//# tDAL_HDF5: Test DAL HDF5 routines
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

#if 1 && defined HAVE_DAL && defined HAVE_HDF5

#define FILENAME        "test.h5"
#define SAMPLES         3056
#define CHANNELS        256
#define SUBBANDS        10
#define BLOCKS          10

#include <hdf5.h>
#include <data_hl/BF_RootGroup.h>
#include <data_hl/BF_StokesDataset.h>
#include <data_common/CommonAttributes.h>
#include <iostream>

#include <data_common/CommonAttributes.h>

#include <boost/format.hpp>
#include <boost/multi_array.hpp>

using namespace DAL;
using namespace std;
using namespace boost;
using boost::format;

#endif


int main()
{
#if 1 && defined HAVE_DAL && defined HAVE_HDF5
  const char * const filename = FILENAME;
  const unsigned nrSamples = SAMPLES;
  const unsigned nrChannels = SUBBANDS * CHANNELS;

  {
    cout << "Creating file " << filename << endl;
    BF_RootGroup rootGroup( filename );

    cout << "Creating primary pointing 0" << endl;
    rootGroup.openPrimaryPointing( 0, true );

    BF_SubArrayPointing sap = rootGroup.primaryPointing( 0 );

    cout << "Creating tied-array beam 0" << endl;
    sap.openBeam( 0, true );

    cout << "Closing file" << endl;
  }

  {
    cout << "Reopening file " << filename << endl;
    hid_t fileID = H5Fcreate( filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );

    cout << "Creating stokes set 0" << endl;
    BF_StokesDataset stokesDataset(
        fileID, 0,
        nrSamples, SUBBANDS, CHANNELS,
        Stokes::I );

    cout << "Creating sample multiarray of " << (SAMPLES|2) << " x " << SUBBANDS << " x " << CHANNELS << endl;
    typedef multi_array<float,3> array;

    array samples(extents[SAMPLES|2][SUBBANDS][CHANNELS]);

    for (unsigned t = 0; t < SAMPLES; t++)
      for (unsigned s = 0; s < SUBBANDS; s++)
        for (unsigned c = 0; c < CHANNELS; c++)
          samples[t][s][c] = t * s * c;


    for (unsigned seqnr = 0; seqnr < BLOCKS; seqnr++) {
      vector<int> start(2), block(2);

      start[0] = 0;
      start[1] = seqnr * nrSamples;

      block[0] = nrChannels;
      block[1] = nrSamples;

      cout << "Writing data block " << seqnr << endl;
      stokesDataset.writeData( samples.origin(), start, block );
    }  
  }
#endif

  return 0;
}
