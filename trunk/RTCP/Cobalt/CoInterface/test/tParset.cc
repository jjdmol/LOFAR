//# tParset.cc
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

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>

#include <UnitTest++.h>
#include <vector>
#include <string>
#include <sstream>
#include <boost/format.hpp>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

// macros for testing true/false keys
#define TESTKEYS(new, old) for ( string k = "x", keystr = new; k != "xxx"; k += "x", keystr = old)
#define TESTBOOL for( unsigned val = 0; val < 2; ++val )
#define valstr ((val) ? "true" : "false")

// generate a vector of zeroes
vector<unsigned> zeroes(size_t n) {
  return vector<unsigned>(n, 0);
}

// generate a vector 0,1,2..n
vector<unsigned> sequence(size_t n) {
  vector<unsigned> result(n);

  for (size_t i = 0; i < n; ++i) {
    result[i] = i;
  }

  return result;
}

// convert a vector to a string
template<typename T> string toStr( const vector<T> &v )
{
  stringstream sstr;

  sstr << v;

  return sstr.str();
}

Parset makeDefaultTestParset() {
  Parset ps;

  // Required keys to pass basic parset checks.
  ps.add("Observation.ObsID", "12345");
  // Use a valid station name (CS001) that is not in any tests below,
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
  ps.add("Observation.DataProducts.Output_Correlated.locations", "[3*localhost:tParset-data/]");

  // basic beamforming output keys
  ps.add("Observation.DataProducts.Output_CoherentStokes.enabled", "true");
  ps.add("Cobalt.BeamFormer.CoherentStokes.which", "I");
  ps.add("Observation.DataProducts.Output_CoherentStokes.filenames", "[L12345_SAP000_B000_S000_P000_bf.h5]");
  ps.add("Observation.DataProducts.Output_CoherentStokes.locations", "[4*localhost:tParset-data/]");

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


// See if we pass basic Parset checks for the default test parset.
TEST(defaultTestParset) {
  Parset ps = makeDefaultTestParset();
}

/*
 * ===============================================
 * Test individual Parset fields through UnitTests
 * ===============================================
 */

/*
 * Test generic information.
 */

TEST(realTime) {
  TESTKEYS("Cobalt.realTime", "OLAP.realTime") {
    TESTBOOL {
      Parset ps = makeDefaultTestParset(keystr, valstr);

      CHECK_EQUAL(val, ps.settings.realTime);
      CHECK_EQUAL(val, ps.realTime());
    }
  }
}

TEST(observationID) {
  Parset ps = makeDefaultTestParset("Observation.ObsID", "12345");

  CHECK_EQUAL(12345U, ps.settings.observationID);
  CHECK_EQUAL(12345U, ps.observationID());
}

TEST(startTime) {
  Parset ps = makeDefaultTestParset("Observation.startTime", "2013-03-17 10:55:08");

  CHECK_CLOSE(1363517708.0, ps.settings.startTime, 0.1);
  CHECK_CLOSE(1363517708.0, ps.startTime(), 0.1);
}

TEST(stopTime) {
  Parset ps = makeDefaultTestParset("Observation.stopTime", "2013-03-17 10:55:08");

  CHECK_CLOSE(1363517708.0, ps.settings.stopTime, 0.1);
  CHECK_CLOSE(1363517708.0, ps.stopTime(), 0.1);
}

SUITE(clockMHz) {
  TEST(200) {
    Parset ps = makeDefaultTestParset("Observation.sampleClock", "200");

    CHECK_EQUAL(200U, ps.settings.clockMHz);
    CHECK_EQUAL(200000000U, ps.clockSpeed());

    CHECK_CLOSE(195312.5, ps.settings.subbandWidth(), 0.001);
    CHECK_CLOSE(195312.5, ps.subbandBandwidth(), 0.001);
    CHECK_CLOSE(1.0/195312.5, ps.sampleDuration(), 0.001);
  }

  TEST(160) {
    Parset ps = makeDefaultTestParset("Observation.sampleClock", "160");

    CHECK_EQUAL(160U, ps.settings.clockMHz);
    CHECK_EQUAL(160000000U, ps.clockSpeed());

    CHECK_CLOSE(156250.0, ps.settings.subbandWidth(), 0.001);
    CHECK_CLOSE(156250.0, ps.subbandBandwidth(), 0.001);
    CHECK_CLOSE(1.0/156250.0, ps.sampleDuration(), 0.001);
  }
}

SUITE(nrBitsPerSample) {
  TEST(16) {
    Parset ps = makeDefaultTestParset("Observation.nrBitsPerSample", "16");

    CHECK_EQUAL(16U, ps.settings.nrBitsPerSample);
    CHECK_EQUAL(16U, ps.nrBitsPerSample());
    CHECK_EQUAL(16U * 2 / 8, ps.nrBytesPerComplexSample());
  }

  TEST(8) {
    Parset ps = makeDefaultTestParset("Observation.nrBitsPerSample", "8");

    CHECK_EQUAL(8U, ps.settings.nrBitsPerSample);
    CHECK_EQUAL(8U, ps.nrBitsPerSample());
    CHECK_EQUAL(8U * 2 / 8, ps.nrBytesPerComplexSample());
  }

  TEST(4) {
    Parset ps = makeDefaultTestParset("Observation.nrBitsPerSample", "4");

    CHECK_EQUAL(4U, ps.settings.nrBitsPerSample);
    CHECK_EQUAL(4U, ps.nrBitsPerSample());
    CHECK_EQUAL(4U * 2 / 8, ps.nrBytesPerComplexSample());
  }
}

TEST(nrPolarisations) {
  size_t nPol = 2;

  Parset ps = makeDefaultTestParset("foo", "bar");

  CHECK_EQUAL(nPol,        ps.settings.nrPolarisations);
  CHECK_EQUAL(nPol * nPol, ps.settings.nrCrossPolarisations());
  CHECK_EQUAL(nPol * nPol, ps.nrCrossPolarisations());
}

SUITE(corrections) {
  TEST(bandPass) {
    TESTKEYS("Cobalt.correctBandPass", "OLAP.correctBandPass") {
      TESTBOOL {
        Parset ps = makeDefaultTestParset(keystr, valstr);

        CHECK_EQUAL(val, ps.settings.corrections.bandPass);
        CHECK_EQUAL(val, ps.correctBandPass());
      }
    }
  }

  TEST(clock) {
    TESTKEYS("Cobalt.correctClocks", "OLAP.correctClocks") {
      TESTBOOL {
        Parset ps = makeDefaultTestParset(keystr, valstr);

        CHECK_EQUAL(val, ps.settings.corrections.clock);
        CHECK_EQUAL(val, ps.correctClocks());
      }
    }
  }

  TEST(dedisperse) {
    TESTKEYS("Cobalt.BeamFormer.coherentDedisperseChannels", "OLAP.coherentDedisperseChannels") {
      TESTBOOL {
        Parset ps = makeDefaultTestParset(keystr, valstr);

        CHECK_EQUAL(val, ps.settings.corrections.dedisperse);
      }
    }
  }
}

SUITE(delayCompensation) {
  TEST(enabled) {
    TESTKEYS("Cobalt.delayCompensation", "OLAP.delayCompensation") {
      TESTBOOL {
        Parset ps = makeDefaultTestParset(keystr, valstr);

        CHECK_EQUAL(val, ps.settings.delayCompensation.enabled);
        CHECK_EQUAL(val, ps.delayCompensation());
      }
    }
  }

  TEST(referencePhaseCenter) {
      Parset ps = makeDefaultTestParset("Observation.referencePhaseCenter", "[1,2,3]");

      vector<double> refPhaseCenter(3);
      refPhaseCenter[0] = 1.0;
      refPhaseCenter[1] = 2.0;
      refPhaseCenter[2] = 3.0;

      CHECK_ARRAY_CLOSE(refPhaseCenter, ps.settings.delayCompensation.referencePhaseCenter, 3, 0.001);
  }
}

/*
 * Test station / antenna field information.
 */

TEST(antennaSetLBA) {
  vector<string> antennaSets;
  antennaSets.push_back("LBA_INNER");
  antennaSets.push_back("LBA_OUTER");
  for (vector<string>::iterator i = antennaSets.begin(); i != antennaSets.end(); ++i) {
    Parset ps = makeDefaultTestParset("Observation.antennaSet", *i);
    ps.replace("Observation.bandFilter", "LBA_30_70");

    CHECK_EQUAL(*i, ps.settings.antennaSet);
    CHECK_EQUAL(*i, ps.antennaSet());
  }
}

TEST(antennaSetHBA) {
  vector<string> antennaSets;
  antennaSets.push_back("HBA_ZERO");
  antennaSets.push_back("HBA_ONE");
  antennaSets.push_back("HBA_DUAL");
  antennaSets.push_back("HBA_JOINED");
  antennaSets.push_back("HBA_ZERO_INNER");
  antennaSets.push_back("HBA_ONE_INNER");
  antennaSets.push_back("HBA_DUAL_INNER");
  antennaSets.push_back("HBA_JOINED_INNER");

  for (vector<string>::iterator i = antennaSets.begin(); i != antennaSets.end(); ++i) {
    Parset ps = makeDefaultTestParset("Observation.antennaSet", *i);
    ps.replace("Observation.bandFilter", "HBA_110_190");

    CHECK_EQUAL(*i, ps.settings.antennaSet);
    CHECK_EQUAL(*i, ps.antennaSet());
  }
}

TEST(bandFilter) {
  // bandFilter[filter] = nyquistZone
  map<string, unsigned> bandFilters;
  bandFilters["LBA_10_90"]   = 1;
  bandFilters["LBA_30_90"]   = 1;
  bandFilters["HBA_110_190"] = 2;
  bandFilters["HBA_170_230"] = 3;
  bandFilters["HBA_210_250"] = 3;

  for (map<string, unsigned>::iterator i = bandFilters.begin(); i != bandFilters.end(); ++i) {
    Parset ps = makeDefaultTestParset("Observation.bandFilter", i->first);

    CHECK_EQUAL(i->first, ps.settings.bandFilter);
    CHECK_EQUAL(i->first, ps.bandFilter());
    
    CHECK_EQUAL(i->second, ps.settings.nyquistZone());
  }
}

SUITE(antennaFieldNames) {
  TEST(LBA) {
    vector<string> stations, expectedFields;
    stations.push_back("CS002");
    expectedFields.push_back("CS002LBA");
    stations.push_back("CS003");
    expectedFields.push_back("CS003LBA");
    stations.push_back("RS210");
    expectedFields.push_back("RS210LBA");
    stations.push_back("DE603");
    expectedFields.push_back("DE603LBA");

    vector<ObservationSettings::AntennaFieldName> antennaFieldNames = ObservationSettings::antennaFieldNames(stations, "LBA_INNER");

    CHECK_EQUAL(expectedFields.size(), antennaFieldNames.size());

    for (size_t i = 0; i < std::min(expectedFields.size(), antennaFieldNames.size()); ++i) {
      CHECK_EQUAL(expectedFields[i], antennaFieldNames[i].fullName());
    }
  }

  TEST(HBA0) {
    vector<string> stations, expectedFields;
    stations.push_back("CS002");
    expectedFields.push_back("CS002HBA0");
    stations.push_back("CS003");
    expectedFields.push_back("CS003HBA0");
    stations.push_back("RS210");
    expectedFields.push_back("RS210HBA");
    stations.push_back("DE603");
    expectedFields.push_back("DE603HBA");

    vector<ObservationSettings::AntennaFieldName> antennaFieldNames = ObservationSettings::antennaFieldNames(stations, "HBA_ZERO");

    CHECK_EQUAL(expectedFields.size(), antennaFieldNames.size());

    for (size_t i = 0; i < std::min(expectedFields.size(), antennaFieldNames.size()); ++i) {
      CHECK_EQUAL(expectedFields[i], antennaFieldNames[i].fullName());
    }
  }

  TEST(HBA1) {
    vector<string> stations, expectedFields;
    stations.push_back("CS002");
    expectedFields.push_back("CS002HBA1");
    stations.push_back("CS003");
    expectedFields.push_back("CS003HBA1");
    stations.push_back("RS210");
    expectedFields.push_back("RS210HBA");
    stations.push_back("DE603");
    expectedFields.push_back("DE603HBA");

    vector<ObservationSettings::AntennaFieldName> antennaFieldNames = ObservationSettings::antennaFieldNames(stations, "HBA_ONE");

    CHECK_EQUAL(expectedFields.size(), antennaFieldNames.size());

    for (size_t i = 0; i < std::min(expectedFields.size(), antennaFieldNames.size()); ++i) {
      CHECK_EQUAL(expectedFields[i], antennaFieldNames[i].fullName());
    }
  }

  TEST(HBA_DUAL) {
    vector<string> stations, expectedFields;
    stations.push_back("CS002");
    expectedFields.push_back("CS002HBA0");
    expectedFields.push_back("CS002HBA1");
    stations.push_back("CS003");
    expectedFields.push_back("CS003HBA0");
    expectedFields.push_back("CS003HBA1");
    stations.push_back("RS210");
    expectedFields.push_back("RS210HBA");
    stations.push_back("DE603");
    expectedFields.push_back("DE603HBA");

    vector<ObservationSettings::AntennaFieldName> antennaFieldNames = ObservationSettings::antennaFieldNames(stations, "HBA_DUAL");

    CHECK_EQUAL(expectedFields.size(), antennaFieldNames.size());

    for (size_t i = 0; i < std::min(expectedFields.size(), antennaFieldNames.size()); ++i) {
      CHECK_EQUAL(expectedFields[i], antennaFieldNames[i].fullName());
    }
  }

  TEST(HBA_JOINED) {
    vector<string> stations, expectedFields;
    stations.push_back("CS002");
    expectedFields.push_back("CS002HBA");
    stations.push_back("CS003");
    expectedFields.push_back("CS003HBA");
    stations.push_back("RS210");
    expectedFields.push_back("RS210HBA");
    stations.push_back("DE603");
    expectedFields.push_back("DE603HBA");

    vector<ObservationSettings::AntennaFieldName> antennaFieldNames = ObservationSettings::antennaFieldNames(stations, "HBA_JOINED");

    CHECK_EQUAL(expectedFields.size(), antennaFieldNames.size());

    for (size_t i = 0; i < std::min(expectedFields.size(), antennaFieldNames.size()); ++i) {
      CHECK_EQUAL(expectedFields[i], antennaFieldNames[i].fullName());
    }
  }
}

SUITE(stations) {
  TEST(phaseCenter) {
    Parset ps = makeDefaultTestParset();

    // set
    ps.replace("Observation.antennaSet", "LBA_INNER");
    ps.replace("PIC.Core.CS001LBA.phaseCenter", "[1.0, 2.0, 3.0]");
    ps.updateSettings();

    // verify settings
    CHECK_EQUAL(3U,  ps.settings.antennaFields[0].phaseCenter.size());
    CHECK_CLOSE(1.0, ps.settings.antennaFields[0].phaseCenter[0], 0.01);
    CHECK_CLOSE(2.0, ps.settings.antennaFields[0].phaseCenter[1], 0.01);
    CHECK_CLOSE(3.0, ps.settings.antennaFields[0].phaseCenter[2], 0.01);
  }

  TEST(default_map) {
    Parset ps = makeDefaultTestParset();

    // add stations and default board/slot lists
    ps.replace("Observation.VirtualInstrument.stationList", "[CS002, CS003, RS210]");
    ps.replace("Observation.antennaSet", "LBA_INNER");
    ps.replace("Observation.nrBeams", "1");
    ps.replace("Observation.Beam[0].subbandList", "[20..29]");
    ps.replace("Observation.rspBoardList", "[3*0, 3*1, 3*2, 1*3]");
    ps.replace("Observation.rspSlotList",  "[0..2, 0..2, 0..2, 0]");
    // turn off to avoid setting even more keys (fails with even stricter Parset checks)
    ps.replace("Observation.DataProducts.Output_Correlated.enabled", "false");
    ps.updateSettings();

    // verify settings
    CHECK_EQUAL(3U , ps.settings.antennaFields.size());
    CHECK_EQUAL(10U, ps.settings.antennaFields[0].rspBoardMap.size());
    CHECK_EQUAL(0U , ps.settings.antennaFields[0].rspBoardMap[0]);
    CHECK_EQUAL(10U, ps.settings.antennaFields[0].rspSlotMap.size());
    CHECK_EQUAL(0U , ps.settings.antennaFields[0].rspSlotMap[0]);
  }

  TEST(station_map) {
    Parset ps = makeDefaultTestParset();

    // add stations and station-specific board/slot lists
    ps.replace("Observation.VirtualInstrument.stationList", "[CS002, CS003]");
    ps.replace("Observation.antennaSet", "LBA_INNER");
    ps.replace("Observation.nrBeams", "1");
    ps.replace("Observation.Beam[0].subbandList", "[20..35]");
    // try something not necessarily most straight forward (also as an example):
    ps.replace("Observation.Dataslots.CS002LBA.RSPBoardList", "[1*0, 6*1, 5*2, 4*3]");
    ps.replace("Observation.Dataslots.CS002LBA.DataslotList", "[0, 0..5, 0..4, 0..3]");
    ps.replace("Observation.Dataslots.CS003LBA.RSPBoardList", "[3*0, 2*1, 9*2, 2*3]");
    ps.replace("Observation.Dataslots.CS003LBA.DataslotList", "[0..2, 0..1, 0..8, 0..1]");
    // turn off to avoid setting even more keys (fails with even stricter Parset checks)
    ps.replace("Observation.DataProducts.Output_Correlated.enabled", "false");
    ps.updateSettings();

    // verify settings
    CHECK_EQUAL(2U , ps.settings.antennaFields.size());

    CHECK_EQUAL(16U, ps.settings.antennaFields[0].rspBoardMap.size());
    CHECK_EQUAL(0U , ps.settings.antennaFields[0].rspBoardMap[0]);
    CHECK_EQUAL(16U, ps.settings.antennaFields[0].rspSlotMap.size());
    CHECK_EQUAL(0U , ps.settings.antennaFields[0].rspSlotMap[0]);

    CHECK_EQUAL(16U, ps.settings.antennaFields[1].rspBoardMap.size());
    CHECK_EQUAL(3U , ps.settings.antennaFields[1].rspBoardMap[15]);
    CHECK_EQUAL(16U, ps.settings.antennaFields[1].rspSlotMap.size());
    CHECK_EQUAL(1U , ps.settings.antennaFields[1].rspSlotMap[15]);
  }
}

SUITE(StationStreams) {


  TEST(restrictNodes) {
    // optional key Cobalt.restrictNodesToStationStreams

    // By default, all (half-)nodes must be used, because we may need them for
    // computations, even if not all receive input. We don't have a perf model.
    Parset ps = makeDefaultTestParset();

    ps.replace("Observation.antennaSet", "HBA_DUAL_INNER");
    ps.replace("Observation.bandFilter", "HBA_110_190");

    const unsigned nrNodes = 4; // in this test, twice the half-nodes
    ps.replace("Cobalt.Nodes", "[node01_0, node01_1, node02_0, node02_1, node03_0, node03_1, node04_0, node04_1]");
    ps.replace("PIC.Core.Cobalt.node01_0.host", "node01_0");
    ps.replace("PIC.Core.Cobalt.node01_0.cpu", "0");
    ps.replace("PIC.Core.Cobalt.node01_0.nic", "mlx4_0");
    ps.replace("PIC.Core.Cobalt.node01_0.gpus", "[0, 1]");
    ps.replace("PIC.Core.Cobalt.node01_1.host", "node01_1");
    ps.replace("PIC.Core.Cobalt.node01_1.cpu", "1");
    ps.replace("PIC.Core.Cobalt.node01_1.nic", "mlx4_1");
    ps.replace("PIC.Core.Cobalt.node01_1.gpus", "[2, 3]");
    ps.replace("PIC.Core.Cobalt.node02_0.host", "node02_0");
    ps.replace("PIC.Core.Cobalt.node02_0.cpu", "0");
    ps.replace("PIC.Core.Cobalt.node02_0.nic", "mlx4_0");
    ps.replace("PIC.Core.Cobalt.node02_0.gpus", "[0, 1]");
    ps.replace("PIC.Core.Cobalt.node02_1.host", "node02_1");
    ps.replace("PIC.Core.Cobalt.node02_1.cpu", "1");
    ps.replace("PIC.Core.Cobalt.node02_1.nic", "mlx4_1");
    ps.replace("PIC.Core.Cobalt.node02_1.gpus", "[2, 3]");
    ps.replace("PIC.Core.Cobalt.node03_0.host", "node03_0");
    ps.replace("PIC.Core.Cobalt.node03_0.cpu", "0");
    ps.replace("PIC.Core.Cobalt.node03_0.nic", "mlx4_0");
    ps.replace("PIC.Core.Cobalt.node03_0.gpus", "[0, 1]");
    ps.replace("PIC.Core.Cobalt.node03_1.host", "node03_1");
    ps.replace("PIC.Core.Cobalt.node03_1.cpu", "1");
    ps.replace("PIC.Core.Cobalt.node03_1.nic", "mlx4_1");
    ps.replace("PIC.Core.Cobalt.node03_1.gpus", "[2, 3]");
    ps.replace("PIC.Core.Cobalt.node04_0.host", "node04_0");
    ps.replace("PIC.Core.Cobalt.node04_0.cpu", "0");
    ps.replace("PIC.Core.Cobalt.node04_0.nic", "mlx4_0");
    ps.replace("PIC.Core.Cobalt.node04_0.gpus", "[0, 1]");
    ps.replace("PIC.Core.Cobalt.node04_1.host", "node04_1");
    ps.replace("PIC.Core.Cobalt.node04_1.cpu", "1");
    ps.replace("PIC.Core.Cobalt.node04_1.nic", "mlx4_1");
    ps.replace("PIC.Core.Cobalt.node04_1.gpus", "[2, 3]");
    ps.updateSettings();

    CHECK_EQUAL(2 * nrNodes, ps.settings.nodes.size());


    ps.replace("Cobalt.restrictNodesToStationStreams", "true");
    ps.updateSettings();

    // no stream connections defined, no nodes have input, so need 0 nodes
    CHECK_EQUAL(0u, ps.settings.nodes.size());


    ps.replace("PIC.Core.CS001HBA0.RSP.ports", "[udp:node02-10GB01:10010, udp:node02-10GB01:10011, udp:node02-10GB01:10012, udp:node02-10GB01:10013]");
    ps.replace("PIC.Core.CS001HBA0.RSP.receiver", "node02_0");
    ps.replace("PIC.Core.CS001HBA1.RSP.ports", "[udp:node04-10GB01:10016, udp:node04-10GB01:10017, udp:node04-10GB01:10018, udp:node04-10GB01:10019]");
    ps.replace("PIC.Core.CS001HBA1.RSP.receiver", "node04_1");
    // add some irrelevant streams (see if it takes ant set into account)
    ps.replace("PIC.Core.CS001HBA.RSP.ports", "[udp:node01-10GB01:10010, udp:node01-10GB01:10011, udp:node01-10GB01:10012, udp:node01-10GB01:10013]");
    ps.replace("PIC.Core.CS001HBA.RSP.receiver", "node01_1");
    ps.replace("PIC.Core.CS001LBA.RSP.ports", "[udp:node03-10GB01:10010, udp:node03-10GB01:10011, udp:node03-10GB01:10012, udp:node03-10GB01:10013]");
    ps.replace("PIC.Core.CS001LBA.RSP.receiver", "node03_0");
    ps.updateSettings();

    // The 2 nodes connected to the 2 ant fields must be the only ones.
    CHECK_EQUAL(2u, ps.settings.nodes.size());
    string name1 = ps.settings.nodes[0].name;
    string name2 = ps.settings.nodes[1].name;
    // verify order independent
    if (name1 == "node02_0")
      CHECK_EQUAL("node04_1", name2);
    else if (name1 == "node04_1")
      CHECK_EQUAL("node02_0", name2);
    else // wrong wrong wrong. Ensure both are printed.
      CHECK_EQUAL(name1 + "XXX", name2 + "YYY");


    // switch off again to have all nodes
    ps.replace("Cobalt.restrictNodesToStationStreams", "false");
    ps.updateSettings();

    CHECK_EQUAL(2 * nrNodes, ps.settings.nodes.size());
  }
}

SUITE(SAPs) {

  TEST(nr) {
    Parset ps = makeDefaultTestParset();
    for (size_t nrSAPs = 1; nrSAPs < 244; ++nrSAPs) {
      ps.replace("Observation.nrBeams", str(format("%u") % nrSAPs));
      ps.replace(str( format("Observation.Beam[%u].subbandList") % (nrSAPs-1) ), "[0]");
      ps.replace("Observation.Dataslots.CS001LBA.RSPBoardList", str(format("[%u*0]") % nrSAPs));
      ps.replace("Observation.Dataslots.CS001LBA.DataslotList", str(format("[0..%u]") % (nrSAPs-1)));
      // turn off to avoid setting even more keys (fails with even stricter Parset checks)
      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "false");
      ps.updateSettings();

      CHECK_EQUAL(nrSAPs, ps.settings.SAPs.size());
    }
  }

  TEST(target) {
    Parset ps = makeDefaultTestParset();

    // set
    ps.replace("Observation.nrBeams", "2");
    ps.replace("Observation.Beam[0].subbandList", "[0..2]");
    ps.replace("Observation.Beam[1].subbandList", "[0]");
    ps.replace("Observation.Beam[0].target", "target 1");
    ps.replace("Observation.Beam[1].target", "target 2");
    ps.replace("Observation.Dataslots.CS001LBA.RSPBoardList", "[4*0]");
    ps.replace("Observation.Dataslots.CS001LBA.DataslotList", "[0..3]");
    // turn off to avoid setting even more keys (fails with even stricter Parset checks)
    ps.replace("Observation.DataProducts.Output_Correlated.enabled", "false");
    ps.updateSettings();

    // verify settings
    CHECK_EQUAL("target 1", ps.settings.SAPs[0].target);
    CHECK_EQUAL("target 2", ps.settings.SAPs[1].target);
  }

  TEST(direction) {
    Parset ps = makeDefaultTestParset();

    // set
    ps.replace("Observation.nrBeams", "1");
    ps.replace("Observation.Beam[0].angle1", "1.0");
    ps.replace("Observation.Beam[0].angle2", "2.0");
    ps.replace("Observation.Beam[0].directionType", "AZEL");

    ps.updateSettings();

    // verify settings
    CHECK_CLOSE(1.0,    ps.settings.SAPs[0].direction.angle1, 0.1);
    CHECK_CLOSE(2.0,    ps.settings.SAPs[0].direction.angle2, 0.1);
    CHECK_EQUAL("AZEL", ps.settings.SAPs[0].direction.type);
  }
}

