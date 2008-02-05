//#  -*- mode: c++ -*-
//#
//#  RCUProtocolWrite.h: Synchronize rcu settings with RSP hardware.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef RCUPROTOCOLWRITE_H_
#define RCUPROTOCOLWRITE_H_

#include <APL/RSP_Protocol/MEPHeader.h>

#include "SyncAction.h"

namespace LOFAR {
  namespace RSP {

    class RCUProtocolWrite : public SyncAction
    {
    public:
      /**
       * Constructors for a RCUProtocolWrite object.
       */
      RCUProtocolWrite(GCFPortInterface& board_port, int board_id);

      /* Destructor for RCUProtocolWrite. */
      virtual ~RCUProtocolWrite();

      /**
       * Send the write message.
       */
      virtual void sendrequest();

      /**
       * Send the read request.
       */
      virtual void sendrequest_status();

      /**
       * Handle the read result.
       */
      virtual GCFEvent::TResult handleack(GCFEvent& event, GCFPortInterface& port);

    private:
      EPA_Protocol::MEPHeader m_hdr;

      friend class RCUResultRead;

      static const int PROTOCOL_SIZE = 10;
      static const int RESULT_SIZE   = 6; 

      // construct i2c sequence
      static uint8 i2c_protocol[PROTOCOL_SIZE];

      // construct expected i2c result
      static uint8 i2c_result[RESULT_SIZE];
    };
  };
};
     
#endif /* RCUPROTOCOLWRITE_H_ */
