//#  CDOWrite.cc: implementation of the CDOWrite class
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>
#include <netinet/in.h>
#include <net/ethernet.h>

#include "CDOWrite.h"
#include "Cache.h"

#define BASEUDPPORT 0x10FA // (=4346) start numbering src and dst UDP ports at this number (4346)
#define EPA_CEP_OUTPUT_HEADER_SIZE 16 // bytes
#define EPA_CEP_BEAMLET_SIZE sizeof(uint16)

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

void CDOWrite::string2mac(const char* macstring, uint8 mac[ETH_ALEN])
{
  unsigned int hx[ETH_ALEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  sscanf(macstring, "%x:%x:%x:%x:%x:%x", &hx[5], &hx[4], &hx[3], &hx[2], &hx[1], &hx[0]);
   
  for (int i = 0; i < ETH_ALEN; i++) mac[i] = (uint8)(hx[i] & 0xFF);
}

uint32 CDOWrite::string2ip(const char* ipstring)
{
  uint32 result;
  unsigned int hx[sizeof(uint32)] = { 0x00, 0x00, 0x00, 0x00 };

  sscanf(ipstring, "%d.%d.%d.%d", &hx[0], &hx[1], &hx[2], &hx[3]);

  result = (hx[3] & 0xFF)
    + ((hx[2] & 0xFF) << 8)
    + ((hx[1] & 0xFF) << 16) 
    + ((hx[0] & 0xFF) << 24);

  return result;
}

void CDOWrite::setup_udpip_header()
{
  uint32 l_srcip;
  uint32 l_dstip;

  char dstip[64];
  char srcip[64];
  snprintf(srcip,  64, "RSPDriver.SRC_IP_ADDR_%d", getBoardId());
  snprintf(dstip,  64, "RSPDriver.DST_IP_ADDR_%d", getBoardId());

  l_srcip = string2ip(GET_CONFIG_STRING(srcip));
  l_dstip = string2ip(GET_CONFIG_STRING(dstip));

  uint32 payload_size = EPA_CEP_OUTPUT_HEADER_SIZE
    + (GET_CONFIG("RSPDriver.CDO_N_BLOCKS", i) 
       * (MEPHeader::N_PHASEPOL * GET_CONFIG("RSPDriver.CDO_N_BEAMLETS", i)) 
       * EPA_CEP_BEAMLET_SIZE);

  //
  // Setup the UDP/IP header
  //
  m_udpip_hdr.ip.version      = 4; // IPv4
  m_udpip_hdr.ip.ihl          = sizeof(m_udpip_hdr.ip) / sizeof(uint32);
  m_udpip_hdr.ip.dscp         = 0x00;
  m_udpip_hdr.ip.total_length = htons(sizeof(m_udpip_hdr) + payload_size);
  m_udpip_hdr.ip.seqnr        = htons(0);
  //m_udpip_hdr.ip.flags        = 0x2;
  m_udpip_hdr.ip.flags_offset = htons(0x2 << 13);
  m_udpip_hdr.ip.ttl          = 128;
  m_udpip_hdr.ip.protocol     = 0x11; // UDP
  m_udpip_hdr.ip.hdrchksum    = 0; // set to zero for checksum computation
  m_udpip_hdr.ip.srcip        = htonl(l_srcip);
  m_udpip_hdr.ip.dstip        = htonl(l_dstip);
  m_udpip_hdr.ip.options      = 0; // no options

  // compute header checksum
  m_udpip_hdr.ip.hdrchksum = compute_ip_checksum(&m_udpip_hdr.ip, sizeof(m_udpip_hdr.ip));

  m_udpip_hdr.udp.srcport     = htons(BASEUDPPORT + getBoardId());
  m_udpip_hdr.udp.dstport     = htons(BASEUDPPORT + getBoardId());
  m_udpip_hdr.udp.length      = htons(sizeof(m_udpip_hdr.udp) + payload_size);
  m_udpip_hdr.udp.checksum    = htons(0); // disable check summing
}

CDOWrite::CDOWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, MEPHeader::N_CDO_REGISTERS)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
  doAtInit();
}

