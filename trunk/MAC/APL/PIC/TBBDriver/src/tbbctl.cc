//#
//#  tbbctl.cc: command line interface to the TBBDriver
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
//#


#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <GCF/GCF_ServiceInfo.h>

#include <APL/TBB_Protocol/TBB_Protocol.ph>

#include <Common/lofar_iostream.h>
#include <Common/lofar_sstream.h>
#include <getopt.h>

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <Common/lofar_set.h>

#include "tbbctl.h"

using namespace std;
//using namespace blitz;
using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TbbCtl;

//---- ALLOC  ----------------------------------------------------------------
AllocCmd::AllocCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ================================================== allocate memory ====\n"));	
}

//-----------------------------------------------------------------------------
void AllocCmd::send()
{
	TBBAllocEvent event;
	
	for(int boardnr = 0; boardnr < MAX_N_TBBBOARDS; boardnr++) {
		if (getSelected()) event.channelmask[boardnr] = getChannelMask(boardnr); // if select cmd is used
		else event.channelmask[boardnr] = 0xFFFF; // otherwise select all 
	}
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult AllocCmd::ack(GCFEvent& e)
{
	TBBAllocackEvent ack(e);
	
	int32 bnr = 0;
	for (int cnr=0; cnr < getMax(); cnr++) {
			bnr = (int32)(cnr / 16);
			if (ack.status[bnr] & SUCCESS) {
				if (isSelected(cnr) ) {
					logMessage(cout,formatString(
							"Board %2d:  Channel %d: memory allocated",
							bnr, cnr));
				}
			}
			
			else {	
				if (!(ack.status[bnr] & NO_BOARD))
					logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
			}
			
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- CHANNELINFO --------------------------------------------------------------
ChannelInfoCmd::ChannelInfoCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ===================================================== channel info ====\n"));
}

//-----------------------------------------------------------------------------
void ChannelInfoCmd::send()
{
	TBBChannelinfoEvent event;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ChannelInfoCmd::ack(GCFEvent& e)
{
	TBBChannelinfoackEvent ack(e);
	
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMax(); cnr++) {
			bnr = (int32)(cnr / 16);
			if (ack.channelsize[cnr] != 0) {
				if (isSelected(cnr) ) {
					logMessage(cout,formatString(
							"Board %2d:  Input %2d:  Status: %c  Start address: 0x%08X  Size: %u pages",
							bnr, (cnr - (bnr * 16)), (char)ack.channelstatus[cnr], ack.channelstartaddr[cnr], ack.channelsize[cnr]));
				}
			}
			
			else {	
					//logMessage(cout,formatString("Board %2d:  input %2d: not allocated",bnr, (cnr - (bnr * 16))));
			}
			
			oldbnr = bnr;
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- FREE ----------------------------------------------------------------
FreeCmd::FreeCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB =================== discard buffer allocation and disable channels ====\n"));
}

//-----------------------------------------------------------------------------
void FreeCmd::send()
{
	TBBFreeEvent event;
	for(int boardnr = 0; boardnr < MAX_N_TBBBOARDS; boardnr++) {
		event.channelmask[boardnr] = getChannelMask(boardnr);
	}
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult FreeCmd::ack(GCFEvent& e)
{
	TBBFreeackEvent ack(e);
	  
	for (int bnr=0; bnr < MAX_N_TBBBOARDS; bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  buffers discard and channels disabled",
						bnr ));
			}
			
			else {	
				if (!(ack.status[bnr] & NO_BOARD))
					logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
			}
			
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- RECORD ----------------------------------------------------------------
RecordCmd::RecordCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ============================= start recording on selected channels ====\n"));
}

//-----------------------------------------------------------------------------
void RecordCmd::send()
{
	TBBRecordEvent event;
	for(int boardnr = 0; boardnr < MAX_N_TBBBOARDS; boardnr++) {
		event.channelmask[boardnr] = getChannelMask(boardnr);
	}
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult RecordCmd::ack(GCFEvent& e)
{
	TBBRecordackEvent ack(e);
		
	for (int bnr=0; bnr < MAX_N_TBBBOARDS; bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  start recording",
						bnr ));
			}
			else {	
				if (!(ack.status[bnr] & NO_BOARD))
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- STOP -------------------------------------------------------------------
StopCmd::StopCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ============================== stop recording on selected channels ====\n"));	
}

//-----------------------------------------------------------------------------
void StopCmd::send()
{
	TBBStopEvent event;
	for(int boardnr = 0; boardnr < MAX_N_TBBBOARDS; boardnr++) {
		event.channelmask[boardnr] = getChannelMask(boardnr);
	}
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StopCmd::ack(GCFEvent& e)
{
	TBBStopackEvent ack(e);
	  
	for (int bnr=0; bnr < MAX_N_TBBBOARDS; bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  stop recording",
						bnr ));
			}
			else {			
				if (!(ack.status[bnr] & NO_BOARD))
					logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- TRIGCLR ----------------------------------------------------------------
TrigclrCmd::TrigclrCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB =========================== clear trigger on all selected channels ====\n"));
}

//-----------------------------------------------------------------------------
void TrigclrCmd::send()
{
	TBBTrigclrEvent event;
	event.channel = (uint32)getChannel();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TrigclrCmd::ack(GCFEvent& e)
{
	TBBTrigclrackEvent ack(e);
	  
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  clear trigger message",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READ -------------------------------------------------------------------
ReadCmd::ReadCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ==============  transfer data to CEP for all selected channels ====\n"));	
}

//-----------------------------------------------------------------------------
void ReadCmd::send()
{
	TBBReadEvent event;
	event.channel = (uint32)getChannel();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadCmd::ack(GCFEvent& e)
{
	TBBReadackEvent ack(e);
	  
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  tranfering data to CEP",
						bnr ));
			}
			else {			
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- UDP --------------------------------------------------------------------
UdpCmd::UdpCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ===================================== udp configure UDP/IP header ====\n"));
}

//-----------------------------------------------------------------------------
void UdpCmd::send()
{
	TBBUdpEvent event;
	event.boardmask = getMask();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult UdpCmd::ack(GCFEvent& e)
{
	TBBUdpackEvent ack(e);
	  
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  UDP/IP configured",
						bnr ));
			}
			else {			
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- VERSION ----------------------------------------------------------------
VersionCmd::VersionCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ====================================== ID and version information ====\n"));
}

//-----------------------------------------------------------------------------
void VersionCmd::send()
{
  TBBVersionEvent event;
  event.boardmask = getMask();
  itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult VersionCmd::ack(GCFEvent& e)
{
  TBBVersionackEvent ack(e);
	
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2u:  ID=%u ; SW=V%u ; board=V%u ; TP=V%u ; MP0=V%u ; MP1=V%u ; MP2=V%u ; MP3=V%u",
	          bnr,
	          ack.boardid[bnr],
	         	ack.swversion[bnr],
	          ack.boardversion[bnr],
	          ack.tpversion[bnr],
	          ack.mp0version[bnr],
	          ack.mp1version[bnr],
	         	ack.mp2version[bnr],
	         	ack.mp3version[bnr] ));
			}
			else {
					logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));	
      }
  	}
	}
	logMessage(cout,formatString("=============================================================================="));
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- SIZE -------------------------------------------------------------------
SizeCmd::SizeCmd(GCFPortInterface& port) : Command(port)
{
  logMessage(cout,formatString("\n== TBB ==================================== installed memory information ====\n"));	
}

