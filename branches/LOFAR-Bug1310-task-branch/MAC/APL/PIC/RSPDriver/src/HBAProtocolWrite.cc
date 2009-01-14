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
#include <Common/hexdump.h>
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
    // The byte sequences originate from the LOFAR-ASTRON-MEM-175 (revision 1.3) Document
    // "HBA Control Design Description"
    //

    //
    // The i2c_protocol and i2c_result arrays must be patched to set
    // the correct server address offsets. This array specifies
    // the indices that should be patched.
    //
    int HBAProtocolWrite::i2c_protocol_patch_indices[] = {
      8, 9,
	  51 + ( 0*17), // stride is 17, server 1
	  51 + ( 1*17),
	  51 + ( 2*17),
	  51 + ( 3*17),
	  51 + ( 4*17),
	  51 + ( 5*17),
	  51 + ( 6*17),
	  51 + ( 7*17),
	  51 + ( 8*17),
	  51 + ( 9*17),
	  51 + (10*17),
	  51 + (11*17),
	  51 + (12*17),
	  51 + (13*17),
	  51 + (14*17),
	  51 + (15*17), // server  16
    };
    int HBAProtocolWrite::i2c_result_patch_indices[] = {
	   4 + ( 0*7), // stride is 7, server 1
	   4 + ( 1*7),
	   4 + ( 2*7),
	   4 + ( 3*7),
	   4 + ( 4*7),
	   4 + ( 5*7),
	   4 + ( 6*7),
	   4 + ( 7*7),
	   4 + ( 8*7),
	   4 + ( 9*7),
	   4 + (10*7),
	   4 + (11*7),
	   4 + (12*7),
	   4 + (13*7),
	   4 + (14*7),
	   4 + (15*7), // server 16
    };

#ifndef HBA_WRITE_DELAYS

    uint8 HBAProtocolWrite::i2c_protocol[HBAProtocolWrite::PROTOCOL_SIZE]
    = {
      // Switch LED on/off
      6,      // PROTOCOL_WRITE_BYTE
      4 >> 1, // Client i2c slave address (shifted right by 1)
      2,      // LED register access command
      0xBB,   // Set LED on/off

      7,      // PROTOCOL_READ_BYTE
      4>>1,   // Client i2c slave address (shifted right by 1)
      2,      // LED register access command

      19,    // PROTOCOL_C_END
    };

     uint8 HBAProtocolWrite::i2c_result[HBAProtocolWrite::RESULT_SIZE] 
     = {
       // Expected protocol result (4 bytes)
       0,    // PROTOCOL_WRITE_BYTE went OK

       0xBB, // Read LED value
       0,    // PROTOCOL_READ_BYTE  went OK

       0,    // PROTOCOL_C_END      went OK
     };

