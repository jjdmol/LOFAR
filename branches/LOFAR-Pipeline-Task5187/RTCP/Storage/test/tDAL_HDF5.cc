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

#include <Storage/MSWriterDAL.h>

#ifdef USE_DAL

#define FILENAME        "test.h5"
#define SAMPLES         3056
#define CHANNELS        256
#define SUBBANDS        10
#define BLOCKS          10

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

int main()
{
  const char * const filename = FILENAME;
  const unsigned nrSamples = SAMPLES;
  const unsigned nrChannels = SUBBANDS * CHANNELS;

  {
    CommonAttributes ca;
    Filename fn( "12345", "test", Filename::bf, Filename::h5, "");
    ca.setFilename( fn );

    ca.setClockFrequency( 200.0 );
    ca.setClockFrequencyUnit( "MHz" );   

    ca.setObserver( "" );
    ca.setObservationStart( "", "", "" );
    ca.setObservationEnd( "", "", "" );

    ca.setAntennaSet( "LBA_INNER" );
    ca.setFilterSelection( "LBA_30_90" );

    ca.setTarget( "" );
    ca.setSystemVersion( "" );
    ca.setPipelineVersion( "" );

    ca.setNotes( "" );

    ca.setTelescope( "LOFAR" );

    cout << "Creating file " << endl;
    BF_RootGroup rootGroup( fn );

    cout << "Creating stokes group 0" << endl;

    rootGroup.openStokesDataset( 0, 0, 0, nrSamples * BLOCKS, SUBBANDS, CHANNELS, Stokes::I );

    {
      /* don't -- this crashes */
      /*
      BF_StokesDataset stokesDataset = rootGroup.getSubArrayPointing( 0 ).getStokesDataset( 0, 0 );
      */

      std::string name = DAL::BF_SubArrayPointing::getName(0)
        + "/" + DAL::BF_BeamGroup::getName(0)
        + "/" + DAL::BF_StokesDataset::getName(0);
      BF_StokesDataset stokesDataset( rootGroup.locationID(), name );

      cout << "Creating sample multiarray of " << (SAMPLES|2) << " x " << SUBBANDS << " x " << CHANNELS << endl;
      typedef multi_array<float,3> array;

      array samples(extents[SAMPLES|2][SUBBANDS][CHANNELS]);

      for (unsigned t = 0; t < SAMPLES; t++)
        for (unsigned s = 0; s < SUBBANDS; s++)
          for (unsigned c = 0; c < CHANNELS; c++)
            samples[t][s][c] = t * s * c;


      for (unsigned seqnr = 0; seqnr < 1; seqnr++) {
        vector<int> start(2), block(2);

        start[0] = seqnr * nrSamples;
        start[1] = 0;

        block[0] = nrSamples;
        block[1] = nrChannels;

        cout << "Writing data block " << seqnr << endl;
        stokesDataset.writeData( samples.origin(), start, block );
      }
      cout << "Finalise StokesDataset" << endl;
    }  
    cout << "Finalise RootGroup" << endl;
  }
  cout << "Done" << endl;

  return 0;
}
#else
int main() {
  return 0;
}
#endif