SUITE(anaBeam) {
  TEST(enabled) {
    TESTBOOL {
      Parset ps = makeDefaultTestParset("Observation.antennaSet", val ? "HBA_ZERO" : "LBA_INNER");

      CHECK_EQUAL(val, ps.settings.anaBeam.enabled);
    }
  }

  TEST(direction) {
    Parset ps = makeDefaultTestParset();

    // set
    ps.replace("Observation.antennaSet", "HBA_ZERO_INNER");
    ps.replace("Observation.AnaBeam[0].angle1", "1.0");
    ps.replace("Observation.AnaBeam[0].angle2", "2.0");
    ps.replace("Observation.AnaBeam[0].directionType", "AZEL");

    ps.updateSettings();

    // verify settings
    CHECK_CLOSE(1.0,    ps.settings.anaBeam.direction.angle1, 0.1);
    CHECK_CLOSE(2.0,    ps.settings.anaBeam.direction.angle2, 0.1);
    CHECK_EQUAL("AZEL", ps.settings.anaBeam.direction.type);
  }
}

SUITE(subbands) {
  TEST(nr) {
    for (size_t nrSubbands = 1; nrSubbands <= 244; ++nrSubbands) {
      Parset ps = makeDefaultTestParset();

      // add subbands
      ps.replace("Observation.nrBeams", "1");
      ps.replace("Observation.Beam[0].subbandList", str(format("[%u*42]") % nrSubbands));
      ps.replace("Observation.Dataslots.CS001LBA.RSPBoardList", str(format("[%u*0]") % nrSubbands));
      ps.replace("Observation.Dataslots.CS001LBA.DataslotList", str(format("[0..%u]") % (nrSubbands-1)));
      // turn off to avoid setting even more keys (fails with even stricter Parset checks)
      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "false");
      ps.updateSettings();

      // verify settings
      CHECK_EQUAL(nrSubbands, ps.settings.subbands.size());
    }
  }

  TEST(idx_stationIdx) {
    Parset ps = makeDefaultTestParset();

    // set
    ps.replace("Observation.nrBeams", "1");
    ps.replace("Observation.Beam[0].subbandList", "[42]");
    ps.updateSettings();

    // verify settings
    CHECK_EQUAL(0U,  ps.settings.subbands[0].idx);
    CHECK_EQUAL(42U, ps.settings.subbands[0].stationIdx);
  }

  TEST(SAP) {
    Parset ps = makeDefaultTestParset();

    // set -- note: for now, omitting actual SAP specifications is allowed
    ps.replace("Observation.nrBeams", "2");
    ps.replace("Observation.Beam[1].subbandList", "[1]");
    // 3 for beam 0 (default test parset) + 1 for beam 1
    ps.replace("Observation.Dataslots.CS001LBA.RSPBoardList", "[4*0]");
    ps.replace("Observation.Dataslots.CS001LBA.DataslotList", "[0..3]");
    // turn off to avoid setting even more keys (fails with even stricter Parset checks)
    ps.replace("Observation.DataProducts.Output_Correlated.enabled", "false");
    ps.updateSettings();

    // verify settings: newly added sb for beam 1 must indeed have SAP 1
    CHECK_EQUAL(1U, ps.settings.subbands[3].SAP);
  }

  TEST(centralFrequency) {
    // test for both 200 and 160 MHz clocks,
    // and for all three Nyquist zones.

    map<unsigned, string> bandFilters;
    bandFilters[1] = "LBA_10_90";
    bandFilters[2] = "HBA_110_190";
    bandFilters[3] = "HBA_170_230";

    for (unsigned clocks = 0; clocks < 2; ++clocks) {
      unsigned clock = clocks == 0 ? 200 : 160;

      for (unsigned zones = 0; zones < 3; ++zones) {
        unsigned nyquistZone = zones + 1;

        Parset ps = makeDefaultTestParset();

        // set
        ps.replace("Observation.sampleClock", str(format("%u") % clock));
        ps.replace("Observation.bandFilter",  bandFilters[nyquistZone]);
        ps.replace("Observation.nrBeams",     "1");
        ps.replace("Observation.Beam[0].subbandList", "[0..511]");
        ps.replace("Observation.Dataslots.CS001LBA.RSPBoardList", "[512*0]");
        ps.replace("Observation.Dataslots.CS001LBA.DataslotList", "[0..511]");
        // turn off to avoid setting even more keys (fails with even stricter Parset checks)
        ps.replace("Observation.DataProducts.Output_Correlated.enabled", "false");
        ps.updateSettings();

        // verify settings
        for (unsigned sb = 0; sb < 512; ++sb) {
          CHECK_CLOSE(ps.settings.subbandWidth() * (512 * (nyquistZone - 1) + sb), ps.settings.subbands[sb].centralFrequency, 0.001);
        }

        // override
        ps.replace("Observation.Beam[0].frequencyList", "[1..512]");
        ps.updateSettings();

        // verify settings
        for (unsigned sb = 0; sb < 512; ++sb) {
          CHECK_CLOSE(sb + 1.0, ps.settings.subbands[sb].centralFrequency, 0.001);
        }
      }
    }
  }
}

