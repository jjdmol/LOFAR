//# PacketReader.cc
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

#include "PacketReader.h"

#include <cmath>
#include <sys/time.h>
#include <typeinfo>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <CoInterface/Stream.h>


namespace LOFAR
{
  namespace Cobalt
  {
    // Create an 'invalid' mode to make it unique and not match any actually used mode.
    const BoardMode PacketReader::MODE_ANY(0, 0);

    PacketReader::PacketReader( const std::string &logPrefix, Stream &inputStream,
                                const BoardMode &mode )
      :
      logPrefix(str(boost::format("%s [PacketReader] ") % logPrefix)),
      inputStream(inputStream),
      mode(mode),

      nrReceived(0),
      nrBadMode(0),
      nrBadTime(0),
      nrBadData(0),
      nrBadOther(0),
      hadSizeError(false),
      lastLogTime(0)
    {
      // Partial reads are not supported on UDP streams, because each read()
      // will consume a full packet.
      try {
        SocketStream &asSocket = dynamic_cast<SocketStream &>(inputStream);
        const bool isUDP = asSocket.protocol == SocketStream::UDP;

        inputIsUDP = isUDP;
      } catch (std::bad_cast&) {
        // inputStream is not a SocketStream
        inputIsUDP = false;
      }
    }


    void PacketReader::readPackets( std::vector<struct RSP> &packets )
    {
      size_t numRead;

      if (inputIsUDP) {
        SocketStream &sstream = dynamic_cast<SocketStream&>(inputStream);

        vector<unsigned> recvdSizes(packets.size());
        numRead = sstream.recvmmsg(&packets[0], sizeof(struct RSP), recvdSizes);

        nrReceived += numRead;

        // validate received packets
        for (size_t i = 0; i < numRead; ++i) {
          packets[i].payloadError(!validatePacket(packets[i], recvdSizes[i]));
        }
      } else {
        // fall-back for non-UDP streams, emit packets
        // one at a time to avoid data loss on EndOfStream.
        packets[0].payloadError(!readPacket(packets[0]));
        numRead = 1;
      }

      // mark unused packet buffers as invalid
      for (size_t i = numRead; i < packets.size(); ++i) {
        packets[i].payloadError(true);
      }
    }


    bool PacketReader::readPacket( struct RSP &packet )
    {
      size_t numbytes;

      if (inputIsUDP) {
        numbytes = inputStream.tryRead(&packet, sizeof packet);
      } else {
        // read header first to determine actual packet size
        inputStream.read(&packet.header, sizeof packet.header);
        size_t pktSize = packet.packetSize();

        // read rest of packet
        inputStream.read(&packet.payload.data, pktSize - sizeof packet.header);
        numbytes = pktSize;
      }

      ++nrReceived;

      return validatePacket(packet, numbytes);
    }

    bool PacketReader::validatePacket( const struct RSP &packet, size_t numbytes )
    {
      // illegal size means illegal packet; don't touch
      if ( numbytes < sizeof(struct RSP::Header) 
            || numbytes != packet.packetSize() ) {
        if (!hadSizeError) {
          LOG_ERROR_STR( logPrefix << "Packet is " << numbytes <<
                         " bytes, but should be " << packet.packetSize() << " bytes" );
          hadSizeError = true;
        }

        ++nrBadOther;
        return false;
      }

      // illegal version means illegal packet
      if (packet.header.version < 2) {
        // This mainly catches packets that are all zero (f.e. /dev/zero or
        // null: streams).
        ++nrBadOther;
        return false;
      }

      // illegal timestamp means illegal packet
      if (packet.header.timestamp == ~0U) {
        ++nrBadTime;
        return false;
      }

      // discard packets with errors
      if (packet.payloadError()) {
        ++nrBadData;
        return false;
      }

      if (packet.bitMode() != mode.bitMode
       || packet.clockMHz() != mode.clockMHz) {
        if (mode != MODE_ANY) {
          ++nrBadMode;
          return false;
        }
      }

      // everything is ok
      return true;
    }


    void PacketReader::logStatistics(unsigned boardNr,
                                     MACIO::RTmetadata &mdLogger,
                                     const string &mdKeyPrefix)
    {
      // Determine time since last log
      struct timeval tv;
      gettimeofday(&tv, NULL);
      const double now = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

      const double interval = now - lastLogTime;

      // Emit log line
      LOG_INFO_STR( logPrefix << (nrReceived / interval) << " pps: received " <<
                    nrReceived << " packets: " << nrBadTime << " bad timestamps, " <<
                    nrBadMode << " bad clock/bitmode, " << nrBadData << " payload errors, " <<
                    nrBadOther << " otherwise bad packets" );

      // Emit data points for monitoring (PVSS)
      // Reproduce PN_CSI_STREAM0_BLOCKS_IN or PN_CSI_STREAM0_REJECTED, but with the right nr.
      string streamStr = str(boost::format("stream%u") % boardNr);
      mdLogger.log(mdKeyPrefix + streamStr + ".blocksIn",
                   (int)round(nrReceived / interval));
      size_t nrBad = nrBadTime + nrBadMode + nrBadData + nrBadOther;
      mdLogger.log(mdKeyPrefix + streamStr + ".rejected",
                   (int)round(nrBad / interval));

      // Reset counters
      nrReceived = 0;
      nrBadTime = 0;
      nrBadMode = 0;
      nrBadData = 0;
      nrBadOther = 0;

      hadSizeError = false;

      lastLogTime = now;
    }


  }
}

