//# tParsetDefault.cc
//# Copyright (C) 2012-2014  ASTRON (Netherlands Institute for Radio Astronomy)
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

// Default RTCP parset creation for test programs.
// This is useful, because we verify a lot of keys when reading a parset and
// we don't want to create a passing key/value set for every test case separately.
//
// The unmodified default test key/value set created below is tested in tParset.cc.

#include <lofar_config.h>

#include <string>
#include <CoInterface/Parset.h>

using std::string;
using LOFAR::Cobalt::Parset;

Parset makeDefaultTestParset() {
  Parset ps;

  // Required keys to pass basic parset checks.
  ps.add("Observation.ObsID", "12345");
  // Use a valid station name (CS001) that is not in any tParset test,
  // so we don't have to remove board and slot list keys in some tests.
  ps.add("Observation.VirtualInstrument.stationList", "[CS001]");
  ps.add("Observation.antennaSet", "LBA_INNER");
  ps.add("Observation.bandFilter", "LBA_30_70");
  ps.add("Observation.nrBeams", "1");
  ps.add("Observation.Beam[0].subbandList", "[21..23]");
  ps.add("Observation.Dataslots.CS001LBA.RSPBoardList", "[3*0]");
  ps.add("Observation.Dataslots.CS001LBA.DataslotList", "[0..2]");

  ps.add("Observation.Beam[0].nrTiedArrayBeams", "1");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].coherent", "true");

  // for tests that use HBA
  ps.add("Observation.Dataslots.CS001HBA.RSPBoardList", "[3*0]");
  ps.add("Observation.Dataslots.CS001HBA.DataslotList", "[0..2]");
  ps.add("Observation.Dataslots.CS001HBA0.RSPBoardList", "[3*0]");
  ps.add("Observation.Dataslots.CS001HBA0.DataslotList", "[0..2]");
  ps.add("Observation.Dataslots.CS001HBA1.RSPBoardList", "[3*0]");
  ps.add("Observation.Dataslots.CS001HBA1.DataslotList", "[0..2]");

  // basic correlation output keys
  ps.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  ps.add("Observation.DataProducts.Output_Correlated.filenames",
         "[L12345_SAP000_SB000_uv.MS, L12345_SAP000_SB001_uv.MS, L12345_SAP000_SB002_uv.MS]");
  ps.add("Observation.DataProducts.Output_Correlated.locations",
         "[3*localhost:tParset-data/]");

  // basic beamforming output keys
  ps.add("Observation.DataProducts.Output_CoherentStokes.enabled", "true");
  ps.add("Cobalt.BeamFormer.CoherentStokes.which", "I");
  ps.add("Observation.DataProducts.Output_CoherentStokes.filenames",
         "[L12345_SAP000_B000_S000_P000_bf.h5]");
  ps.add("Observation.DataProducts.Output_CoherentStokes.locations",
         "[1*localhost:tParset-data/]");

  ps.updateSettings();

  return ps;
}

// Create a Parset out of one key/value pair in addition to
// minimally required key/value pairs.
Parset makeDefaultTestParset(const string& key, const string& value) {
  Parset ps = makeDefaultTestParset();

  // use replace() instead of set() in case one of the default keys is specified
  ps.replace(key, value);
  ps.updateSettings();

  return ps;
}