//-----------------------------------------------------------------------------
void SizeCmd::send()
{
  TBBSizeEvent event;
  event.boardmask = getMask();
  itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult SizeCmd::ack(GCFEvent& e)
{
  TBBSizeackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2u:  %d pages of 2048 bytes ; Total= %6.3f GByte",
						bnr,
						ack.npages[bnr],
						(((double)ack.npages[bnr] * 2048.) / (1024. * 1024. * 1024.)) ));
			}
    	else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
      }
  	}
	}
	logMessage(cout,formatString("=============================================================================="));
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- STATUS -----------------------------------------------------------------
StatusCmd::StatusCmd(GCFPortInterface& port) : Command(port)
{
  logMessage(cout,formatString("\n== TBB ============================= voltage and temperature information ====\n"));	
}

//-----------------------------------------------------------------------------
void StatusCmd::send()
{
  TBBStatusEvent event;
  event.boardmask = getMask();
  itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StatusCmd::ack(GCFEvent& e)
{
  TBBStatusackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  P1.2= %4.2fV ; P2.5= %4.2fV ; P5.0= %4.2fV ; Tpcb= %uC ; Ttp= %uC ; Tmp0= %uC ; Tmp1= %uC ; Tmp2= %uC ; Tmp3= %uC",
						bnr,
						//(double)ack.V25[bnr],	
						//(double)ack.V33[bnr],	
						//(double)ack.V12[bnr],	
						((double)ack.V25[bnr] * (2.5 / 192.)),	// MAX6652 pin-2:  2.5 / 192	= 0.0130 / count
						((double)ack.V33[bnr] * (3.3 / 192.)),	// MAX6652 pin-3:  3.3 / 192	= 0.0172 / count
						((double)ack.V12[bnr] * (12. / 192.)),	// MAX6652 pin-1: 12.0 / 192	= 0.0625 / count
						ack.Tpcb[bnr],
						ack.Ttp[bnr],
						ack.Tmp0[bnr],
						ack.Tmp1[bnr],
						ack.Tmp2[bnr],
						ack.Tmp3[bnr] ));
			}
    	else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
      }
  	}
	}
	logMessage(cout,formatString("=============================================================================="));
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- CLEAR -------------------------------------------------------------------
ClearCmd::ClearCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB =============================================== clear in progress ====\n"));
}

