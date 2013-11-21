//# filterRSP.h
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
  puts("Usage: filterRSP [options] < input.udp > output.udp");
  puts("");
  puts("-f from       Discard packets before `from' (format: '2012-01-01 11:12:00')");
  puts("-t to         Discard packets at or after `to' (format: '2012-01-01 11:12:00')");
  puts("-q            Quit if packets are received after `to'");
  puts("-s nrbeamlets Reduce or expand the number of beamlets per packet");
  puts("-b bitmode    Discard packets with bitmode other than `bitmode' (16, 8, or 4)");
  puts("-c clock      Discard packets with a clock other than `clock' (200 or 160)");
  puts("-i streamdesc Stream descriptor for input (default = file:/dev/stdin)");
  puts("-o streamdesc Stream descriptor for output (default = file:/dev/stdout)");
  puts("");
  puts("Note: invalid packets are always discarded.");
}

struct packetSet {
  vector<struct RSP> packets;
  vector<bool>       valid;
};

int main(int argc, char **argv)
{
  INIT_LOGGER("filterRSP");

  int opt;

  time_t from = 0;
  time_t to = 0;
  unsigned nrbeamlets = 0;
  unsigned bitmode = 0;
  unsigned clock = 0;
  bool quit_after_to = false;

  string inputStreamDesc  = "file:/dev/stdin";
  string outputStreamDesc = "file:/dev/stdout";

  // parse all command-line options
  while ((opt = getopt(argc, argv, "f:t:s:b:c:i:o:q")) != -1) {
    switch (opt) {
    case 'f':
      from = parseTime(optarg);
      break;

    case 't':
      to = parseTime(optarg);
      break;

    case 's':
      nrbeamlets = atoi(optarg);
      break;

    case 'b':
      bitmode = atoi(optarg);
      break;

    case 'c':
      clock = atoi(optarg);
      break;

    case 'i':
      inputStreamDesc = optarg;
      break;

    case 'o':
      outputStreamDesc = optarg;
      break;

    case 'q':
      quit_after_to = true;
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
  SmartPtr<Stream> inputStream = createStream(inputStreamDesc, true);
  SmartPtr<Stream> outputStream = createStream(outputStreamDesc, false);
  PacketReader reader("", *inputStream);

  // create packet queues between reader and writer
  Queue< SmartPtr<packetSet> > readQueue;
  Queue< SmartPtr<packetSet> > writeQueue;

  for (size_t i = 0; i < 256; ++i) {
    SmartPtr<packetSet> p = new packetSet;
    p->packets.resize(256);
    p->valid.resize(p->packets.size());

    readQueue.append(p);
  }

  /*
   * We need to read and write in separate threads
   * for the best performance.
   *
   * A dedicated read thread allows us to always
   * listen to the input, which is especially
   * important if we're listening to UDP.
   */

  volatile bool writerDone = false;

# pragma omp parallel sections num_threads(2)
  {
#   pragma omp section
    {
      try {
        Thread::ScopedPriority sp(SCHED_FIFO, 10);

        SmartPtr<packetSet> p;

        while (!writerDone && (p = readQueue.remove()) != NULL) {
          // Read packets and queue them
          reader.readPackets(p->packets, p->valid);
          writeQueue.append(p);
        }
      } catch(Stream::EndOfStreamException&) {
      }

      writeQueue.append(NULL);
    }

#   pragma omp section
    {
      SmartPtr<packetSet> p;

      // Keep reading until NULL
      while ((p = writeQueue.remove()) != NULL) {
        for (size_t i = 0; i < p->packets.size(); ++i) {
          if (!p->valid[i])
            continue;

          struct RSP &packet = p->packets[i];

          // **** Apply FROM filter ****
          if (from > 0 && packet.header.timestamp < from)
            continue;

          // **** Apply TO filter ****
          if (to > 0 && packet.header.timestamp >= to) {
            if (quit_after_to) {
              writerDone = true;
              break;
            }

            continue;
          }

          // **** Apply BITMODE filter ****
          if (bitmode > 0 && packet.bitMode() != bitmode)
            continue;

          // **** Apply CLOCK filter ****
          if (clock > 0 && packet.clockMHz() != clock)
            continue;

          // **** Apply NRBEAMLETS filter ****
          if (nrbeamlets > 0) {
            // the new number of beamlets has to be valid
            ASSERT(nrbeamlets <= 62 * (16 / packet.bitMode()));

            // convert
            packet.header.nrBeamlets = nrbeamlets;
          }

          // Write packet
          outputStream->write(&packet, packet.packetSize());
        }

        // Give back packets holder
        readQueue.append(p);

        // Add a NULL if we're done to free up
        // readQueue.remove(), to prevent race conditions.
        if (writerDone) {
          readQueue.append(NULL);
          break;
        }
      }
    }
  }
}

