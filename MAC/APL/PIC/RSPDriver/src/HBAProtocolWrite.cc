//#  HBAProtocolWrite.cc: implementation of the HBAProtocolWrite class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "StationSettings.h"
#include "HBAProtocolWrite.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace blitz;

namespace LOFAR {
  namespace RSP {

    //
    // The byte sequences originate from the LOFAR-ASTRON-MEM-175 Document
    // "HBA Control Design Description"
    //

    uint8 HBAProtocolWrite::i2c_protocol[HBAProtocolWrite::PROTOCOL_SIZE] 
    = {
      // Instruct client to do a broadcast access to the 16 HBA delay servers (43 bytes)
      13,   // PROTOCOL_C_WRITE_BLOCK_NO_CNT
       2,   // I2C address for RCU
       0,   // Request register access command
      39,   // Count of number of data bytes to write
       0,   // Cast type is broadcast
      32,   // Length of the request data for one server
       0,   // Expected length of the response from one server
       1,   // First server address participating in the cast
      16,   // Laster server address participating in the cast
       4,   // Function set word
       0,   // Server X-delay register ID
      0xFF, // X-delay data for server 1 (offset = PROTOCOL_DELAY_OFFSET)
      0xFF, // Y-delay data for server 1
      0xFF, // X-delay data for server 2
      0xFF, // Y-delay data for server 2
      0xFF, // X-delay data for server 3
      0xFF, // Y-delay data for server 3
      0xFF, // X-delay data for server 4
      0xFF, // Y-delay data for server 4
      0xFF, // X-delay data for server 5
      0xFF, // Y-delay data for server 5
      0xFF, // X-delay data for server 6
      0xFF, // Y-delay data for server 6
      0xFF, // X-delay data for server 7
      0xFF, // Y-delay data for server 7
      0xFF, // X-delay data for server 8
      0xFF, // Y-delay data for server 8
      0xFF, // X-delay data for server 9
      0xFF, // Y-delay data for server 9
      0xFF, // X-delay data for server 10
      0xFF, // Y-delay data for server 10
      0xFF, // X-delay data for server 11
      0xFF, // Y-delay data for server 11
      0xFF, // X-delay data for server 12
      0xFF, // Y-delay data for server 12
      0xFF, // X-delay data for server 13
      0xFF, // Y-delay data for server 13
      0xFF, // X-delay data for server 14
      0xFF, // Y-delay data for server 14
      0xFF, // X-delay data for server 15
      0xFF, // Y-delay data for server 15
      0xFF, // X-delay data for server 16
      0xFF, // Y-delay data for server 16
      
      // Wait to allow for the broadcast to finish (5 bytes)
      18, // PROTOCOL_C_WAIT
      0, 0, 0, 0, // Timeout value least significant - most significant

      // Instruct the client to do a read back multicast access to the 16 HBA delay servers (11 bytes)
      13, // PROTOCOL_C_WRITE_BLOCK_NO_CNT
       2, // Client i2c slave address
       0, // Request register access command
       7, // Count of number of data bytes to write
       1, // Cast type is multicast
       0, // Length of the request data for one server
       2, // Expected length of the response from one server
       1, // First server address participating in the cast
      16, // Last server address participating in the cast
       5, // Function get word
       0, // Server X-delay register ID

      // Wait to allow for the multicast to finish
      18, // PROTOCOL_C_WAIT (5 bytes)
      0, 0, 0, 0, // Timeout value least significant - most significant

      // Read back the response results from the client (4 bytes)
      14, // PROTOCOL_C_READ_BLOCK_NO_CNT
       2, // Client i2c slave address
       1, // Response register access command
      33, // Count of number of data bytes to read

      // Mark the end of the protocol list (1 byte)
      19, // PROTOCOL_C_END
    };

    uint8 HBAProtocolWrite::i2c_result[HBAProtocolWrite::RESULT_SIZE] 
    = {
      // Expected protocol result (39 bytes)
      0,    // Table 15 PROTOCOL_C_WRITE_BLOCK_NO_CNT went OK
      0,    // Table 16 PROCOCOL_C_WAIT went OK
      0,    // Table 17 PROTOCOL_C_WRITE_BLOCK_NO_CNT went OK
      0,    // Table 18 PROTOCOL_C_WAIT went OK
      0,    // Expected response status
      0xFF, // X-delay data from server 1 (offset = RESULT_DELAY_OFFSET)
      0xFF, // Y-delay data from server 1
      0xFF, // X-delay data from server 2
      0xFF, // Y-delay data from server 2
      0xFF, // X-delay data from server 3
      0xFF, // Y-delay data from server 3
      0xFF, // X-delay data from server 4
      0xFF, // Y-delay data from server 4
      0xFF, // X-delay data from server 5
      0xFF, // Y-delay data from server 5
      0xFF, // X-delay data from server 6
      0xFF, // Y-delay data from server 6
      0xFF, // X-delay data from server 7
      0xFF, // Y-delay data from server 7
      0xFF, // X-delay data from server 8
      0xFF, // Y-delay data from server 8
      0xFF, // X-delay data from server 9
      0xFF, // Y-delay data from server 9
      0xFF, // X-delay data from server 10
      0xFF, // Y-delay data from server 10
      0xFF, // X-delay data from server 11
      0xFF, // Y-delay data from server 11
      0xFF, // X-delay data from server 12
      0xFF, // Y-delay data from server 12
      0xFF, // X-delay data from server 13
      0xFF, // Y-delay data from server 13
      0xFF, // X-delay data from server 14
      0xFF, // Y-delay data from server 14
      0xFF, // X-delay data from server 15
      0xFF, // Y-delay data from server 15
      0xFF, // X-delay data from server 16
      0xFF, // Y-delay data from server 16
      0,    // Table 19 PROTOCOL_C_READ_BLOCK_NO_CNT went OK
      0,    // Table 20 PROTOCOL_C_END went OK
    };
  };
};