/*
 * Test correlator pipeline settings.
 */

SUITE(correlator) {
  TEST(enabled) {
    TESTBOOL {
      Parset ps = makeDefaultTestParset("Observation.DataProducts.Output_Correlated.enabled", valstr);

      CHECK_EQUAL(val, ps.settings.correlator.enabled);
    }
  }

  TEST(nrChannels) {
    // for now, nrChannels is also defined if the correlator is disabled
    TESTKEYS("Cobalt.Correlator.nrChannelsPerSubband", "Observation.channelsPerSubband") {
      Parset ps = makeDefaultTestParset();

      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
      ps.replace(keystr, "256");
      ps.updateSettings();

      CHECK_EQUAL(256U, ps.settings.correlator.nrChannels);
      CHECK_EQUAL(256U, ps.nrChannelsPerSubband());
    }
  }

  TEST(channelWidth) {
    // validate all powers of 2 in [1, 4096]
    for (size_t nrChannels = 1; nrChannels <= 4096; nrChannels <<= 1) {
      Parset ps = makeDefaultTestParset();

      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
      ps.replace("Observation.channelsPerSubband", str(format("%u") % nrChannels));
      ps.updateSettings();

      CHECK_CLOSE(ps.settings.subbandWidth() / nrChannels, ps.settings.correlator.channelWidth, 0.00001);
      CHECK_CLOSE(ps.settings.subbandWidth() / nrChannels, ps.channelWidth(), 0.00001);
    }
  }

  TEST(nrSamplesPerChannel) {
    TESTKEYS("Cobalt.Correlator.nrChannelsPerSubband", "Observation.nrChannelsPerSubband") {
      Parset ps = makeDefaultTestParset();
      
      // set
      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
      ps.replace("Cobalt.blockSize", "256");
      ps.replace(keystr, "64");
      ps.updateSettings();

      // verify settings
      CHECK_EQUAL(4U, ps.settings.correlator.nrSamplesPerChannel);
      CHECK_EQUAL(4U, ps.CNintegrationSteps());
      CHECK_EQUAL(4U, ps.nrSamplesPerChannel());
    }
  }

  TEST(nrBlocksPerIntegration) {
    TESTKEYS("Cobalt.Correlator.nrBlocksPerIntegration", "OLAP.IONProc.integrationSteps") {
      Parset ps = makeDefaultTestParset();
      
      // set
      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
      ps.replace(keystr, "42");
      ps.updateSettings();

      // verify settings
      CHECK_EQUAL(42U, ps.settings.correlator.nrBlocksPerIntegration);
      CHECK_EQUAL(42U, ps.IONintegrationSteps());
    }
  }

  /* TODO: test super-station beam former */

  SUITE(files) {
    TEST(filenames_mandatory) {
      Parset ps = makeDefaultTestParset();
      
      // set
      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
      ps.replace("Observation.nrBeams",             "1");
      ps.replace("Observation.Beam[0].subbandList", "[0]");
      ps.remove ("Observation.DataProducts.Output_Correlated.filenames");
      ps.replace("Observation.DataProducts.Output_Correlated.locations", "[localhost:.]");

      // forget filenames == throw
      CHECK_THROW(ps.updateSettings(), CoInterfaceException);

      // add filenames
      ps.replace("Observation.DataProducts.Output_Correlated.filenames", "[SB000.MS]");

      // should be OK now
      ps.updateSettings();
    }

    TEST(locations_mandatory) {
      Parset ps = makeDefaultTestParset();
      
      // set
      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
      ps.replace("Observation.nrBeams",             "1");
      ps.replace("Observation.Beam[0].subbandList", "[0]");
      ps.replace("Observation.DataProducts.Output_Correlated.filenames", "[SB000.MS]");
      ps.remove ("Observation.DataProducts.Output_Correlated.locations");

      // forget locations == throw
      CHECK_THROW(ps.updateSettings(), CoInterfaceException);

      // add locations
      ps.replace("Observation.DataProducts.Output_Correlated.locations", "[localhost:.]");

      // should be OK now
      ps.updateSettings();
    }

    TEST(nr) {
      // this test is expensive, so select a few values to test
      vector<size_t> testNrSubbands;
      testNrSubbands.push_back(1);
      testNrSubbands.push_back(2);
      testNrSubbands.push_back(61);
      testNrSubbands.push_back(122);
      testNrSubbands.push_back(244);
      testNrSubbands.push_back(488);

      for (size_t i = 0; i < testNrSubbands.size(); ++i) {
        size_t nrSubbands = testNrSubbands[i];
        Parset ps = makeDefaultTestParset();

        // set
        ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
        ps.replace("Observation.nrBeams", "1");
        ps.replace("Observation.Beam[0].subbandList", str(format("[%u*42]") % nrSubbands));
        ps.replace("Observation.Dataslots.CS001LBA.RSPBoardList", str(format("[%u*0]") % nrSubbands));
        ps.replace("Observation.Dataslots.CS001LBA.DataslotList", str(format("[0..%u]") % (nrSubbands-1)));
        ps.replace("Observation.DataProducts.Output_Correlated.filenames", str(format("[%u*SBxxx.MS]") % nrSubbands));
        ps.replace("Observation.DataProducts.Output_Correlated.locations", str(format("[%u*localhost:.]") % nrSubbands));
        ps.updateSettings();

        // verify settings
        CHECK_EQUAL(nrSubbands, ps.settings.correlator.files.size());
      }
    }

    TEST(location) {
      Parset ps = makeDefaultTestParset();

      // set
      ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
      ps.replace("Observation.nrBeams", "1");
      ps.replace("Observation.Beam[0].subbandList", "[0]");
      ps.replace("Observation.DataProducts.Output_Correlated.filenames", "[SB000.MS]");
      ps.replace("Observation.DataProducts.Output_Correlated.locations", "[host:/dir]");
      ps.updateSettings();

      // verify settings
      CHECK_EQUAL("SB000.MS", ps.settings.correlator.files[0].location.filename);
      CHECK_EQUAL("host",     ps.settings.correlator.files[0].location.host);
      CHECK_EQUAL("/dir",     ps.settings.correlator.files[0].location.directory);
    }
  }
}


