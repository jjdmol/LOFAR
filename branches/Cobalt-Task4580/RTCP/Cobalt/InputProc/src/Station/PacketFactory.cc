/* PacketFactory.cc
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include <lofar_config.h>

#include "PacketFactory.h"

#include <string.h>

namespace LOFAR
{
  namespace Cobalt
  {

    PacketFactory::PacketFactory( const BufferSettings &settings, const struct BoardMode &mode )
      :
      settings(settings),
      mode(mode)
    {
    }

    PacketFactory::~PacketFactory()
    {
    }

    
    void PacketFactory::makeHeader( struct RSP &packet, const TimeStamp &timestamp, size_t boardNr )
    {
      // configure the packet header
      packet.header.version = 3; // we emulate BDI 6.0

      packet.header.sourceInfo1 =
        (boardNr & 0x1F) | (mode.clockMHz == 200 ? 1 << 7 : 0);

      switch (mode.bitMode) {
      case 16:
        packet.header.sourceInfo2 = 0;
        break;

      case 8:
        packet.header.sourceInfo2 = 1;
        break;

      case 4:
        packet.header.sourceInfo2 = 2;
        break;
      }

      packet.header.nrBeamlets = mode.nrBeamletsPerBoard();
      packet.header.nrBlocks = 16;

      packet.header.timestamp = timestamp.getSeqId();
      packet.header.blockSequenceNumber = timestamp.getBlockId();

      // verify whether the packet really reflects what we intended
      ASSERT(packet.rspBoard()     == boardNr);
      ASSERT(packet.payloadError() == false);
      ASSERT(packet.bitMode()      == mode.bitMode);
      ASSERT(packet.clockMHz()     == mode.clockMHz);

      // verify that the packet has a valid size
      ASSERT(packet.packetSize()   <= sizeof packet);
    }


    
    void PacketFactory::makePayload( struct RSP &packet )
    {
      // insert data that is different for each packet
      int64 data = packet.timeStamp();

      memset(packet.payload.data, data & 0xFF, sizeof packet.payload.data);
    }


    void PacketFactory::makePacket( struct RSP &packet, const TimeStamp &timestamp, size_t boardNr )
    {
      makeHeader(packet, timestamp, boardNr);
      makePayload(packet);
    }
  }
}

