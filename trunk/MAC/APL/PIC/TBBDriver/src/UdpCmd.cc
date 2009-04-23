//#  UdpCmd.cc: implementation of the UdpCmd class
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
#include <Common/StringUtil.h>
#include <netinet/in.h>
#include <net/ethernet.h>

#include "UdpCmd.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;

static const uint16 BASEUDPPORT = 0x7BB0; // (=31664) start numbering src and dst UDP ports at this number
static const uint16 TRANSIENT_FRAME_SIZE = 2140; // bytes, header(88) + payload(2048) + CRC(4)
static const uint16 SUBBANDS_FRAME_SIZE = 2012;  // bytes, header(88) + payload(1920) + CRC(4)

void UdpCmd::string2mac(const char* macstring, uint32 mac[2])
{
	unsigned int hx[ETH_ALEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	sscanf(macstring, "%x:%x:%x:%x:%x:%x", &hx[5], &hx[4], &hx[3], &hx[2], &hx[1], &hx[0]);
	 
	mac[0]	= ((hx[0] & 0xFF))
					+ ((hx[1] & 0xFF) << 8)
					+ ((hx[2] & 0xFF) << 16)
					+ ((hx[3] & 0xFF) << 24);  
	
	mac[1] 	= ((hx[4] & 0xFF)) 
					+ ((hx[5] & 0xFF) << 8);
}

uint32 UdpCmd::string2ip(const char* ipstring)
{
	uint32 result;
	unsigned int hx[sizeof(uint32)] = { 0x00, 0x00, 0x00, 0x00 };

	sscanf(ipstring, "%d.%d.%d.%d", &hx[3], &hx[2], &hx[1], &hx[0]);

	result	= ((hx[0] & 0xFF))
					+ ((hx[1] & 0xFF) << 8)
					+ ((hx[2] & 0xFF) << 16) 
					+ ((hx[3] & 0xFF) << 24);

	return result;
}

void UdpCmd::setup_udpip_header(uint32 boardnr, uint32 ip_hdr[6], uint32 udp_hdr[2])
{
	uint32 iphdr[6];
	uint32 udphdr[2];
	
	uint32 ip_hdr_size = sizeof(iphdr); // bytes
	uint32 udp_hdr_size = sizeof(udphdr); // bytes
	 
	uint32 data_size = 0;
	
	if (itsMode == TBB_MODE_TRANSIENT) data_size = TRANSIENT_FRAME_SIZE;
	if (itsMode == TBB_MODE_SUBBANDS) data_size = SUBBANDS_FRAME_SIZE;	
		
	// IP header values
	uint32 version 				= 4; // IPv4
	uint32 ihl 						= 6; // 6 x uint32
	uint32 tos						= 0;
	uint32 total_length			= ip_hdr_size + udp_hdr_size + data_size;
	uint32 identification		= 0;	
	uint32 flags_offset			= 0x2 << 13;
	uint32 ttl						= 128;
	uint32 protocol				= 0x11;
	uint32 header_checksum		= 0; // set to zero for checksum calculation
	uint32 src_ip_address		= string2ip(TS->getSrcIp(boardnr).c_str());
	uint32 dst_ip_address		= string2ip(TS->getDstIp(boardnr).c_str());   
	uint32 options					= 0; // no options
	// UDP header values  	
	uint32 src_udp_port			= BASEUDPPORT + boardnr;
	uint32 dst_udp_port			= BASEUDPPORT + boardnr;
	uint32 length					= udp_hdr_size + data_size;
	uint32 checksum				= 0; // disable checksum

	// put all ip settings on the correct place
	iphdr[0] = ((version & 0xF) << 28)
						+ ((ihl & 0xF) << 24) 
						+ ((tos & 0xFF) << 16) 
						+ (total_length & 0xFFFF);
	iphdr[1] = ((identification & 0xFFFF) << 16) 
						+ (flags_offset & 0xFFFF);
	iphdr[2] = ((ttl & 0xFF) << 24) 
						+ ((protocol & 0xFF) << 16) 
						+ (header_checksum & 0xFFFF);
	iphdr[3] = src_ip_address;
	iphdr[4] = dst_ip_address;
	iphdr[5] = options;
	
	// compute header checksum
	header_checksum = compute_ip_checksum(&iphdr, 24); //sizeof(ip_hdr));
	iphdr[2] += (uint32)(header_checksum & 0xFFFF); // add checksum
	//LOG_DEBUG_STR(formatString("Checksum = 0x%08X", header_checksum));
		
	// put all udp settings on the correct place
	udphdr[0] = ((src_udp_port & 0xFFFF) << 16) 
						+ (dst_udp_port & 0XFFFF);
	udphdr[1] = ((length & 0xFFFF) << 16) 
						+ (checksum & 0xFFFF);
	
	for (int i = 0; i < 6; i++) { 
		ip_hdr[i] = iphdr[i];
	}
	for (int i = 0; i < 2; i++) { 
		udp_hdr[i] = udphdr[i];
	}
}

uint16 UdpCmd::compute_ip_checksum(void* addr, int count)
{
	// Compute Internet Checksum for "count" bytes
	// beginning at location "addr".
	
	register long sum = 0;

	uint16* addr16 = (uint16*)addr;
	while ( count > 1) {
		//  This is the inner loop 
		sum += *addr16++;
		count -= 2;
	}

	//  Add left-over byte, if any 
	if ( count > 0 )
		sum += * (uint8 *) addr16;

	//  Fold 32-bit sum to 16 bits 
	while (sum>>16)
		sum = (sum & 0xffff) + (sum >> 16);

	return(~sum);
}

//--Constructors for a UdpCmd object.----------------------------------------
UdpCmd::UdpCmd():
		itsMode(0)
{
	TS = TbbSettings::instance();
	
	for (int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) {
		itsStatus[boardnr] = TBB_NO_BOARD;
	}
	setWaitAck(true);	
}

//--Destructor for UdpCmd.---------------------------------------------------
UdpCmd::~UdpCmd() { }

// ----------------------------------------------------------------------------
bool UdpCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_MODE)||(event.signal == TP_UDP_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void UdpCmd::saveTbbEvent(GCFEvent& event)
{
	TBBModeEvent tbb_event(event);
	
	setBoardMask(0xFFF);
	
	for (int boardnr = 0; boardnr < TS->maxBoards(); boardnr++) {
		itsStatus[boardnr] = 0;
		if (!TS->isBoardActive(boardnr)) {
			itsStatus[boardnr] |= TBB_NO_BOARD;
		}
	}

	itsMode = tbb_event.rec_mode;
	for (int ch = 0; ch < TS->maxChannels(); ch++) {
		TS->setChOperatingMode(ch,static_cast<uint8>(itsMode));
	}

	nextBoardNr();	
}

// ----------------------------------------------------------------------------
void UdpCmd::sendTpEvent()
{
	TPUdpEvent tp_event;
	tp_event.opcode = TPUDP;
	tp_event.status = 0;
	
	// fill in destination mac address
	string2mac(TS->getDstMac(getBoardNr()).c_str(),tp_event.mac);
	for ( int i = 0; i < 2; i++) {
		LOG_DEBUG_STR(formatString("MAC[%d]= 0x%08X", i, tp_event.mac[i]));
	}		
	// fill in udp-ip header
	setup_udpip_header(getBoardNr(), tp_event.ip, tp_event.udp);
	for ( int i = 0; i < 6; i++) {
		LOG_DEBUG_STR(formatString("IP[%d]= 0x%08X", i, tp_event.ip[i]));
	}
	for ( int i = 0; i < 2; i++) {
		LOG_DEBUG_STR(formatString("UDP[%d]= 0x%08X", i, tp_event.udp[i]));
	}
	
	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void UdpCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		itsStatus[getBoardNr()] |= TBB_COMM_ERROR;
	}
	else {
		TPUdpAckEvent tp_ack(event);
		
		if ((tp_ack.status >= 0xF0) && (tp_ack.status <= 0xF6)) {
			itsStatus[getBoardNr()] |= (1 << (16 + (tp_ack.status & 0x0F)));
		}
		
		LOG_DEBUG_STR(formatString("Received UdpAck from boardnr[%d], status[0x%08X]", 
											getBoardNr(), tp_ack.status));
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void UdpCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBModeAckEvent tbb_ack;
	
	for (int32 boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) { 
		if (itsStatus[boardnr] == 0) {
			tbb_ack.status_mask[boardnr] = TBB_SUCCESS;
		} else {
			tbb_ack.status_mask[boardnr] = itsStatus[boardnr];
		}
	}

	clientport->send(tbb_ack);
}