/*
 * TODO: Test other beam former pipeline settings too.
 */

SUITE(beamformer) {
  SUITE(files) {
    TEST(coherentLocation) {
      Parset ps = makeDefaultTestParset();

      // set
      ps.replace("Observation.DataProducts.Output_CoherentStokes.enabled", "true");
      ps.replace("Observation.nrBeams", "1");
      ps.replace("Observation.Beam[0].tiedArrayBeam[0].coherent", "true");
      ps.replace("Observation.Beam[0].nrTiedArrayBeams", "1");
      ps.replace("Observation.Beam[0].subbandList", "[0]");
      ps.replace("Observation.DataProducts.Output_CoherentStokes.filenames", "[tab1.hdf5]");
      ps.replace("Observation.DataProducts.Output_CoherentStokes.locations", "[host:/dir]");
      ps.updateSettings();

      // verify settings
      CHECK_EQUAL("tab1.hdf5", ps.settings.beamFormer.files[0].location.filename);
      CHECK_EQUAL("host",      ps.settings.beamFormer.files[0].location.host);
      CHECK_EQUAL("/dir",      ps.settings.beamFormer.files[0].location.directory);
    }

    TEST(manyLocations) {
      Parset ps = makeDefaultTestParset();

      // set
      ps.replace("Observation.DataProducts.Output_CoherentStokes.enabled", "true");
      ps.replace("Observation.nrBeams", "1");
      ps.replace("Observation.Beam[0].nrTiedArrayBeams", "500");
      ps.replace("Observation.Beam[0].subbandList", "[0]");
      ps.replace("Observation.DataProducts.Output_CoherentStokes.filenames", "[tab1..tab500]");
      ps.replace("Observation.DataProducts.Output_CoherentStokes.locations", "[500*host:/dir]");
      ps.updateSettings();

      // verify settings
      for(size_t i = 0; i < 500; ++i) {
        CHECK_EQUAL(str(format("tab%u") % (i+1)), ps.settings.beamFormer.files[i].location.filename);
        CHECK_EQUAL("host",      ps.settings.beamFormer.files[i].location.host);
        CHECK_EQUAL("/dir",      ps.settings.beamFormer.files[i].location.directory);
      }
    }
  }

  TEST(calcInternalNrChannels) {
    // Validate that we compute the (max) nr of channels for delay compensation
    // correctly.
    Parset ps = makeDefaultTestParset();

    ps.replace("Observation.DataProducts.Output_CoherentStokes.enabled", "true");
    ps.replace("Observation.DataProducts.Output_Correlated.enabled", "true");
    ps.replace("Observation.sampleClock", "200");
    ps.replace("Observation.antennaSet", "HBA_JOINED");

    ps.replace("Observation.nrBeams", "1");
    ps.replace("Observation.DataProducts.Output_Correlated.filenames", "[SB000_uv.MS, SB001_uv.MS, SB002_uv.MS]");
    ps.replace("Observation.DataProducts.Output_Correlated.locations", "[3*:.]");

    ps.replace("Observation.Dataslots.CS002HBA.RSPBoardList", "[0, 0, 1]");
    ps.replace("Observation.Dataslots.CS003HBA.RSPBoardList", "[0, 0, 1]");
    ps.replace("Observation.Dataslots.CS002HBA.DataslotList", "[0, 1, 2]");
    ps.replace("Observation.Dataslots.CS003HBA.DataslotList", "[0, 1, 2]");

    ps.replace("Observation.VirtualInstrument.stationList", "[CS002, CS003]");
    ps.replace("Observation.referencePhaseCenter", "[0.0, 0.0, 0.0]");

    // Check what it calculated. Compare the max for delay comp with the numbers
    // in the bf pipeline design doc, but note that:
    // - we also enforce a power of 2 FFT size
    // - we have maxed it at 256 channels/sb
    const unsigned maxNrDelayCh = 256;
    unsigned nDelayCh;

    // Fake the coords and ref of CS002 and CS003 to test with numbers that
    // correspond to the Cobalt design document. Idem for the subbands.

    // HBA_110_190 top of sb 409 is almost 180 MHz
    ps.replace("Observation.bandFilter", "HBA_110_190");
    ps.replace("Observation.Beam[0].subbandList", "[407, 408, 409]");

    // 3 km max (unprojected) delay distance
    ps.replace("PIC.Core.CS002HBA.phaseCenter", "[-3000.0, 0.0, 0.0]");
    ps.replace("PIC.Core.CS003HBA.phaseCenter", "[1000.0, 0.0, 0.0]");
    ps.updateSettings();

    nDelayCh = ps.settings.beamFormer.nrDelayCompensationChannels;
    CHECK_EQUAL(maxNrDelayCh, nDelayCh); // doc states 36718, pow2 and maxed gives 256


    // 1500 km max (unprojected) delay distance
    ps.replace("PIC.Core.CS002HBA.phaseCenter", "[0.0, 1500000.0, 0.0]");
    ps.updateSettings();

    nDelayCh = ps.settings.beamFormer.nrDelayCompensationChannels;
    CHECK_EQUAL(64u, nDelayCh); // doc states 73, pow2 gives 64
  }
}

