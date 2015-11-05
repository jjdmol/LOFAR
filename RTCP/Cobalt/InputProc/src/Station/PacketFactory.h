/* PacketFactory.h
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

#ifndef LOFAR_INPUT_PROC_PACKETFACTORY_H
#define LOFAR_INPUT_PROC_PACKETFACTORY_H

#include <InputProc/Buffer/BufferSettings.h>
#include <InputProc/RSPTimeStamp.h>

#include "RSP.h"

namespace LOFAR
{
  namespace Cobalt
  {
    /* Generate RSP packets */

    class PacketFactory
    {
    public:
      PacketFactory( const BufferSettings &settings );
      virtual ~PacketFactory();

      /*
       * Fill an RSP packet for a certain RSP board and time stamp.
       */
      virtual void makePacket( struct RSP &packet, const TimeStamp &timestamp, size_t boardNr);

    protected:
      const BufferSettings &settings;

      /*
       * Fill packet.header.
       */
      virtual void makeHeader( struct RSP &packet, const TimeStamp &timestamp, size_t boardNr);

      /*
       * Fill packet.payload. Called after makeHeader().
       */
      virtual void makePayload( struct RSP &packet );
    };

  }
}

#endif

