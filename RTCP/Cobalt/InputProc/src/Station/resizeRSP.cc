//# resizeRSP.cc: resize the number of beamlets of each packet in an RSP stream
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "PacketReader.h"

#include <cstdio>
#include <cstring>

#include <Common/LofarLogger.h>
#include <Stream/FileStream.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/Stream.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

int main(int argc, char **argv)
{
  INIT_LOGGER("resizeRSP");

  if (argc < 2) {
    puts("Usage: resizeRSP nrBeamlets < input.udp > output.udp");
    puts("");
    puts("Reduces or expands the number of beamlets in the packets.");
    exit(1);
  }

  const size_t target_nr_beamlets = atoi(argv[1]);

  SmartPtr<Stream> inputStream = createStream("file:/dev/stdin", true);
  PacketReader reader("", *inputStream);
  struct RSP packet;

  try {
    for(;;) {
      bool havePacket;
      memset(&packet, sizeof packet, 0);

      // read from input
      havePacket = reader.readPacket(packet);

      if (havePacket) {
        // the new number of beamlets has to be valid
        ASSERT(target_nr_beamlets <= 62 * (16 / packet.bitMode()));

        // convert
        packet.header.nrBeamlets = target_nr_beamlets;

        // write to output
        fwrite(&packet, packet.packetSize(), 1, stdout);
      }
    }
  } catch(Stream::EndOfStreamException&) {
  }
}