//-----------------------------------------------------------------------------
void ClearCmd::send()
{
	TBBClearEvent event;
	event.boardmask = getMask();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ClearCmd::ack(GCFEvent& e)
{
	TBBClearackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  board is cleared",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- RESET -------------------------------------------------------------------
ResetCmd::ResetCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB =============================================== reset in progress ====\n"));
}

//-----------------------------------------------------------------------------
void ResetCmd::send()
{
	TBBResetEvent event;
	event.boardmask = getMask();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ResetCmd::ack(GCFEvent& e)
{
	TBBResetackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  board is reset",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- CONFIG -------------------------------------------------------------------
ConfigCmd::ConfigCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ========================================= reconfigure TP and MP's ====\n"));
}

//-----------------------------------------------------------------------------
void ConfigCmd::send()
{
	TBBConfigEvent event;
	event.boardmask = getMask();
	event.imagenr = itsImage;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ConfigCmd::ack(GCFEvent& e)
{
	TBBConfigackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status[bnr] & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  reconfigured TP and MP's",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status[bnr]).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- ERASEF -------------------------------------------------------------------
ErasefCmd::ErasefCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ===================================================== erase flash ====\n"));	
}

//-----------------------------------------------------------------------------
void ErasefCmd::send()
{
	TBBErasefEvent event;
	event.board = (uint32)getBoard();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ErasefCmd::ack(GCFEvent& e)
{
	TBBErasefackEvent ack(e);
		

    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  flash is erased",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READF -------------------------------------------------------------------
ReadfCmd::ReadfCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ====================================================== read flash ====\n"));
}

//-----------------------------------------------------------------------------
void ReadfCmd::send()
{
	TBBReadfEvent event;
	event.board = (uint32)getBoard();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadfCmd::ack(GCFEvent& e)
{
	TBBReadfackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  flash is read",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- WRITEF -------------------------------------------------------------------
WritefCmd::WritefCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ===================================================== write flash ====\n"));
}

//-----------------------------------------------------------------------------
void WritefCmd::send()
{
	TBBWritefEvent event;
	event.board = (uint32)getBoard();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult WritefCmd::ack(GCFEvent& e)
{
	TBBWritefackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  write flash",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READW -------------------------------------------------------------------
ReadwCmd::ReadwCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ======================================================= read DDR2 ====\n"));
}

//-----------------------------------------------------------------------------
void ReadwCmd::send()
{
	TBBReadwEvent event;
	event.board = (uint32)getBoard();
	event.mp = itsMp;
	event.addr = itsAddr;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadwCmd::ack(GCFEvent& e)
{
	TBBReadwackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  MP[%u] Addr[0x%08X]  [0x%08X] [0x%08X]",
						bnr, itsMp, itsAddr, ack.wordlo, ack.wordhi ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
				itsAddr = itsStopAddr - 1;
			}
		}
	}
	if (itsAddr == (itsStopAddr - 1))
	logMessage(cout,formatString("=============================================================================="));
	
	itsAddr++;
	if (itsAddr >= itsStopAddr) { setCmdDone(true); }
	else { itsPort.setTimer(0.01); }
	return(GCFEvent::HANDLED);
}

//---- WRITEW -------------------------------------------------------------------
WritewCmd::WritewCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n== TBB ====================================================== write DDR2 ====\n"));
}

//-----------------------------------------------------------------------------
void WritewCmd::send()
{
	TBBWritewEvent event;
	event.board = (uint32)getBoard();
	event.mp = itsMp;
	event.addr = itsAddr;
	event.wordlo = itsWordLo;
	event.wordhi = itsWordHi;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult WritewCmd::ack(GCFEvent& e)
{
	TBBWritewackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  DDR2 write SUCCESS",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
			}
		}
	}
	
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READR -------------------------------------------------------------------
ReadrCmd::ReadrCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n==== TBB-board, read register ================================================\n"));	
}

//-----------------------------------------------------------------------------
void ReadrCmd::send()
{
	TBBReadrEvent event;
	event.board = (uint32)getBoard();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadrCmd::ack(GCFEvent& e)
{
	TBBReadrackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  read register",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- WRITER -------------------------------------------------------------------
WriterCmd::WriterCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,formatString("\n==== TBB-board, write register ===============================================\n"));	
}

//-----------------------------------------------------------------------------
void WriterCmd::send()
{
	TBBWriterEvent event;
	event.board = (uint32)getBoard();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult WriterCmd::ack(GCFEvent& e)
{
	TBBWriterackEvent ack(e);
    
	for (int bnr=0; bnr < getMax(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status & SUCCESS) {
				logMessage(cout,formatString(
						"Board %2d:  write register",
						bnr ));
			}
			else {	
				logMessage(cout,formatString("Board %2d: %s",bnr, getErrorStr(ack.status).c_str()));
			}
		}
	}
	logMessage(cout,formatString("=============================================================================="));
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READPAGE ------------------------------------------------------------------
ReadPageCmd::ReadPageCmd(GCFPortInterface& port) : Command(port),
	itsCmdStage(0),itsPage(0),itsMp(0),itsAddr(0),itsPages(1),itsStationId(0),itsRspId(0),itsRcuId(0),itsSampleFreq(0),
	itsTime(0),itsSampleNr(0),itsSamplesPerFrame(0),itsFreqBands(0),itsTotalSamples(0),itsTotalBands(0)
{
	logMessage(cout,formatString("\n==== TBB-board, readx register ================================================\n"));
}
//-----------------------------------------------------------------------------
void ReadPageCmd::send()
{
	
	switch (itsCmdStage) {
		case 0: {
			TBBWriterEvent event0;
			event0.board = (uint32)getBoard();
			event0.mp = itsMp;
			event0.pid = PID6;
			event0.regid = REGID1;
			event0.data[0] = itsAddr;
			event0.data[1] = 0; 
			event0.data[2] = 0;
			itsPort.send(event0);
			
			LOG_DEBUG_STR(formatString("Writer[%x][%x][%x][%x]", 
				event0.mp,event0.pid,event0.regid,event0.data[0]));
		} break;
		
		case 1: {
			TBBWriterEvent event1;
			event1.board = (uint32)getBoard();
			event1.mp = itsMp;
			event1.pid = PID6;
			event1.regid = REGID0;
			event1.data[0] = PAGEREAD;
			event1.data[1] = 0; 
			event1.data[2] = 0;
			itsPort.send(event1);
			
			LOG_DEBUG_STR(formatString("Writer[%x][%x][%x][%x]", 
				event1.mp,event1.pid,event1.regid,event1.data[0]));
		} break;
		
		case 2: {
			TBBReadxEvent event2;
			event2.board = (uint32)getBoard();	
			event2.mp = itsMp;
			event2.pid = PID6;
			event2.regid = REGID2;
			event2.pagelength = 256;
			event2.pageaddr = 0;
			
			itsPort.send(event2);
			LOG_DEBUG_STR(formatString("Readx[%x][%x][%x][%x][%x]", 
				event2.mp,event2.pid,event2.regid,event2.pagelength,event2.pageaddr));
		} break;
		
		case 3: {
			TBBReadxEvent event3;
			event3.board = (uint32)getBoard();	
			event3.mp = itsMp;
			event3.pid = PID6;
			event3.regid = REGID2;
			event3.pagelength = 256;
			event3.pageaddr = 256;
			
			itsPort.send(event3);
			LOG_DEBUG_STR(formatString("Readx[%x][%x][%x][%x][%x]", 
				event3.mp,event3.pid,event3.regid,event3.pagelength,event3.pageaddr));
		} break;
		
		default:{
		} break;
	}
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadPageCmd::ack(GCFEvent& e)
{
	uint errorMask = 0;
	int16 val[1400];
		
	switch (itsCmdStage) {
		case 0: {
			TBBWriterackEvent ack(e);
			if (!(ack.status & SUCCESS)) {
				logMessage(cout,formatString("%s",getErrorStr(ack.status).c_str()));
				itsCmdStage = 10;
			}
		} break;
		
		case 1: {
			TBBWriterackEvent ack(e);
			if (!(ack.status & SUCCESS)) {
				logMessage(cout,formatString("%s",getErrorStr(ack.status).c_str()));
				itsCmdStage = 10;
			}
		} break;
		
		case 2: {
			TBBReadxackEvent ack(e);
			if (!(ack.status & SUCCESS)) {
				logMessage(cout,formatString("%s", getErrorStr(ack.status).c_str()));
				itsCmdStage = 10;
		  }
			for (int32 dn = 0; dn < 256; dn++) { 
				itsData[dn] = ack.pagedata[dn];
			}
		} break;
		
		case 3: {
			TBBReadxackEvent ack(e);
			if (!(ack.status & SUCCESS)) {
				logMessage(cout,formatString("%s", getErrorStr(ack.status).c_str()));
				itsCmdStage = 10;
		  }
			for (int32 dn = 0; dn < 256; dn++) { 
				itsData[256 + dn] = ack.pagedata[dn];
			}
		} break;
	}
	
	// if error in communication stop
	if (itsCmdStage == 10) {
		setCmdDone(true);
		return(GCFEvent::HANDLED);
	}
	
	itsCmdStage++;
	if (itsCmdStage < 4) {
		itsPort.setTimer(0.01);
	}
	else { 
		//logMessage(cout,formatString("=============================================================================="));
				
		if (itsPage == 0) {
			itsStationId = (int)(itsData[0] & 0xFF);
			itsRspId = (int)((itsData[0] >> 8) & 0xFF);
			itsRcuId = (int)((itsData[0] >> 16) & 0xFF);
			itsSampleFreq = (int)((itsData[0] >> 24) & 0xFF);
			itsTime = (time_t)itsData[2];
			itsSampleNr = (int)itsData[3];
		}
		itsSamplesPerFrame = (int)(itsData[4] & 0xFFFF);
		itsFreqBands = (int)((itsData[4] >> 16) & 0xFFFF);
		itsTotalSamples += itsSamplesPerFrame;
		itsTotalBands += itsFreqBands; 
		
				
		int sample_cnt = 0;
		int val_cnt = 0;
		int data_cnt = 22; // 22 = startadress of data in frame
				
		if (itsFreqBands > 0) {
			// its SPECTRAL data

			// convert uint32 to complex int16
			while (sample_cnt < itsSamplesPerFrame) {
				// get complex sample
				val[val_cnt++] = (int16) (itsData[data_cnt] & 0xFFFF);	// re part
				val[val_cnt++] = (int16)((itsData[data_cnt++] >> 16) & 0xFFFF);	// im part
				sample_cnt++;
			}
		}
		else {
			// its RAW data

			// convert uint32 to int12
			uint32 data[3];
			while (sample_cnt < itsSamplesPerFrame) {
				// get 96 bits from received data
				data[0] = itsData[data_cnt++];
				data[1] = itsData[data_cnt++];
				data[2] = itsData[data_cnt++];
				
				// extract 8 values of 12 bit
				val[val_cnt++] = (int16)  (data[0] & 0x00000FFF);
				val[val_cnt++] = (int16) ((data[0] & 0x00FFF000) >> 12);
				val[val_cnt++] = (int16)(((data[0] & 0xFF000000) >> 24) | ((data[1] & 0x0000000F) << 8));
				val[val_cnt++] = (int16) ((data[1] & 0x0000FFF0) >> 4 );
				val[val_cnt++] = (int16) ((data[1] & 0x0FFF0000) >> 16);
				val[val_cnt++] = (int16)(((data[1] & 0xF0000000) >> 28) | ((data[2] & 0x000000FF) << 4));
				val[val_cnt++] = (int16) ((data[2] & 0x000FFF00) >> 8);
				val[val_cnt++] = (int16) ((data[2] & 0xFFF00000) >> 20);
				
				sample_cnt += 8;
			}
			
			// convert all received samples from signed 12bit to signed 16bit
			for (int cnt = 0; cnt < val_cnt; cnt++) {
				if (val[cnt] & 0x0800) val[cnt] |= 0xF000;
			}
		}
		
		// write all data to file
		FILE* file;
		char line[10][256];
		char basefilename[PATH_MAX];
		char filename[PATH_MAX];
		char timestring[256];
		
		strftime(timestring, 255, "%Y%m%d_%H%M%S", gmtime(&itsTime));
		snprintf(basefilename, PATH_MAX, "%s_%02d%02d", timestring,itsStationId,itsRcuId);
		
		snprintf(filename, PATH_MAX, "%s.dat",basefilename);
		file = fopen(filename,"a");
		fwrite(val,sizeof(int16),val_cnt,file);
		fclose(file);
		
		itsPage++;
		if ((itsCmdStage == 4) && (itsPage < itsPages)) {
			itsCmdStage = 0;
			itsAddr++;
			itsPort.setTimer(0.01);
		}
		else {
			// print page information
			strftime(timestring, 255, "%Y-%m-%d  %H:%M:%S", gmtime(&itsTime));
			
			sprintf(line[0],"Station ID      : %d",itsStationId);
			sprintf(line[1],"RSP ID          : %d",itsRspId);
			sprintf(line[2],"RCU ID          : %d",itsRcuId);
			sprintf(line[3],"Sample freq     : %d MHz",itsSampleFreq);
			if (itsTime < 0)
				sprintf(line[4],"Time            : invalid");
			else
				sprintf(line[4],"Time            : %s",timestring);
			sprintf(line[5],"SampleNr        : %u",itsSampleNr);
			if (itsTotalBands) {
				sprintf(line[6],"FreqBands       : %u",itsTotalBands);
				sprintf(line[7],"Data file format: binary complex(int16 Re, int16 Im)");
			}
			else	 {
				sprintf(line[6],"Samples         : %u",itsTotalSamples);
				sprintf(line[7],"Data file format: binary  int16");
			}
			sprintf(line[8],"Filename        : %s.nfo",basefilename);
			sprintf(line[9],"                : %s.dat",basefilename);
						
			snprintf(filename, PATH_MAX, "%s.nfo",basefilename);
			file = fopen(filename,"w");		
									
			for (int32 lnr = 0;lnr < 10; lnr++) {
				logMessage(cout,line[lnr]);
				fprintf(file,line[lnr]);
				fprintf(file,"\n");
			}
			
			
			logMessage(cout,formatString("=============================================================================="));
			
			fclose(file);
			setCmdDone(true);
		}                                                                                               
	}
	return(GCFEvent::HANDLED);
}

//====END OF TBB COMMANDS========================================================================== 


//---- HELP --------------------------------------------------------------------
void TBBCtl::help()
{
	logMessage(cout,"\n==== tbbctl command usage ===================================================================\n");
	
	logMessage(cout,"# --command          : all boards or channels are selected, and will be displayed");
	logMessage(cout,"# --command --select : information for all selected boards is displayed\n"
									"#    Example: --select=0,1,4  or  --select=0:6  or  --select=0,1,2,8:11\n");
	logMessage(cout,"tbbctl --alloc [--select=]                  # allocate memmory locations for selected channels");
	logMessage(cout,"tbbctl --free [--select=]                   # free memmory locationsfor selected channels");
	logMessage(cout,"tbbctl --record [--select=]                 # start recording on selected channels");
	logMessage(cout,"tbbctl --stop [--select=]                   # stop recording on all selected channels");
	logMessage(cout,"tbbctl --trigclr=channel                    # clear tigger message for all selected channel");
	//logMessage(cout,"tbbctl --read=channel                       # transfer recorded data to CEP");
	logMessage(cout,"tbbctl --udp [--select=]                    # configure UDP/IP header for Data transport to CEP");
	logMessage(cout,"tbbctl --version [--select=]                # get version information");
	logMessage(cout,"tbbctl --status [--select=]                 # get board status");	
	logMessage(cout,"tbbctl --size [--select=]                   # get installed memory size");
	//logMessage(cout,"tbbctl --eraseflash=addr                    # erase flash page starting on addr");
	//logMessage(cout,"tbbctl --readflash=addr                     # read flash page starting on addr");
	//logMessage(cout,"tbbctl --writeflash=addr                    # write flash page starting on addr");
	logMessage(cout,"tbbctl --readddr=board,mp,addr,size         # read 2 words from DDR2 memory");
	logMessage(cout,"tbbctl --writeddr=board,mp,addr,wordL,wordH # write 2 words to DDR2 memory at addr");
	logMessage(cout,"tbbctl --readpage=board,mp,addr,pages       # read n pages from DDR2 memory starting at addr");
	logMessage(cout,"tbbctl --clear [--select=]                  # clear board");
	logMessage(cout,"tbbctl --reset [--select=]                  # reset factory images");
	logMessage(cout,"tbbctl --config=imagenr [--select=]         # reconfigure TP and MP's with imagenr [0 .. 31]");
	logMessage(cout,"tbbctl --help                               # this help screen");
	logMessage(cout,"=============================================================================================");
}
//-----------------------------------------------------------------------------

//====END OF COMMANDS==========================================================================
//-----------------------------------------------------------------------------
TBBCtl::TBBCtl(string name, int argc, char** argv): GCFTask((State)&TBBCtl::initial, name),
	itsCommand(0),itsActiveBoards(0),itsMaxBoards(0),itsMaxChannels(0),itsArgc(argc),itsArgv(argv)
{
	for(int boardnr = 0; boardnr < MAX_N_TBBBOARDS; boardnr++) {
		itsMemory[boardnr] = 0;
	}
	registerProtocol(TBB_PROTOCOL, TBB_PROTOCOL_signalnames);
	itsServerPort.init(*this, MAC_SVCMASK_TBBDRIVER, GCFPortInterface::SAP, TBB_PROTOCOL);
}

//-----------------------------------------------------------------------------
TBBCtl::~TBBCtl()
{
  if (itsCommand) {
  	delete itsCommand;
  }
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TBBCtl::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case F_INIT: {
  	} break;

    case F_ENTRY: {
    	if (!itsServerPort.isConnected()) {
      	itsServerPort.open();
      	itsServerPort.setTimer((long)1);
      }
    } break;

    case F_CONNECTED: {
      if (itsServerPort.isConnected()) {
        itsServerPort.cancelAllTimers();
        TBBGetconfigEvent getconfig;
        itsServerPort.send(getconfig);
       }
    } break;

    case TBB_GETCONFIGACK: {
      TBBGetconfigackEvent ack(e);
      itsMaxBoards		= ack.max_boards;
      itsActiveBoards	= ack.active_boards;
			itsMaxChannels = itsMaxBoards * 16;
      //logMessage(cout,formatString("\nMax nr of TBB boards = %d",itsMaxBoards));
			//logMessage(cout,formatString("Max nr of Channels   = %d\n",itsMaxChannels));
			//logMessage(cout,formatString("Active boards mask   = 0x%03X",itsActiveBoards));
			
      TRAN(TBBCtl::docommand);
    } break;

    case F_DISCONNECTED: {
      //port.setTimer((long)1);
      port.close();
    } break;

    case F_TIMER: {
      // try again
      logMessage(cout,"========================================");
      logMessage(cout,"  TBBDriver is not responding           ");
      logMessage(cout,"========================================");
      //itsServerPort.open();
      exit(EXIT_FAILURE);
    } break;

    default: {
      status = GCFEvent::NOT_HANDLED;
    } break;
  }

  return(status);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TBBCtl::docommand(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case F_INIT: {
  	} break;
  	
    case F_ENTRY: {
	  	// reparse options
      itsCommand = parse_options(itsArgc, itsArgv);
      if (itsCommand == 0) {
        logMessage(cerr,"Warning: no command specified.");
        exit(EXIT_FAILURE);
      }
      itsCommand->send();
    } break;

    case F_CONNECTED: {
    } break;
    
    case F_DISCONNECTED: {
      port.close();
      logMessage(cerr,formatString("Error: port '%s' disconnected.",port.getName().c_str()));
      exit(EXIT_FAILURE);
    } break;
		
		case F_TIMER: {
			itsCommand->send();
		} break;
    
    case TBB_ALLOCACK:
    case TBB_CHANNELINFOACK:
    case TBB_FREEACK:
    case TBB_RECORDACK:
    case TBB_STOPACK:
    case TBB_TRIGCLRACK:
    case TBB_READACK:
    case TBB_UDPACK:
    case TBB_VERSIONACK:
    case TBB_SIZEACK:
    case TBB_STATUSACK: 
    case TBB_CLEARACK:
    case TBB_RESETACK:
    case TBB_CONFIGACK:
    case TBB_ERASEFACK:
    case TBB_READFACK:
    case TBB_WRITEFACK:
    case TBB_READWACK:
    case TBB_WRITEWACK:
    case TBB_READRACK:
    case TBB_WRITERACK:
    case TBB_READXACK: {	
    	status = itsCommand->ack(e); // handle the acknowledgement
    	if (itsCommand->isCmdDone()) {
    		GCFTask::stop();
    	}
    	//else {
    	//	port.setTimer(0.02);	
    	//}
    } break;

    default: {
      logMessage(cerr,"Error: unhandled event.");
      logMessage(cerr,formatString("Error: unhandled event. %d",e.signal));
      GCFTask::stop();
    } break;
  }

  return(status);
}

//-----------------------------------------------------------------------------
Command* TBBCtl::parse_options(int argc, char** argv)
{
  Command*	command = 0;
  list<int> select;
  
  optind = 0; // reset option parsing
  //opterr = 0; // no error reporting to stderr
  
  while(1) {
  	static struct option long_options[] = {
			{ "select",			required_argument,	0,	'l' }, //ok
			{ "alloc",			no_argument,				0,	'a' }, //ok ??
			{ "channelinfo",no_argument,				0,	'i' }, //ok ??
			{ "free",				no_argument,				0,	'f' }, //ok ??
		  { "record",			no_argument,				0,	'r' }, //ok ??
		  { "stop",				no_argument,				0,	's' }, //ok ??
		  { "trigclr",		required_argument,	0,	't' }, //ok ??
			{ "read",				required_argument,	0,	'R' }, //ok ??
			{ "udp",				required_argument,	0,	'u' }, //ok ??
			{ "version",		no_argument,				0,	'v' }, //ok
			{ "size",				no_argument,				0,	'z' }, //ok
			{ "status",			no_argument,				0,	'A' }, //ok
			{ "readpage",		required_argument,	0,	'p' }, //ok ??
			{ "clear",			no_argument,				0,	'C' }, //ok ??
			{ "reset",			no_argument,				0,	'Z' }, //ok ??
			{ "config",			required_argument,	0,	'S' }, //ok ??
		  { "eraseflash",	required_argument,	0,	'1' }, //ok
		  { "readflash",	required_argument,	0,	'2' }, //ok
		  { "writeflash",	required_argument,	0,	'3' }, //ok
		  { "readddr",		required_argument,	0,	'4' }, //ok
		  { "writeddr",		required_argument,	0,	'5' }, //ok
		  { "readreg",		required_argument,	0,	'6' }, //ok not in help
		  { "writereg",		required_argument,	0,	'7' }, //ok not in help
			{ "help",				no_argument,				0,	'h' }, //ok ??
		  { 0, 					  0, 									0,	0 },
		};

    int option_index = 0;
    int c = getopt_long(argc, argv,
				"l:afrst:R:u:vzAp:CZS:1:2:3:4:5:6:7:h",
				long_options, &option_index);

    if (c == -1)
			break;

		switch (c) {
			
			case 'l': { 	// --select
	  		if (!command->getSelected()) {
  	  		if (optarg) {
  	      	if (!command) {
  		  			logMessage(cerr,"Error: 'command' argument should come before --select argument");
  		  			exit(EXIT_FAILURE);
  					}
  					select = strtolist(optarg, command->getMax());
  											
  	      	if (select.empty()) {
  		  			logMessage(cerr,"Error: invalid or missing '--select' option");
  		  			exit(EXIT_FAILURE);
  					}
  					else {
  						command->setSelected(true);
  					}
  	    	}
  	  		else {
  	      	logMessage(cerr,"Error: option '--select' requires an argument");
  	      	exit(EXIT_FAILURE);
  	      }
	      }
	      else
	      	logMessage(cerr,"Error: channels already selected");
	    } break;
	    
	    case 'a': { 	// --alloc
				if (command) delete command;
				AllocCmd* alloccmd = new AllocCmd(itsServerPort);
				command = alloccmd;
				command->setMax(itsMaxChannels);
			}	break;
			
			case 'i': { 	// --channelinfo
				if (command) delete command;
				ChannelInfoCmd* channelinfocmd = new ChannelInfoCmd(itsServerPort);
				command = channelinfocmd;
				command->setMax(itsMaxChannels);
			}	break;
			
			case 'f': { 	// --free
				if (command) delete command;
				FreeCmd* freecmd = new FreeCmd(itsServerPort);
				command = freecmd;
				command->setMax(itsMaxChannels);
			}	break;
			
			case 'r': { 	// --record
				if (command) delete command;
				RecordCmd* recordcmd = new RecordCmd(itsServerPort);
				command = recordcmd;
				command->setMax(itsMaxChannels);
			}	break;
			
			case 's': { 	// --stop
				if (command) delete command;
				StopCmd* stopcmd = new StopCmd(itsServerPort);
				command = stopcmd;
				command->setMax(itsMaxChannels);
			}	break;
			
			case 't': { 	// --trigclear
				if (command) delete command;
				TrigclrCmd* trigclrcmd = new TrigclrCmd(itsServerPort);
				command = trigclrcmd;
				if (optarg) {
					int channel = 0;
					int numitems = sscanf(optarg, "%d",&channel);
					if (numitems == 0 || numitems == EOF) {
						logMessage(cerr,"Error: invalid number of arguments. Should be of the format "
								"'--trigclr=channel]' ");  
						exit(EXIT_FAILURE);
					}
					select.clear();
		  		select.push_back(channel);
					command->setSelected(true);
				}
				
				command->setMax(itsMaxChannels);
			}	break;
			
			case 'R': { 	// --read
				if (command) delete command;
				ReadCmd* readcmd = new ReadCmd(itsServerPort);
				command = readcmd;
				command->setMax(itsMaxChannels);
			}	break;
			
			case 'u': { 	// --udp
				if (command) delete command;
				UdpCmd* udpcmd = new UdpCmd(itsServerPort);
				command = udpcmd;
				/*
				if (optarg) {
					uint32 imagenr = 0;
					int numitems = sscanf(optarg, "%u", &imagenr);
					if (numitems == 0 || numitems == EOF || imagenr < 0 || imagenr > 31) {
						logMessage(cerr,"Error: invalid image value. Should be of the format "
								"'--config=value' where value is a int value in the range [0,10].");  //TODO value range
						exit(EXIT_FAILURE);
					}
					udpcmd->setImage(imagenr);
				}
				*/
				command->setMax(itsMaxChannels);
			}	break;									
						
			case 'v': { 	// --version
	  		if (command) delete command;
	    	VersionCmd* versioncmd = new VersionCmd(itsServerPort);
	  		command = versioncmd;
				command->setMax(itsMaxBoards);
	  	}	break;	
	  	
	  	case 'z': { 	// --size
	  		if (command) delete command;
	    	SizeCmd* sizecmd = new SizeCmd(itsServerPort);
	  		command = sizecmd;
				command->setMax(itsMaxBoards);
	  	}	break;
	  	
	  	case 'A': { 	// --status
	  		if (command) delete command;
	    	StatusCmd* statuscmd = new StatusCmd(itsServerPort);
	  		command = statuscmd;
				command->setMax(itsMaxBoards);
	  	}	break;
			
			case 'C': { 	// --clear
				if (command) delete command;
				ClearCmd* clearcmd = new ClearCmd(itsServerPort);
				command = clearcmd;
				command->setMax(itsMaxBoards);
			}	break;
			
			case 'Z': { 	// --reset
				if (command) delete command;
				ResetCmd* resetcmd = new ResetCmd(itsServerPort);
				command = resetcmd;
				command->setMax(itsMaxBoards);
			}	break;
			
			case 'S': { 	// --config
				if (command) delete command;
				ConfigCmd* configcmd = new ConfigCmd(itsServerPort);
				command = configcmd;
				
				if (optarg) {
					int32 imagenr = 0;
					int numitems = sscanf(optarg, "%d", &imagenr);
					if (numitems == 0 || numitems == EOF || imagenr < 0 || imagenr > 31) {
						logMessage(cerr,"Error: invalid image value. Should be of the format "
								"'--config=value' where value is a int value in the range [0,10].");  //TODO value range
						exit(EXIT_FAILURE);
					}
					configcmd->setImage((uint32)imagenr);
				}
				command->setMax(itsMaxBoards);
			}	break;
			
			case 'p': { 	// --readpage
				if (command) delete command;
				ReadPageCmd* readddrcmd = new ReadPageCmd(itsServerPort);
				command = readddrcmd;
				
				if (optarg) {
					int32 board = 0;
					uint32 mp = 0;
					uint32 addr = 0;
					uint32 pages = 0;
					
					int numitems = sscanf(optarg, "%d,%u,%u,%u", &board,&mp,&addr,&pages);
					
					if (numitems < 3 || numitems == EOF || board < 0 || board > 11 || mp > 3) {
						logMessage(cerr,"Error: invalid read ddr value. Should be of the format "
								"'--readw=board,mp,addr' where board= 0..11, mp= 0..3 and addr= 0x..");  
						exit(EXIT_FAILURE);
					}
					readddrcmd->setMp(mp);
					readddrcmd->setAddr(addr);
					readddrcmd->setPages(pages);
					select.clear();
		  		select.push_back(board);
		  		command->setSelected(true);
				}
				else {
					logMessage(cerr,"Error: invalid read ddr value. Should be of the format "
								"'--readw=board,mp,addr' where board= 0..11, mp= 0..3 and addr= 0x..");  
						exit(EXIT_FAILURE);
				}
				command->setMax(itsMaxBoards);
			}	break;			
						
			case '1': { 	// --erasef
				if (command) delete command;
				ErasefCmd* erasefcmd = new ErasefCmd(itsServerPort);
				command = erasefcmd;
				command->setMax(itsMaxBoards);
			}	break;
			
			case '2': { 	// --readf
				if (command) delete command;
				ReadfCmd* readfcmd = new ReadfCmd(itsServerPort);
				command = readfcmd;
				command->setMax(itsMaxBoards);
			}	break;
			
			case '3': { 	// --writef
				if (command) delete command;
				WritefCmd* writefcmd = new WritefCmd(itsServerPort);
				command = writefcmd;
				command->setMax(itsMaxBoards);
			}	break;
			
			case '4': { 	// --readw
				if (command) delete command;
				ReadwCmd* readwcmd = new ReadwCmd(itsServerPort);
				command = readwcmd;
				
				if (optarg) {
					int32 board = 0;
					uint32 mp = 0;
					uint32 startaddr = 0;
					uint32 size = 0;
					
					int numitems = sscanf(optarg, "%d,%x,%x,%x", &board,&mp,&startaddr, &size);
					
					if (numitems < 3 || numitems == EOF || board < 0 || board > 11 || mp > 3) {
						logMessage(cerr,"Error: invalid read ddr value. Should be of the format "
								"'--readw=board,mp,addr' where board= 0..11, mp= 0..3 and addr= 0x..");  
						exit(EXIT_FAILURE);
					}
					readwcmd->setMp(mp);
					readwcmd->setStartAddr(startaddr);
					readwcmd->setStopAddr(startaddr+size);
					select.clear();
		  		select.push_back(board);
		  		command->setSelected(true);
				}
				else {
					logMessage(cerr,"Error: invalid read ddr value. Should be of the format "
								"'--readw=board,mp,addr' where board= 0..11, mp= 0..3 and addr= 0x..");  
						exit(EXIT_FAILURE);
				}
				command->setMax(itsMaxBoards);
			}	break;
			
			case '5': { 	// --writew
				if (command) delete command;
				WritewCmd* writewcmd = new WritewCmd(itsServerPort);
				command = writewcmd;
				if (optarg) {
					int32 board = 0;
					int32 mp = 0;
					uint32 addr = 0;
					uint32 wordlo = 0;
					uint32 wordhi = 0;
					
					int numitems = sscanf(optarg, "%d,%d,%x,%x,%x", &board,&mp,&addr,&wordlo,&wordhi);
					if (numitems < 5 || numitems == EOF || board < 0 || board > 11 || mp > 3) {
						logMessage(cerr,"Error: invalid write ddr value. Should be of the format "
								"'--writew=board,mp,addr,wordlo,wordhi' where board= 0..11, mp= 0..3 and\n"
								"addr= 0x.., wordlo= 0x.., wordhi= 0x..");  
						exit(EXIT_FAILURE);
					}
					writewcmd->setMp(mp);
					writewcmd->setAddr(addr);
					writewcmd->setWordLo(wordlo);
					writewcmd->setWordHi(wordhi);
					select.clear();
		  		select.push_back(board);
		  		command->setSelected(true);
				}
				else {
					logMessage(cerr,"Error: invalid write ddr value. Should be of the format "
								"'--writew=board,mp,addr,wordlo,wordhi' where board= 0..11, mp= 0..3 and addr= 0..x.");  
						exit(EXIT_FAILURE);
				}
				command->setMax(itsMaxBoards);
			}	break;
			/*
			case '6': { 	// --readr
				if (command) delete command;
				ReadrCmd* readrcmd = new ReadrCmd(itsServerPort);
				command = readrcmd;
				command->setMax(itsMaxBoards);
			}	break;
			
			case '7': { 	// --writer
				if (command) delete command;
				WriterCmd* writercmd = new WriterCmd(itsServerPort);
				command = writercmd;
				command->setMax(itsMaxBoards);
			}	break;
			*/			
			case 'h': {
				help();
			}
			
			case '?':
			default:
			{
				exit(EXIT_FAILURE);
			} break;
		}
	}

	if (command) {
		if (!command->getSelected()) {	// --select not used, select all
		  select.clear();
			for (int i = 0; i < command->getMax(); ++i) {
		  	select.push_back(i);
			}
		}
	  command->setSelect(select);
	}
  return(command);
}

//-----------------------------------------------------------------------------
std::list<int> TBBCtl::strtolist(const char* str, int max)
{
  string inputstring(str);
  char* start = (char*)inputstring.c_str();
  char* end   = 0;
  bool  range = false;
  long prevval = 0;
  list<int> resultset;

  resultset.clear();

  while(start) {
    long val = strtol(start, &end, 10); // read decimal numbers
    start = (end ? (*end ? end + 1 : 0) : 0); // determine next start
    if (val >= max || val < 0) {
      logMessage(cerr,formatString("Error: value %ld out of range",val));
      resultset.clear();
      return(resultset);
    }

    if (end) {
      switch (*end) {
        case ',':
        case 0:
        {
          if (range) {
            if (0 == prevval && 0 == val) {
              val = max - 1;
            }
            if (val < prevval) {
              logMessage(cerr,"Error: invalid range specification");
              resultset.clear();
              return(resultset);
            }
            for(long i = prevval; i <= val; i++)
              resultset.push_back(i);
          }
          else {
            resultset.push_back(val);
          }
          range = false;
        } break;

        case ':': {
          range = true;
        } break;

        default: {
          logMessage(cerr,formatString("Error: invalid character %c",*end));
          resultset.clear();
          return(resultset);
        } break;
      }
    }
    prevval = val;
  }

  return(resultset);
}

//-----------------------------------------------------------------------------
void TBBCtl::logMessage(ostream& stream, const string& message)
{
  if (itsCommand != 0) {
    itsCommand->logMessage(stream,message);
  }
  else {
    stream << message << endl;
  }
}

//-----------------------------------------------------------------------------
void TBBCtl::mainloop()
{
  start(); // make initial transition
  GCFTask::run();
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  TBBCtl tbbctl("tbbctl", argc, argv);

  try {
    tbbctl.mainloop();
  }
  catch (Exception e) {
    cerr << "Exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }
	
  LOG_INFO("Normal termination of program");

  return(0);
}