#else

    uint8 HBAProtocolWrite::i2c_protocol[HBAProtocolWrite::PROTOCOL_SIZE/* 42 + 5 + 16 * 17 + 1 = 320 bytes*/] 
    = {
      // Table 32 t/m 37 from LOFAR-ASTRON-MEM-175 (Revision 1.3)

      // Table 32
      // Instruct client to do a broadcast access to the 16 HBA delay servers (42 bytes)
      13,   // PROTOCOL_C_WRITE_BLOCK_NO_CNT
      4>>1, // Client i2c slave address (shifted right by 1)
      0,    // Request register access command
      38,   // Count of number of data bytes to write excluding this byte
      0,    // Broadcast server address
      37,   // Payload length icluding this octet
      4,    // Function: set word
      0,    // Server X-delay register ID
      0,    // First server (initialization will add offset)
      15,   // Last server
      0xBB, // X-delay data for server 0 (offset = PROTOCOL_DELAY_OFFSET) 
      0xBB, // Y-delay data for server 0 
      0xBB, // X-delay data for server 1
      0xBB, // Y-delay data for server 1
      0xBB, // X-delay data for server 2
      0xBB, // Y-delay data for server 2
      0xBB, // X-delay data for server 3
      0xBB, // Y-delay data for server 3
      0xBB, // X-delay data for server 4
      0xBB, // Y-delay data for server 4
      0xBB, // X-delay data for server 5
      0xBB, // Y-delay data for server 5
      0xBB, // X-delay data for server 6
      0xBB, // Y-delay data for server 6
      0xBB, // X-delay data for server 7
      0xBB, // Y-delay data for server 7
      0xBB, // X-delay data for server 8
      0xBB, // Y-delay data for server 8
      0xBB, // X-delay data for server 9
      0xBB, // Y-delay data for server 9
      0xBB, // X-delay data for server 10
      0xBB, // Y-delay data for server 10
      0xBB, // X-delay data for server 11
      0xBB, // Y-delay data for server 11
      0xBB, // X-delay data for server 12
      0xBB, // Y-delay data for server 12
      0xBB, // X-delay data for server 13
      0xBB, // Y-delay data for server 13
      0xBB, // X-delay data for server 14
      0xBB, // Y-delay data for server 14
      0xBB, // X-delay data for server 15
      0xBB, // Y-delay data for server 15

      // Table 33
      // Wait to allow for the broadcast to finish (5 bytes)
      18, // PROTOCOL_C_WAIT
      0x80, 0x96, 0x98, 0x00, // 0x00989680 = 10e6 * 5ns = 50ms

      // Table 34
      // Instruct the client to do a unicast read request to server 0 (8 bytes)
      13,   // PROTOCOL_C_WRITE_BLOCK_NO_CNT
      4>>1, // Client i2c slave address
      0,    // Request register access command
      4,    // Count of number of data bytes to write
      0, // Server address 0
      3,    // Payload length, including this octet
      5,    // Function: get word (document LOFAR-ASTRON-MEM-175 Revision 1.3 has an error here (value 2))
      0,    // Server X-delay register ID
      
      // Table 35
      // Wait to allow for the multicast to finish
      18, // PROTOCOL_C_WAIT (5 bytes)
      0x00, 0x09, 0x3D, 0x00, // 0x003D0900 = 4e6 * 5ns = 20ms pause between write request register and read response register

      // Table 36
      // Read back the response results from the client (4 bytes)
      14,   // PROTOCOL_C_READ_BLOCK_NO_CNT
      4>>1, // Client i2c slave address
      1,    // Response register access command
      4,    // Count of number of data bytes to read (document LOFAR-ASTRON-MEM-175 has an error here (value 3))

      // Repeat tables 34,35,36 fo servers 1 to 15
      13, 4>>1, 0, 4,  1, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 1
      13, 4>>1, 0, 4,  2, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 2
      13, 4>>1, 0, 4,  3, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 3
      13, 4>>1, 0, 4,  4, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 4
      13, 4>>1, 0, 4,  5, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 5
      13, 4>>1, 0, 4,  6, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 6
      13, 4>>1, 0, 4,  7, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 7
      13, 4>>1, 0, 4,  8, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 8
      13, 4>>1, 0, 4,  9, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 9
      13, 4>>1, 0, 4, 10, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 10
      13, 4>>1, 0, 4, 11, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 11
      13, 4>>1, 0, 4, 12, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 12
      13, 4>>1, 0, 4, 13, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 13
      13, 4>>1, 0, 4, 14, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 14
      13, 4>>1, 0, 4, 15, 3, 5, 0, 18, 0x00, 0x09, 0x3D, 0x00, 14, 4>>1, 1, 4, // server 15

      // Mark the end of the protocol list (1 byte)
      19, // PROTOCOL_C_END
    };

    uint8 HBAProtocolWrite::i2c_result[HBAProtocolWrite::RESULT_SIZE/* 2 + 7*16 + 1 = 115 bytes*/] 
    = {
      0, // PROTOCOL_C_WRITE_BLOCK_NO_CNT went OK

      0, // PROTOCOL_C_WAIT went OK

      // Expected protocol result (7 bytes)
      0,       // PROTOCOL_C_WRITE_BLOCK_NO_CNT went OK
      0,       // PROTOCOL_C_WAIT_WENT_OK
      0 + 128, // Server response address (> 0, indicating OK)
      3,       // Nof bytes (including this byte)
      0xBB,    // X-delay data from server X (offset = RESULT_DELAY_OFFSET)
      0xBB,    // Y-delay data from server X
      0,       // PROTOCOL_C_READ_BLOCK_NO_CNT went OK

      0, 0,  1 + 128, 3, 0xBB, 0xBB, 0, // server  1 result
      0, 0,  2 + 128, 3, 0xBB, 0xBB, 0, // server  2 result
      0, 0,  3 + 128, 3, 0xBB, 0xBB, 0, // server  3 result
      0, 0,  4 + 128, 3, 0xBB, 0xBB, 0, // server  4 result
      0, 0,  5 + 128, 3, 0xBB, 0xBB, 0, // server  5 result
      0, 0,  6 + 128, 3, 0xBB, 0xBB, 0, // server  6 result
      0, 0,  7 + 128, 3, 0xBB, 0xBB, 0, // server  7 result
      0, 0,  8 + 128, 3, 0xBB, 0xBB, 0, // server  8 result
      0, 0,  9 + 128, 3, 0xBB, 0xBB, 0, // server  9 result
      0, 0, 10 + 128, 3, 0xBB, 0xBB, 0, // server 10 result
      0, 0, 11 + 128, 3, 0xBB, 0xBB, 0, // server 11 result
      0, 0, 12 + 128, 3, 0xBB, 0xBB, 0, // server 12 result
      0, 0, 13 + 128, 3, 0xBB, 0xBB, 0, // server 13 result
      0, 0, 14 + 128, 3, 0xBB, 0xBB, 0, // server 14 result
      0, 0, 15 + 128, 3, 0xBB, 0xBB, 0, // server 15 result

      // Marks the end of the protocol list (1 byte)
      0,    // PROTOCOL_C_END went OK
    };

#endif

    int HBAProtocolWrite::m_on_off = 0; // default LED is on

  };
};

#define N_WRITES 2 // 2 writes, one for protocol register, one to clear results register

