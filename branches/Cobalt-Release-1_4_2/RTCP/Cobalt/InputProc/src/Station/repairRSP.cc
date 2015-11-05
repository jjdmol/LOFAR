//# repairRSP.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <omp.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Common/Thread/Queue.h>
#include <Common/Thread/Thread.h>
#include <Common/LofarLogger.h>
#include <ApplCommon/PosixTime.h>
#include <CoInterface/Stream.h>
#include <CoInterface/SmartPtr.h>
#include "RSP.h"
#include "PacketReader.h"

using namespace LOFAR;
using namespace Cobalt;

time_t parseTime(const char *str)
{
  return to_time_t(boost::posix_time::time_from_string(str));
}

void usage()
{
  puts("Usage: repairRSP [options] < input.udp > output.udp");
  puts("");
  puts("-f from       Discard packets before `from' (format: '2012-01-01 11:12:00')");
  puts("-t to         Discard packets at or after `to' (format: '2012-01-01 11:12:00')");
  puts("-3            Upgrade packets to v3 (BDI 6.0)");
  puts("-s nrbeamlets Override the number of beamlets per packet");
}

int main(int argc, char **argv)
{
  INIT_LOGGER("repairRSP");

  // Force printing times in UTC
  setenv("TZ", "UTC", 1);

  int opt;

  time_t from = 0;
  time_t to = 0;
  bool quit_after_to = false;
  unsigned nrbeamlets = 0;
  bool to_v3 = false;

  // parse all command-line options
  while ((opt = getopt(argc, argv, "f:t:qs:3")) != -1) {
    switch (opt) {
    case 'f':
      from = parseTime(optarg);
      break;

    case 't':
      to = parseTime(optarg);
      break;

    case 'q':
      quit_after_to = true;
      break;

    case 's':
      nrbeamlets = atoi(optarg);
      break;

    case '3':
      to_v3 = true;
      break;

    default: /* '?' */
      usage();
      exit(1);
    }
  }

  // we expect no further arguments
  if (optind != argc) {
    usage();
    exit(1);
  }

  // create in- and output streams
  SmartPtr<Stream> inputStream = createStream("file:/dev/stdin", true);
  SmartPtr<Stream> outputStream = createStream("file:/dev/stdout", false);

  try {
    for(;;) {
      struct RSP packet;

      // Read header
      inputStream->read(&packet.header, sizeof packet.header);

      // **** Apply NRBEAMLETS ****
      if (nrbeamlets > 0) {
        packet.header.nrBeamlets = nrbeamlets;
      }

      // **** Convert to VERSION 3 ****
      if (to_v3 && packet.header.version < 3) {
        packet.header.version = 3;
        packet.bitMode(16);
      }

      // Read payload after header repair
      inputStream->read(&packet.payload.data, packet.packetSize() - sizeof packet.header);

      // **** Apply FROM filter ****
      if (from > 0 && packet.header.timestamp < from)
        continue;

      // **** Apply TO filter ****
      if (to > 0 && packet.header.timestamp >= to) {
        if (quit_after_to)
          break;

        continue;
      }

      // Write packet
      outputStream->write(&packet, packet.packetSize());
    }
  } catch(Stream::EndOfStreamException&) {
  }
}