#define N_WRITES 2 // 2 writes, one for protocol register, one to clear results register

HBAProtocolWrite::HBAProtocolWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard() * N_WRITES)
  // using nrBlpsPerBoard() because only the Y RCU's (odd numbered 1,3,5,...)
  // control both the X and Y delays of HBA
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

HBAProtocolWrite::~HBAProtocolWrite()
{
  /* TODO: delete event? */
}

void HBAProtocolWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + (getCurrentIndex() / N_WRITES);

  // only update if rcuprotocol is not being updated and one of hbaprotocol needs updating
  if (RTC::RegisterState::IDLE != Cache::getInstance().getState().rcuprotocol().get(global_blp * MEPHeader::N_POL)
      || RTC::RegisterState::IDLE != Cache::getInstance().getState().rcuprotocol().get(global_blp * MEPHeader::N_POL + 1))
  {
    setContinue(true);
    return;
  }
   
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().hbaprotocol().get(global_blp * MEPHeader::N_POL)
      && RTC::RegisterState::WRITE != Cache::getInstance().getState().hbaprotocol().get(global_blp * MEPHeader::N_POL + 1))
  {
    Cache::getInstance().getState().hbaprotocol().unmodified(global_blp * MEPHeader::N_POL);
    Cache::getInstance().getState().hbaprotocol().unmodified(global_blp * MEPHeader::N_POL + 1);
    setContinue(true);
    return;
  }

  // delays for at least on HBA need to be written, and the RCUProtocol register is not in use by RCUProtocolWrite

  switch (getCurrentIndex() % N_WRITES) {
    
  case 0:
    {
      // reverse and copy control bytes into i2c_protocol
      Array<uint8, 2> delays(i2c_protocol + PROTOCOL_DELAY_OFFSET,
			     shape(MEPHeader::N_HBA_DELAYS, 2),
			     neverDeleteData);

      delays(Range::all(), 0) = Cache::getInstance().getBack().getHBASettings()()(global_blp * MEPHeader::N_POL, Range::all());
      delays(Range::all(), 1) = Cache::getInstance().getBack().getHBASettings()()(global_blp * MEPHeader::N_POL + 1, Range::all());

      // create the event
      EPARcuProtocolEvent rcuprotocol;
      rcuprotocol.hdr.set(MEPHeader::RCU_PROTOCOLY_HDR, 1 << (getCurrentIndex() / N_WRITES), MEPHeader::WRITE, sizeof(i2c_protocol));
      rcuprotocol.protocol.setBuffer(i2c_protocol, sizeof(i2c_protocol));
  
      m_hdr = rcuprotocol.hdr; // remember header to match with ack
      getBoardPort().send(rcuprotocol);
    }
    break;

  case 1:
    {
      EPAWriteEvent rcuresultwrite;

      // set the result register to 0xFF's
      rcuresultwrite.hdr.set(MEPHeader::WRITE, 1 << (getCurrentIndex() / N_WRITES),
			     MEPHeader::RCU, MEPHeader::RCU_RESULTY, sizeof(i2c_result), 0);
      uint8 clear[RESULT_SIZE];
      memset(clear, 0xFF, RESULT_SIZE); // clear result
      rcuresultwrite.payload.setBuffer(clear, RESULT_SIZE);

      m_hdr = rcuresultwrite.hdr; // remember header to match with ack
      getBoardPort().send(rcuresultwrite);
    }
    break;
  }
}

void HBAProtocolWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult HBAProtocolWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("HBAProtocolWrite::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + (getCurrentIndex() / N_WRITES);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("HBAProtocolWrite::handleack: invalid ack");
    Cache::getInstance().getState().hbaprotocol().write_error(global_blp * MEPHeader::N_POL);
    Cache::getInstance().getState().hbaprotocol().write_error(global_blp * MEPHeader::N_POL + 1);
    return GCFEvent::NOT_HANDLED;
  }

  if (1 == (getCurrentIndex() % N_WRITES)) {

    // Mark modification as applied when write of RCU result register has completed

    Cache::getInstance().getState().hbaprotocol().read_schedule(global_blp * MEPHeader::N_POL);
    Cache::getInstance().getState().hbaprotocol().read_schedule(global_blp * MEPHeader::N_POL + 1);

  }
  
  return GCFEvent::HANDLED;
}