/*
* ===============================================
* Test correct creation of tab rings
* ===============================================
*/
TEST(testRing) {
  string prefix = "Observation.Beam[0]";
  Parset ps = makeDefaultTestParset();

  // First add enough coherent names for all the created tabs
  ps.replace("Observation.DataProducts.Output_CoherentStokes.locations", "[8*localhost:.]");
  ps.replace("Observation.DataProducts.Output_CoherentStokes.filenames", "[8*SB000.MS]");
  ps.replace("Observation.DataProducts.Output_CoherentStokes.enabled", "true");

  ps.replace(prefix + ".TiedArrayBeam[0].angle1", "0.0");
  ps.replace(prefix + ".TiedArrayBeam[0].angle2", "0.0");
  ps.replace(prefix + ".TiedArrayBeam[0].coherent", "true");
  ps.replace(prefix + ".TiedArrayBeam[0].directionType", "J2000");
  ps.replace(prefix + ".TiedArrayBeam[0].dispersionMeasure", "0.0");
  ps.replace(prefix + ".TiedArrayBeam[0].specificationType", "manual"); // to clarify; not used by Parset obj
  ps.replace(prefix + ".nrTiedArrayBeams", "1");

  // We have 1 tabring
  string key = prefix + ".nrTabRings";
  string value = "1"; 
  ps.replace(key, value);

  // ringwidth == 1
  key = prefix + ".ringWidth";
  value = "2";
  ps.replace(key, value);

  // type
  key = prefix + ".directionType";
  value = "J2000";
  ps.replace(key, value);

  // location
  key = prefix + ".angle1";
  value = "3.0";
  ps.replace(key, value);

  key = prefix + ".angle2";
  value = "4.0";
  ps.replace(key, value);

  ps.updateSettings();

  // Validate output
  // number of tabs
  CHECK_EQUAL((size_t)8, ps.settings.beamFormer.maxNrCoherentTABsPerSAP());
  // take two random pointing and validate ( functionality is checking seperate
  // test suite)
  // The manual tabs should be added before the ring tabs. 
  // test values are for the 3rd value in the ring
  struct ObservationSettings::Direction sap = ps.settings.SAPs[0].direction;
  CHECK_CLOSE(sap.angle1 + 4.04656677402571, ps.settings.beamFormer.SAPs[0].TABs[3].direction.angle1, 0.00000001);
  CHECK_CLOSE(sap.angle2 + 1.15470053837925, ps.settings.beamFormer.SAPs[0].TABs[3].direction.angle2, 0.00000001);
  // Full list of value for 1 circle:
  //[(0, 0), (0, 2.3094010767585), (4.04656677402571, 1.15470053837925), (1.73205080756888, -1.15470053837925), (0, -2.3094010767585), (-1.73205080756888, -1.15470053837925), (-4.04656677402571, 1.15470053837925)]
}

