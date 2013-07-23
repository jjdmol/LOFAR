//#  -*- mode: c++ -*-
//#
//#  HBAProtocolWrite.h: Synchronize rcu settings with RSP hardware.
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

#ifndef HBAPROTOCOLWRITE_H_
#define HBAPROTOCOLWRITE_H_

#include <APL/RSP_Protocol/MEPHeader.h>

#include "SyncAction.h"

namespace LOFAR {
  namespace RSP {

    class HBAProtocolWrite : public SyncAction
    {
    public:
      /**
       * Constructors for a HBAProtocolWrite object.
       */
      HBAProtocolWrite(GCFPortInterface& board_port, int board_id);

      /* Destructor for HBAProtocolWrite. */
      virtual ~HBAProtocolWrite();

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

      friend class HBAResultRead;

      //
      // Uncomment next line to actually write HBA delays
      // If commented out, the code will only switch the LED on/off
      // to test the i2c interface from the EPA firmware to the HBA piggy back board
      //
#define HBA_WRITE_DELAYS

#ifdef HBA_WRITE_DELAYS
      static const int PROTOCOL_SIZE         = 320;
      static const int RESULT_SIZE           = 115;

      static const int PROTOCOL_DELAY_OFFSET = 10; // offset of delay settings in i2c_protocol
      static const int RESULT_DELAY_OFFSET   = 6;  // offset of delay settings in i2c_result
      static const int RESULT_DELAY_STRIDE   = 7;  // nof bytes to next pair of X,Y delays
#else
      static const int PROTOCOL_SIZE         = 8;
      static const int RESULT_SIZE           = 4;
      static const int PROTOCOL_LED_OFFSET   = 3;
      static const int RESULT_LED_OFFSET     = 1;
#endif

      static int i2c_protocol_patch_indices[];
      static int i2c_result_patch_indices[];

      // construct i2c sequence
      static uint8 i2c_protocol[PROTOCOL_SIZE];

      // construct expected i2c result
      static uint8 i2c_result[RESULT_SIZE];

      // used to switch LED on/off each time
    public:
      static int m_on_off;
    };
  };
};
     
#endif /* HBAPROTOCOLWRITE_H_ */
