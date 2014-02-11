//# tMSWriterDAL: Test HDF5 routines through DAL
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <CoInterface/BeamFormedData.h>
#include <CoInterface/Parset.h>

#ifdef HAVE_DAL
#include <OutputProc/MSWriterDAL.h>
#endif

#include <cstring>

using namespace std;
using namespace LOFAR;
using namespace Cobalt;

int main()
{
#ifdef HAVE_DAL
  Parset parset("tMSWriterDAL.parset");

  {
    const size_t fileNo = 0;

    MSWriterDAL<float,3> writer("tMSWriterDAL_tmp.h5", parset, fileNo);

    const ObservationSettings::BeamFormer::File &file =
      parset.settings.beamFormer.files[fileNo];

    const ObservationSettings::BeamFormer::StokesSettings &sset =
      file.coherent
      ? parset.settings.beamFormer.coherentSettings
      : parset.settings.beamFormer.incoherentSettings;

    BeamFormedData data(
        sset.nrSamples,
        parset.settings.SAPs[file.sapNr].subbands.size(),
        sset.nrChannels);

    memset(data.samples.origin(), 0, data.samples.num_elements() * sizeof *data.samples.origin());

    writer.write(&data);

    // Dump feedback data to stdout
    cout << writer.configuration() << endl;
  }
#else
  cout << "Built without DAL, skipped actual test code." << endl;
#endif

  return 0;
}