/*
 * ===============================================
 * Test interaction with full Parsets for coherency
 * between fields.
 * ===============================================
 */

SUITE(integration) {
  TEST(228591) {
    // ===== read parset of observation L228591
    Parset ps("tParset.parset_obs228591");

    // check basic assumptions without which the subsequent
    // checks will return random crap
    CHECK(ps.settings.beamFormer.enabled);
    CHECK_EQUAL(1U, ps.settings.beamFormer.SAPs.size());

    // check the TAB rings: 4 rings = 61 TABs
    CHECK_EQUAL(61U, ps.settings.beamFormer.SAPs[0].TABs.size());

    // first TAB is equal to SAP
    struct ObservationSettings::Direction sap = ps.settings.SAPs[0].direction;
    CHECK_EQUAL(sap.angle1, ps.settings.beamFormer.SAPs[0].TABs[0].direction.angle1);
    CHECK_EQUAL(sap.angle2, ps.settings.beamFormer.SAPs[0].TABs[0].direction.angle2);

    // subsequent TABs are NOT (0,0)
    for (size_t tab = 1; tab < ps.settings.beamFormer.SAPs[0].TABs.size(); tab++) {
      struct ObservationSettings::Direction dir = ps.settings.beamFormer.SAPs[0].TABs[tab].direction;

      CHECK(dir.angle1 != sap.angle1 || dir.angle2 != sap.angle2);
    }
  }

  TEST(99275) {
    // ===== read parset of observation L99275
    Parset ps("tParset.parset_obs99275");

    // some constants we expect
    const size_t nrSubbands = 26;
    const size_t nrStations = 28;
    const size_t nrSAPs = 2;

    // ===== test the basics
    CHECK_EQUAL(99275U,      ps.settings.observationID);
    CHECK_EQUAL(true,        ps.settings.realTime);
    CHECK_EQUAL(16U,         ps.settings.nrBitsPerSample);
    CHECK_EQUAL(200U,        ps.settings.clockMHz);
    CHECK_EQUAL("LBA_OUTER", ps.settings.antennaSet);
    CHECK_EQUAL("LBA_10_90", ps.settings.bandFilter);
    CHECK_EQUAL(false,       ps.settings.anaBeam.enabled);

    // test antenna fields list
    CHECK_EQUAL(nrStations,  ps.settings.antennaFields.size());
    for (unsigned st = 0; st < nrStations; ++st) {
      CHECK_EQUAL(nrSubbands, ps.settings.antennaFields[st].rspBoardMap.size());
      CHECK_ARRAY_EQUAL(zeroes(nrSubbands),   ps.settings.antennaFields[st].rspBoardMap, nrSubbands);

      CHECK_EQUAL(nrSubbands, ps.settings.antennaFields[st].rspSlotMap.size());
      CHECK_ARRAY_EQUAL(sequence(nrSubbands), ps.settings.antennaFields[st].rspSlotMap, nrSubbands);
    }

    // check core antenna fields
    for (unsigned st = 0; st < 21; ++st) {
      CHECK_EQUAL("CS", ps.settings.antennaFields[st].name.substr(0,2));
      CHECK_CLOSE(3827000.0, ps.settings.antennaFields[st].phaseCenter[0], 2000);
      CHECK_CLOSE( 460900.0, ps.settings.antennaFields[st].phaseCenter[1], 2000);
      CHECK_CLOSE(5065000.0, ps.settings.antennaFields[st].phaseCenter[2], 2000);
    }

    // check remote antenna fields
    for (unsigned st = 21; st < nrStations; ++st) {
      CHECK_EQUAL("RS", ps.settings.antennaFields[st].name.substr(0,2));
      CHECK_CLOSE(3827000.0, ps.settings.antennaFields[st].phaseCenter[0], 30000);
      CHECK_CLOSE( 460900.0, ps.settings.antennaFields[st].phaseCenter[1], 20000);
      CHECK_CLOSE(5065000.0, ps.settings.antennaFields[st].phaseCenter[2], 20000);

      CHECK_EQUAL(0.0, ps.settings.antennaFields[st].clockCorrection);
    }

    // test subband/sap configuration
    CHECK_EQUAL(nrSubbands,  ps.settings.subbands.size());
    CHECK_EQUAL(nrSAPs,      ps.settings.SAPs.size());

    // check SAP 0
    CHECK_EQUAL("Sun",   ps.settings.SAPs[0].target);
    CHECK_EQUAL("J2000", ps.settings.SAPs[0].direction.type);
    for (unsigned sb = 0; sb < 13; ++sb) {
      CHECK_EQUAL(sb, ps.settings.subbands[sb].idx);
      CHECK_EQUAL(0U, ps.settings.subbands[sb].SAP);

      // subband list is increasing and positive
      CHECK(ps.settings.subbands[sb].stationIdx > (sb == 0 ? 0 : ps.settings.subbands[sb-1].stationIdx));
    }

    // check SAP 1
    CHECK_EQUAL("3C444", ps.settings.SAPs[1].target);
    CHECK_EQUAL("J2000", ps.settings.SAPs[1].direction.type);
    for (unsigned sb = 13; sb < nrSubbands; ++sb) {
      CHECK_EQUAL(sb, ps.settings.subbands[sb].idx);
      CHECK_EQUAL(1U, ps.settings.subbands[sb].SAP);

      // subband list of SAP 1 is equal to SAP 0
      CHECK_EQUAL(ps.settings.subbands[sb - 13].stationIdx, ps.settings.subbands[sb].stationIdx);
    }

    // ===== test correlator settings
    CHECK_EQUAL(true,       ps.settings.correlator.enabled);
    CHECK_EQUAL(64U,        ps.settings.correlator.nrChannels);
    CHECK_CLOSE(3051.76,    ps.settings.correlator.channelWidth, 0.01);
    CHECK_EQUAL(768U,       ps.settings.correlator.nrSamplesPerChannel);
    CHECK_EQUAL(30U,         ps.settings.correlator.nrBlocksPerIntegration);
    CHECK_EQUAL(nrStations, ps.settings.correlator.stations.size());
    for (unsigned st = 0; st < nrStations; ++st) {
      CHECK_EQUAL(ps.settings.antennaFields[st].name, ps.settings.correlator.stations[st].name);

      CHECK_EQUAL(1U, ps.settings.correlator.stations[st].inputStations.size());
      CHECK_EQUAL(st, ps.settings.correlator.stations[st].inputStations[0]);
    }
    CHECK_EQUAL(nrSubbands, ps.settings.correlator.files.size());

    // ===== test beam-former settings
    CHECK_EQUAL(true,       ps.settings.beamFormer.enabled);
    CHECK_EQUAL(nrSAPs,     ps.settings.beamFormer.SAPs.size());

    // check SAP 0
    CHECK_EQUAL(2U,         ps.settings.beamFormer.SAPs[0].TABs.size());
    CHECK_EQUAL(true,       ps.settings.beamFormer.SAPs[0].TABs[0].coherent);
    CHECK_EQUAL(false,      ps.settings.beamFormer.SAPs[0].TABs[1].coherent);

    // check SAP 1
    CHECK_EQUAL(2U,         ps.settings.beamFormer.SAPs[1].TABs.size());
    CHECK_EQUAL(true,       ps.settings.beamFormer.SAPs[1].TABs[0].coherent);
    CHECK_EQUAL(false,      ps.settings.beamFormer.SAPs[1].TABs[1].coherent);

    // check coherent settings
    CHECK_EQUAL(STOKES_I,   ps.settings.beamFormer.coherentSettings.type);
    CHECK_EQUAL(1U,         ps.settings.beamFormer.coherentSettings.nrStokes);
    CHECK_EQUAL(64U,        ps.settings.beamFormer.coherentSettings.nrChannels);
    CHECK_EQUAL(30U,        ps.settings.beamFormer.coherentSettings.timeIntegrationFactor);

    // check incoherent settings
    CHECK_EQUAL(STOKES_I,   ps.settings.beamFormer.incoherentSettings.type);
    CHECK_EQUAL(1U,         ps.settings.beamFormer.incoherentSettings.nrStokes);
    CHECK_EQUAL(64U,        ps.settings.beamFormer.incoherentSettings.nrChannels);
    CHECK_EQUAL(1U,         ps.settings.beamFormer.incoherentSettings.timeIntegrationFactor);
  }
}

int main(void)
{
  INIT_LOGGER("tParset");

  return UnitTest::RunAllTests() > 0;
}

