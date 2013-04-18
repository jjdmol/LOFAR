//# PacketsToBuffer.cc
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
//# $Id: $

#include <lofar_config.h>

#include "PacketsToBuffer.h"

#include <boost/format.hpp>

#include <Common/LofarLogger.h>

#include <InputProc/SampleType.h>
#include <InputProc/Buffer/SampleBuffer.h>

#include "PacketWriter.h"

namespace LOFAR
{
  namespace Cobalt
  {


    PacketsToBuffer::PacketsToBuffer( Stream &inputStream, const BufferSettings &settings, unsigned boardNr )
      :
      logPrefix(str(boost::format("[station %s board %u] [PacketsToBuffer] ") % settings.station.stationName % boardNr)),
      inputStream(inputStream),
      lastlog_timestamp(0),
      settings(settings),
      boardNr(boardNr)
    {
      LOG_INFO_STR( logPrefix << "Initialised" );
    }


    void PacketsToBuffer::process()
    {
      // Holder for packet
      struct RSP packet;

      // Whether packet has been read already
      bool packetValid = false;

      // Keep reading if mode changes
      for(;; ) {
        try {
          // Process packets based on (expected) bit mode
          switch(settings.station.bitMode) {
          case 16:
            process< SampleType<i16complex> >(packet, packetValid);
            break;

          case 8:
            process< SampleType<i8complex> >(packet, packetValid);
            break;

          case 4:
            process< SampleType<i4complex> >(packet, packetValid);
            break;
          }

          // process<>() exited gracefully, so we're done
          break;
        } catch (PacketReader::BadModeException &ex) {
          // Mode switch detected
          unsigned bitMode = packet.bitMode();
          unsigned clockMHz = packet.clockMHz();
          unsigned nrBeamlets = packet.header.nrBeamlets;

          LOG_INFO_STR( logPrefix << "Mode switch detected to " << clockMHz << " MHz, " << bitMode << " bit, " << nrBeamlets << " beamlets");

          // update settings
          settings.station.bitMode = bitMode;
          settings.station.clockMHz = clockMHz;
          settings.nrBeamletsPerBoard = nrBeamlets;

          // Process packet again
          packetValid = true;
        }
      }
    }


    void PacketsToBuffer::logStatistics( PacketReader &reader, const struct RSP &packet )
    {
      if (packet.header.timestamp < lastlog_timestamp + LOG_INTERVAL) {
        lastlog_timestamp = packet.header.timestamp;

        reader.logStatistics();
      }
    }


    template<typename T>
    void PacketsToBuffer::process( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException)
    {
      // Create input structures
      PacketReader reader(logPrefix, inputStream);

      // Create output structures
      SampleBuffer<T> buffer(settings, true);
      PacketWriter<T> writer(logPrefix, buffer, boardNr);

      LOG_INFO_STR( logPrefix << "Processing packets" );

      try {
        // Process lingering packet from previous run, if any
        if (writeGivenPacket) {
          writer.writePacket(packet);
          logStatistics(reader, packet);
        }

        // Transport packets from reader to writer
        for(;; ) {
          if (reader.readPacket(packet, settings)) {
            writer.writePacket(packet);
            logStatistics(reader, packet);
          }
        }

      } catch (PacketReader::BadModeException &ex) {
        // Packet has different clock or bitmode
        throw;

      } catch (Stream::EndOfStreamException &ex) {
        // Ran out of data
        LOG_INFO_STR( logPrefix << "End of stream");

      } catch (SystemCallException &ex) {
        if (ex.error == EINTR)
          LOG_INFO_STR( logPrefix << "Aborted: " << ex.what());
        else
          LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);

      } catch (Exception &ex) {
        LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
      }

      LOG_INFO_STR( logPrefix << "End");
    }


    // Explcitly create the instances we use
    template void PacketsToBuffer::process< SampleType<i16complex> >( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException);
    template void PacketsToBuffer::process< SampleType<i8complex> >( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException);
    template void PacketsToBuffer::process< SampleType<i4complex> >( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException);


  }
}
