//# PacketReader.h
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

#ifndef LOFAR_INPUT_PROC_PACKETREADER_H
#define LOFAR_INPUT_PROC_PACKETREADER_H

#include <string>

#include <Common/Exception.h>
#include <Stream/Stream.h>
#include <InputProc/Buffer/BoardMode.h>

#include "RSP.h"

namespace LOFAR
{
  namespace Cobalt
  {

    /*
     * Reads RSP packets from a Stream, and collects statistics.
     *
     * Thread-safefy: none.
     */
    class PacketReader
    {
    public:
      static const BoardMode MODE_ANY;

      PacketReader( const std::string &logPrefix, Stream &inputStream, const BoardMode &mode = MODE_ANY );

      // Reads a set of packets from the input stream. Sets the payloadError
      // flag for all invalid packets.
      void readPackets( std::vector<struct RSP> &packets );

      // Reads a packet from the input stream. Returns true if a packet was
      // succesfully read.
      bool readPacket( struct RSP &packet );

      // Logs (and resets) statistics about the packets read.
      void logStatistics();

    private:
      const std::string logPrefix;

      // The stream from which packets are read.
      Stream &inputStream;

      // The mode against which to validate (ignored if mode == MODE_ANY)
      const BoardMode mode;

      // Whether inputStream is an UDP stream
      // (UDP streams allow partial reads, and recvmmsg).
      bool inputIsUDP;

      // Statistics covering the packets read so far
      size_t nrReceived; // nr. of packets received
      size_t nrBadMode; // nr. of packets with wrong mode (clock, bit mode)
      size_t nrBadTime; // nr. of packets with an illegal time stamp
      size_t nrBadData; // nr. of packets with payload errors
      size_t nrBadOther; // nr. of packets that are bad in another fashion (illegal header, packet size, etc)

      bool hadSizeError; // already reported about wrongly sized packets since last logStatistics()

      double lastLogTime; // time since last log print, to monitor data rates

      bool validatePacket(const struct RSP &packet);
    };


  }
}

#endif

