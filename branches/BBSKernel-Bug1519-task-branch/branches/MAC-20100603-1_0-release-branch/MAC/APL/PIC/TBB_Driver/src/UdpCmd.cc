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
#include <iostream>
#include <fstream>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <netinet/in.h>
#include <net/ethernet.h>

#include "UdpCmd.h"
#include "UdpIpTools.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;

string getMac(char *storage)
{
    char mac[20];
    char line[100];
	char *key;
	char *val;

    ifstream fin("/opt/lofar/etc/StaticMetaData/Storage+MAC.dat", ifstream::in );
	strcpy(mac,"0");
	while (!fin.eof()) {
        fin.getline(line,100);
		if (strlen(line) < 6 || line[0] == '#') { continue; }
        key = strtok (line," ");
        if (strcmp(storage, key) == 0) {
            val = strtok(NULL, " ");
			strcpy(mac,val);
            LOG_DEBUG_STR(formatString("getMac(), storage=%s  mac=%s", key, mac));
			break;
        }
    }
    fin.close();
	return(static_cast<string>(mac));
}

string getIp(char *storage)
{
    char ip[20];
    char line[100];
    char *key;
	char *val;

    ifstream fin("/opt/lofar/etc/StaticMetaData/Storage+MAC.dat", ifstream::in );
	strcpy(ip,"0");
	while (!fin.eof()) {
        fin.getline(line,100);
		if (strlen(line) < 6 || line[0] == '#') { continue; }
        key = strtok (line," ");
        if (strcmp(storage, key) == 0) {
            strtok(NULL, " ");
            val = strtok(NULL, " ");
			strcpy(ip,val);
            LOG_DEBUG_STR(formatString("getIp(), storage=%s  ip=%s", key, ip));
			break;
		}
    }
    fin.close();
	return(static_cast<string>(ip));
}


//--Constructors for a UdpCmd object.----------------------------------------
UdpCmd::UdpCmd():
		itsMode(0), itsSignal(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
}

//--Destructor for UdpCmd.---------------------------------------------------
UdpCmd::~UdpCmd() { }

// ----------------------------------------------------------------------------
bool UdpCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_MODE)
		|| (event.signal == TBB_CEP_STORAGE)
		|| (event.signal == TP_UDP_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void UdpCmd::saveTbbEvent(GCFEvent& event)
{
	if (event.signal == TBB_MODE) {
		TBBModeEvent tbb_event(event);
		setBoards(0xFFF);
		itsMode = tbb_event.rec_mode;
		itsSignal = 1;
	}
	if (event.signal == TBB_CEP_STORAGE) {
		TBBCepStorageEvent tbb_event(event);
		setBoards(tbb_event.boardmask);
		itsSignal = 2;
		for (int bnr = 0; bnr < TS->maxBoards(); bnr++) {
			if (tbb_event.boardmask & (1 << bnr)) {
				// get data from Storage+MAC.dat file
				// get IP and MAC from metadata
				LOG_DEBUG_STR(formatString("Storage node=%s", tbb_event.destination));
				
				string ip = getIp(tbb_event.destination);
				string mac = getMac(tbb_event.destination);
				LOG_DEBUG_STR(formatString("ip=%s", ip.c_str()));
				LOG_DEBUG_STR(formatString("mac=%s", mac.c_str()));
				if (ip.length() == 1 || mac.length() == 1 ) {
					setStatus(bnr, TBB_STORAGE_SELECT_ERROR);
				}
				else {
					TS->setDstIpCep(bnr, ip);
					TS->setDstMacCep(bnr, mac);
				}
				
			}
		}
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void UdpCmd::sendTpEvent()
{
	TPUdpEvent tp_event;
	tp_event.opcode = oc_UDP;
	tp_event.status = 0;
	if (itsMode == 0) {
		itsMode = TS->getChOperatingMode(getBoardNr() * TS->nrChannelsOnBoard());
		if (itsMode == 0) {
			itsMode = TBB_MODE_TRANSIENT;
		}
	}
	// fill in destination mac address
	string2mac(TS->getSrcMacCep(getBoardNr()).c_str(), tp_event.srcmac);
	string2mac(TS->getDstMacCep(getBoardNr()).c_str(), tp_event.dstmac);
	// fill in udp-ip header
	setup_udpip_header(	getBoardNr(),
						itsMode,
						TS->getSrcIpCep(getBoardNr()).c_str(),
						TS->getDstIpCep(getBoardNr()).c_str(),
						tp_event.ip,
						tp_event.udp );

	TS->boardPort(getBoardNr()).send(tp_event);
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void UdpCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}
	else {
		TPUdpAckEvent tp_ack(event);

		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		}
		else {
			int start_channel = getBoardNr() * TS->nrChannelsOnBoard();
			for (int i = 0; i < TS->nrChannelsOnBoard();++i) {
				TS->setChOperatingMode((start_channel + i), itsMode);
			}
		}
		LOG_DEBUG_STR(formatString("Received UdpAck from boardnr[%d]", getBoardNr()));
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void UdpCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	if (itsSignal == 1) {
		TBBModeAckEvent tbb_ack;
		for (int32 i = 0; i < MAX_N_TBBOARDS; i++) {
			tbb_ack.status_mask[i] = getStatus(i);
		}

		if (clientport->isConnected()) { clientport->send(tbb_ack); }
	}
	
	if (itsSignal == 2) {
		TBBCepStorageAckEvent tbb_ack;
		for (int32 i = 0; i < MAX_N_TBBOARDS; i++) {
			tbb_ack.status_mask[i] = getStatus(i);
		}
		if (clientport->isConnected()) { clientport->send(tbb_ack); }
	}
}

