//# station_stream.cc: Generate station input stream information from a parset
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
//# $Id: mpi_node_list.cc 25888 2013-08-01 09:05:27Z mol $

#include <lofar_config.h>

#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>
#include <boost/format.hpp>

#include <Common/StreamUtil.h>
#include <CoInterface/Parset.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

void usage(char **argv)
{
  cerr << "usage: " << argv[0] << " -S station            -h parset" << endl;
  cerr << "usage: " << argv[0] << " -S station [-B board] -s parset" << endl;
  cerr << "usage: " << argv[0] << " -S station            -c parset" << endl;
  cerr << endl;
  cerr << "-h        : print the host name    on which this station is received" << endl;
  cerr << "-s        : print the stream       on which this station is received" << endl;
  cerr << "-s        : print the cpu (socket) on which this station is received" << endl;
  cerr << endl;
  cerr << "-S station: select station (CS001LBA, etc)" << endl;
  cerr << "-B board  : select board (0, 1, 2, 3)" << endl;
  cerr << endl;
}

void print_node_list(Parset &ps)
{
  // Collect all host names
  vector<string> nodes;
  for (size_t node = 0; node < ps.settings.nodes.size(); ++node)
    nodes.push_back(ps.settings.nodes[node].hostName);

  // Output all host names
  writeVector(cout, nodes, ",", "", "");
  cout << endl;
}

int main(int argc, char **argv)
{
  INIT_LOGGER("station_stream");

  string station = "";
  unsigned board = 0;
  bool print_host = false;
  bool print_stream = false;
  bool print_cpu = false;

  // parse all command-line option
  int opt;
  while ((opt = getopt(argc, argv, "B:S:hsc")) != -1) {
    switch (opt) {
    case 'B':
      board = atoi(optarg);
      break;

    case 'S':
      station = optarg;
      break;

    case 'h':
      print_host = true;
      break;

    case 's':
      print_stream = true;
      break;

    case 'c':
      print_cpu = true;
      break;

    default: /* '?' */
      usage(argv);
      exit(1);
    }
  }

  
  if ( optind >= argc   // we expect a parset filename as an additional parameter
    || station.empty()  // we expect a station to be selected
    || (print_host + print_stream + print_cpu != 1) // print either host or stream
     ) {
    usage(argv);
    exit(1);
  }

  // Create a parameters set object based on the inputs
  Parset ps(argv[optind]);


  // Find the selected station
  ssize_t stationIdx = ps.settings.stationIndex(station);

  if (stationIdx < 0) {
    LOG_WARN_STR("Station not found in parset, adding: " << station);

    // Add our station explicitly
    ps.add("Observation.VirtualInstrument.stationList", str(format("[%s]") % station));
    ps.updateSettings();

    // Update index
    stationIdx = ps.settings.stationIndex(station);

    ASSERT(stationIdx >= 0);
  }

  if (print_stream) {
    if (board >= ps.settings.stations[stationIdx].inputStreams.size()) {
      LOG_ERROR_STR("Input for board " << board << " not found for station " << station);
      cout << "file:/dev/null" << endl;
      return 1;
    }

    // Print the input stream for the given station and board
    cout << ps.settings.stations[stationIdx].inputStreams[board] << endl;
  } else if (print_host || print_cpu) {
    // Print the hostName of the given station, or localhost if
    // unknown.

    const string receiver = ps.settings.stations[stationIdx].receiver;
    bool found = false;

    for (size_t i = 0; i < ps.settings.nodes.size(); ++i) {
      struct ObservationSettings::Node &node = ps.settings.nodes[i];
      
      if (node.name == receiver) {
        if (print_host) {
          cout << node.hostName << endl;
        } else {
          // print_cpu
          cout << node.cpu << endl;
        }

        found = true;
        break;
      }
    }

    if (!found) {
      if (print_host) {
        // Default to localhost
        cout << "localhost" << endl;
      } else {
        // Default to cpu 0
        cout << "0" << endl;
      }
    }
  }
  return 0;
}