CDOWrite::~CDOWrite()
{
  /* TODO: delete event? */
}

uint16 CDOWrite::compute_ip_checksum(void* addr, int count)
{
  /* Compute Internet Checksum for "count" bytes
   *         beginning at location "addr".
   */
  register long sum = 0;

  uint16* addr16 = (uint16*)addr;
  while( count > 1 )  {
    /*  This is the inner loop */
    sum += *addr16++;
    count -= 2;
  }

  /*  Add left-over byte, if any */
  if( count > 0 )
    sum += * (uint8 *) addr16;

  /*  Fold 32-bit sum to 16 bits */
  while (sum>>16)
    sum = (sum & 0xffff) + (sum >> 16);

  return ~sum;
}

void CDOWrite::sendrequest()
{
  // skip update if the CDO settings have not been modified
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().cdo().get(getBoardId() * getNumIndices() + getCurrentIndex()))
  {
    Cache::getInstance().getState().cdo().unmodified(getBoardId() * getNumIndices() + getCurrentIndex());
    setContinue(true);
    return;
  }

  switch (getCurrentIndex()) {

  case 0:
    {
      EPACdoSettingsEvent cdo;

      cdo.hdr.set(MEPHeader::CDO_SETTINGS_HDR);

      int output_lane = -1;
      for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {
	char paramname[64];
	snprintf(paramname, 64, "RSPDriver.LANE_%d_BLET_OUT", lane);
	if (getBoardId() == GET_CONFIG(paramname, i)) {
	  output_lane = lane;
	  break;
	}
      }

      cdo.station_id.lane = output_lane;
      cdo.station_id.id = GET_CONFIG("RS.STATION_ID", i);

      // fill in some magic so we recognise these fields easily in tcpdump/ethereal output
      cdo.configuration_id = 0xBBAA;
      cdo.ffi              = 0xDDCC;

      cdo.nof_blocks       = GET_CONFIG("RSPDriver.CDO_N_BLOCKS", i);
      cdo.nof_beamlets     = GET_CONFIG("RSPDriver.CDO_N_BEAMLETS", i);

      if (output_lane >= 0) {
	cdo.control.enable   = 1;
	cdo.control.lane     = output_lane;
	char srcmac[64]; snprintf(srcmac, 64, GET_CONFIG_STRING("RSPDriver.CDO_SRCMAC_FORMAT"),
				  GET_CONFIG("RS.STATION_ID", i), getBoardId());
	char dstmac[64]; snprintf(dstmac, 64, "RSPDriver.DST_MAC_ADDR_%d", getBoardId());
	string2mac(srcmac, cdo.src_mac);
	string2mac(GET_CONFIG_STRING(dstmac), cdo.dst_mac);

	LOG_INFO_STR("SRC_MAC lane=" << output_lane << " rspboard=" << getBoardId()
		     << " srcmac=" << srcmac << " dstmac=" << GET_CONFIG_STRING(dstmac));

	// setup UDP/IP header
	setup_udpip_header();

      } else {
	cdo.control.enable = 0;
	cdo.control.lane   = 0;

	memset(cdo.src_mac, 0, ETH_ALEN);
	memset(cdo.dst_mac, 0, ETH_ALEN);
      }
      cdo.control.ffi      = 0;

      m_hdr = cdo.hdr;
      getBoardPort().send(cdo);
    }
    break;

  case 1:
    {
      LOG_DEBUG("Setting CDO_HEADER");

      EPAWriteEvent write;
      write.hdr.set(MEPHeader::CDO_HEADER_HDR);
      write.payload.setBuffer((void*)&m_udpip_hdr, sizeof(m_udpip_hdr));
      m_hdr = write.hdr;
      getBoardPort().send(write);
    }
    break;

  }
}

void CDOWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult CDOWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("CDOWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("CDOWrite::handleack: invalid ack");
    Cache::getInstance().getState().cdo().write_error(getBoardId() * getNumIndices() + getCurrentIndex());
    return GCFEvent::NOT_HANDLED;
  }

  Cache::getInstance().getState().cdo().write_ack(getBoardId() * getNumIndices() + getCurrentIndex());
  
  return GCFEvent::HANDLED;
}