HBAProtocolWrite::HBAProtocolWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard() * N_WRITES)
  // using nrBlpsPerBoard() because only the Y RCU's (odd numbered 1,3,5,...)
  // control both the X and Y delays of HBA
{
	static	bool	i2c_tables_patched = false;

	memset(&m_hdr, 0, sizeof(MEPHeader));

#ifdef HBA_WRITE_DELAYS
	LOG_INFO_STR("HBAProtocolWrite: " << board_id);
	if (!i2c_tables_patched) {
		// add the hba server address offset to specified indices
		int		offset(GET_CONFIG("RSPDriver.HBA_SERVER_ADDRESS_OFFSET", i));
		LOG_INFO_STR ("Patching i2c tables with offset " << offset);
		// patch protocol table
		for (int i = 0; i < (int)(sizeof(i2c_protocol_patch_indices)/sizeof(int)); i++) {
			i2c_protocol[i2c_protocol_patch_indices[i]] += offset;
		}
		// patch result table
		for (int i = 0; i < (int)(sizeof(i2c_result_patch_indices)/sizeof(int)); i++) {
			i2c_result[i2c_result_patch_indices[i]] += offset;
		}
		i2c_tables_patched = true;
	}
#endif
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
	 || RTC::RegisterState::IDLE != Cache::getInstance().getState().rcuprotocol().get(global_blp * MEPHeader::N_POL + 1)) {
		setContinue(true);
		return;
	}
   
	if (RTC::RegisterState::WRITE != Cache::getInstance().getState().hbaprotocol().get(global_blp * MEPHeader::N_POL)
	 && RTC::RegisterState::WRITE != Cache::getInstance().getState().hbaprotocol().get(global_blp * MEPHeader::N_POL + 1)) {
		Cache::getInstance().getState().hbaprotocol().unmodified(global_blp * MEPHeader::N_POL);
		Cache::getInstance().getState().hbaprotocol().unmodified(global_blp * MEPHeader::N_POL + 1);
		setContinue(true);
		return;
	}

	// delays for at least on HBA need to be written, and the RCUProtocol register is not in use by RCUProtocolWrite

	LOG_INFO_STR("HBAsendrequest: " << getCurrentIndex());
	switch (getCurrentIndex() % N_WRITES) {
	case 0: {
#ifdef HBA_WRITE_DELAYS
		if (PROTOCOL_DELAY_OFFSET > 0) {
			Array<uint8, 2> delays(i2c_protocol + PROTOCOL_DELAY_OFFSET,
									shape(MEPHeader::N_HBA_DELAYS, 2),
									neverDeleteData);

			delays(Range::all(), 0) = Cache::getInstance().getBack().getHBASettings()()(global_blp * MEPHeader::N_POL, Range::all());
			delays(Range::all(), 1) = Cache::getInstance().getBack().getHBASettings()()(global_blp * MEPHeader::N_POL + 1, Range::all());
	
			// copy set delays to i2c_result which is the expected result
			uint8* cur = i2c_result + RESULT_DELAY_OFFSET;
			for (int elem = 0; elem < MEPHeader::N_HBA_DELAYS; elem++){
				*(cur+0) = delays(elem, 0); // X
				*(cur+1) = delays(elem, 1); // Y
				cur += RESULT_DELAY_STRIDE;
			}
		}
#else
		i2c_protocol[PROTOCOL_LED_OFFSET] = (m_on_off ? 0xf : 0x0);
		i2c_result[RESULT_LED_OFFSET] = i2c_protocol[PROTOCOL_LED_OFFSET];
		LOG_INFO(formatString("Switch LED (HBA_blp=%d) %s", global_blp, m_on_off ? "on" : "off"));
#endif
		// create the event
		EPARcuProtocolEvent rcuprotocol;
		rcuprotocol.hdr.set(MEPHeader::RCU_PROTOCOLY_HDR, 1 << (getCurrentIndex() / N_WRITES), MEPHeader::WRITE, sizeof(i2c_protocol));
		rcuprotocol.protocol.setBuffer(i2c_protocol, sizeof(i2c_protocol));

		string tmpbuf;
		hexdump (tmpbuf, i2c_protocol, sizeof(i2c_protocol));
		LOG_INFO_STR("HBA WRITE: " << tmpbuf);
  
		m_hdr = rcuprotocol.hdr; // remember header to match with ack
		getBoardPort().send(rcuprotocol);
	}
	break;

	case 1: {
		EPAWriteEvent rcuresultwrite;

		// set the result register to 0xBB's
		rcuresultwrite.hdr.set(MEPHeader::WRITE, 1 << (getCurrentIndex() / N_WRITES),
		MEPHeader::RCU, MEPHeader::RCU_RESULTY, sizeof(i2c_result), 0);
		uint8 clear[RESULT_SIZE];
		memset(clear, 0xBB, RESULT_SIZE); // clear result
		rcuresultwrite.payload.setBuffer(clear, RESULT_SIZE);

		string tmpbuf;
		hexdump (tmpbuf, clear, sizeof(clear));
		LOG_INFO_STR("HBA RESULT WRITE: " << tmpbuf);

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

LOG_INFO_STR("hba[" << (int)(global_blp) << "]: handleAck");
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
