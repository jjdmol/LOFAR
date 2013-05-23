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
#include <boost/date_time/posix_time/posix_time.hpp>

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

int main(int argc, char **argv)
{
  INIT_LOGGER("filterRSP");

  if (argc < 3) {
    puts("Usage: filterRSP '2012-01-01 11:12:00' '2012-01-01 11:13:00' < input.udp > output.udp");
    puts("");
    puts("Writes all packets between the given timestamps: [from,to).");
    exit(1);
  }

  time_t from = parseTime(argv[1]);
  time_t to = parseTime(argv[2]);

  SmartPtr<Stream> inputStream = createStream("file:/dev/stdin", true);
  PacketReader reader("", *inputStream);
  struct RSP packet;

  try {
    for(;; ) {
      if( reader.readPacket(packet) ) {
        if (packet.header.timestamp < from || packet.header.timestamp >= to)
          continue;

        fwrite(&packet, packet.packetSize(), 1, stdout);
      }
    }
  } catch(Stream::EndOfStreamException&) {
  }
}

