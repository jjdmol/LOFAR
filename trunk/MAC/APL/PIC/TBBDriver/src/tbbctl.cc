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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <Common/lofar_set.h>
#include <time.h>

#include <netinet/in.h>    
#include <net/ethernet.h>  


#include "tbbctl.h"

using namespace std;
//using namespace blitz;
using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TbbCtl;

//---- ALLOC  ----------------------------------------------------------------
AllocCmd::AllocCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ================================================== allocate memory ====\n");	
}

//-----------------------------------------------------------------------------
void AllocCmd::send()
{
	TBBAllocEvent event;
	
	if (getSelected()) event.rcu_mask = getRcuMask(); // if select cmd is used
	
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult AllocCmd::ack(GCFEvent& e)
{
	TBBAllocackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = (int32)(cnr / 16);
		
		if (bnr != oldbnr) {
			if ((ack.status_mask[bnr] & TBB_SUCCESS) || (ack.status_mask[bnr] & TBB_RCU_COMM_ERROR)) {
				logMessage(cout,formatString(" %2d memory allocated for selected rcu's", bnr));
			}	else {
				logMessage(cout,formatString(" %2d %s", bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
			}
		}
		
		if (isSelected(cnr) && !(ack.status_mask[bnr] & TBB_NO_BOARD) ) {
			if (ack.rcu_mask.test(cnr)) {
				if (ack.status_mask[bnr] & TBB_SUCCESS) {
					logMessage(cout,formatString("     ERROR, Rcu-%d NOT in correct state\n",cnr));
				} else {
					logMessage(cout,formatString("     ERROR, Rcu-%d  %s\n",cnr,getDriverErrorStr(ack.status_mask[bnr] & 0xFFFF0000).c_str()));
				}
			}
		}
		oldbnr = bnr;
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- CHANNELINFO --------------------------------------------------------------
ChannelInfoCmd::ChannelInfoCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ===================================================== rcu info =======\n");
}

//-----------------------------------------------------------------------------
void ChannelInfoCmd::send()
{
	TBBRcuinfoEvent event;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ChannelInfoCmd::ack(GCFEvent& e)
{
	TBBRcuinfoackEvent ack(e);
	
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int rcu=0; rcu < getMaxSelections(); rcu++) {
	 		bnr = (int32)(rcu / 16);
			if (ack.rcu_state[rcu] != 'F') {
				if (isSelected(rcu) ) {
					if (bnr != oldbnr) {
						logMessage(cout,"Rcu  Board  Input  State  Start-address  Size[pages]");
						logMessage(cout,"---  -----  -----  -----  -------------  -----------");
					}
					logMessage(cout,formatString(" %2d    %2d     %2d      %c     0x%08X  %11u %s",
							rcu, ack.rcu_on_board[rcu], ack.rcu_on_input[rcu],
							(char)ack.rcu_state[rcu],	ack.rcu_start_addr[rcu], ack.rcu_pages[rcu],
							getBoardErrorStr(ack.rcu_status[rcu]).c_str()));
				}
			}
			oldbnr = bnr;
	}
	logMessage(cout,"\n *State:  F = Free, A = Allocated, R = Recording, S = Stopped, E = Error");
	logMessage(cout," *Only NOT Free rcu's are listed ");
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- FREE ----------------------------------------------------------------
FreeCmd::FreeCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB =================== discard buffer allocation and disable channels ====\n");
}

//-----------------------------------------------------------------------------
void FreeCmd::send()
{
	TBBFreeEvent event;
	
	if (getSelected()) event.rcu_mask = getRcuMask(); // if select cmd is used
	
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult FreeCmd::ack(GCFEvent& e)
{
	TBBFreeackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = (int32)(cnr / 16);
		
		if (bnr != oldbnr) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  buffer dischard and channel disabled for selected rcu's", bnr));
			} else {
				logMessage(cout,formatString(" %2d  %s", bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
			}
		}
		if (isSelected(cnr) && !(ack.status_mask[bnr] & TBB_NO_BOARD) ) {
			if (ack.rcu_mask.test(cnr)) {
				if (ack.status_mask[bnr] & TBB_SUCCESS) {
					logMessage(cout,formatString("     ERROR, Rcu-%d NOT in correct state\n",cnr));
				} else {
					logMessage(cout,formatString("     ERROR, Rcu-%d  %s\n",cnr,getDriverErrorStr(ack.status_mask[bnr]).c_str()));
				}
			}
		}
		oldbnr = bnr;
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- RECORD ----------------------------------------------------------------
RecordCmd::RecordCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ============================= start recording on selected channels ====\n");
}

//-----------------------------------------------------------------------------
void RecordCmd::send()
{
	TBBRecordEvent event;
	
	if (getSelected()) event.rcu_mask = getRcuMask(); // if select cmd is used
	
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult RecordCmd::ack(GCFEvent& e)
{
	TBBRecordackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = (int32)(cnr / 16);
		
		if (bnr != oldbnr) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  recording started for selected rcu's", bnr));
			} else {
				logMessage(cout,formatString(" %2d  %s", bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
			}
		}
		
		if (isSelected(cnr) && !(ack.status_mask[bnr] & TBB_NO_BOARD) ) {
			if (ack.rcu_mask.test(cnr)) {
				if (ack.status_mask[bnr] & TBB_SUCCESS) {
					logMessage(cout,formatString("      ERROR, Rcu-%d NOT in correct state\n",cnr));
				} else {
					logMessage(cout,formatString("      ERROR, Rcu-%d  %s\n",cnr,getDriverErrorStr(ack.status_mask[bnr]).c_str()));
				}
			}
		}
		oldbnr = bnr;
	}
	
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- STOP -------------------------------------------------------------------
StopCmd::StopCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ============================== stop recording on selected channels ====\n");	
}

//-----------------------------------------------------------------------------
void StopCmd::send()
{
	TBBStopEvent event;
	
	if (getSelected()) event.rcu_mask = getRcuMask(); // if select cmd is used
	
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StopCmd::ack(GCFEvent& e)
{
	TBBStopackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = (int32)(cnr / 16);
		
		if (bnr != oldbnr) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  recording stopped for selected rcu's", bnr));
			} else {
				logMessage(cout,formatString(" %2d  %s", bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
			}
		}
		
		if (isSelected(cnr) && !(ack.status_mask[bnr] & TBB_NO_BOARD) ) {
			if (ack.rcu_mask.test(cnr)) {
				if (ack.status_mask[bnr] & TBB_SUCCESS) {
					logMessage(cout,formatString("      ERROR, Rcu-%d NOT in correct state\n",cnr));
				} else {
				  logMessage(cout,formatString("      ERROR, Rcu-%d  %s\n",cnr,getDriverErrorStr(ack.status_mask[bnr]).c_str()));
				}
			}
		}
		oldbnr = bnr;
	}
	
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- TRIGCLR ----------------------------------------------------------------
TrigclrCmd::TrigclrCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ============================ clear trigger on selected channel ====\n");
}

//-----------------------------------------------------------------------------
void TrigclrCmd::send()
{
	TBBTrigclrEvent event;
	event.channel = (uint32)getRcu();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TrigclrCmd::ack(GCFEvent& e)
{
	TBBTrigclrackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");  
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  clear trigger message",bnr ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask).c_str()));
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READ -------------------------------------------------------------------
ReadCmd::ReadCmd(GCFPortInterface& port) : Command(port),
	itsSecondsTime(0),itsSampleTime(0),itsPrePages(0),itsPostPages(0)
{
	logMessage(cout,"\n== TBB ==============  transfer data to CEP for all selected channels ====\n");	
}

//-----------------------------------------------------------------------------
void ReadCmd::send()
{
	TBBReadEvent event;
	event.channel = (uint32)getRcu();
	event.secondstime = itsSecondsTime;
	event.sampletime = itsSampleTime;
	event.prepages = itsPrePages;
	event.postpages = itsPostPages;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadCmd::ack(GCFEvent& e)
{
	TBBReadackEvent ack(e);
	logMessage(cout,"RCU  Info");
	logMessage(cout,"---  -------------------------------------------------------");  
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		if (isSelected(cnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  tranfering data to CEP",cnr ));
			}	else {			
				logMessage(cout,formatString(" %2d  %s",cnr, getDriverErrorStr(ack.status_mask).c_str()));
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- MODE --------------------------------------------------------------------
ModeCmd::ModeCmd(GCFPortInterface& port) : Command(port)
	,itsRecMode(0)
{
	logMessage(cout,"\n== TBB ===================================== set mode command ===============\n");
}

//-----------------------------------------------------------------------------
void ModeCmd::send()
{
	TBBModeEvent event;
	event.board = (uint32)getBoard();
	event.rec_mode = itsRecMode;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ModeCmd::ack(GCFEvent& e)
{
	TBBModeackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");  
	if (ack.status_mask & TBB_SUCCESS) {
		logMessage(cout,formatString(" %2d  mode set and UDP/IP configured", getBoard()));
	}	else {			
		logMessage(cout,formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()));
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- VERSION ----------------------------------------------------------------
VersionCmd::VersionCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ====================================== ID and version information ====\n");
}

//-----------------------------------------------------------------------------
void VersionCmd::send()
{
  TBBVersionEvent event;
  event.boardmask = getBoardMask();
  itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult VersionCmd::ack(GCFEvent& e)
{
  TBBVersionackEvent ack(e);
	
	logMessage(cout,formatString("TBBDriver software version %3.1f\n",(ack.driverversion / 10.)));
	logMessage(cout,"TBB  ID  Software   Board    TP0      MP0      MP1      MP2      TP3");
	logMessage(cout,"---  --  --------  -------  -------  -------  -------  -------  -------");
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2u  %2u   V%5.1f    V%4.1f    V%4.1f    V%4.1f    V%4.1f    V%4.1f    V%4.1f",
	          bnr,
	          ack.boardid[bnr],
						(ack.swversion[bnr] / 10.),
						(ack.boardversion[bnr] / 10.),
						(ack.tpversion[bnr] / 10.),
						(ack.mp0version[bnr] / 10.),
						(ack.mp1version[bnr] / 10.),
						(ack.mp2version[bnr] / 10.),
						(ack.mp3version[bnr] / 10.)));
			}	else {
					logMessage(cout,formatString(" %2u  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));	
      }
  	}
	}
	
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- SIZE -------------------------------------------------------------------
SizeCmd::SizeCmd(GCFPortInterface& port) : Command(port)
{
  logMessage(cout,"\n== TBB ==================================== installed memory information ====\n");	
}

//-----------------------------------------------------------------------------
void SizeCmd::send()
{
  TBBSizeEvent event;
  event.boardmask = getBoardMask();
  itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult SizeCmd::ack(GCFEvent& e)
{
  TBBSizeackEvent ack(e);
	logMessage(cout,"TBB  pages     Total memory ");
	logMessage(cout,"---  --------  ------------"); 
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				double gbyte = ((double)ack.npages[bnr] * 2048.) / (1024. * 1024. * 1024.); 
				logMessage(cout,formatString(" %2d  %8d  %6.3f GByte",bnr,ack.npages[bnr],gbyte));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
      }
  	}
	}
	logMessage(cout,"\n1 page = 2048 Bytes");
	
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- STATUS -----------------------------------------------------------------
StatusCmd::StatusCmd(GCFPortInterface& port) : Command(port)
{
  logMessage(cout,"\n== TBB ============================= voltage and temperature information ====\n");	
}

//-----------------------------------------------------------------------------
void StatusCmd::send()
{
  TBBStatusEvent event;
  event.boardmask = getBoardMask();
  itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StatusCmd::ack(GCFEvent& e)
{
  TBBStatusackEvent ack(e);
    
	logMessage(cout,"TBB  Voltage 1.2  Voltage 2.5  Voltage 3.3  Temp PCB  Temp TP   Temp MP0  Temp MP1  Temp MP2  Temp MP3");
	logMessage(cout,"---  -----------  -----------  -----------  --------  --------  --------  --------  --------  --------");
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				
				logMessage(cout,formatString(" %2d     %4.2f V       %4.2f V       %4.2f V    %3u 'C    %3u 'C    %3u 'C    %3u 'C    %3u 'C    %3u 'C",
						bnr,
						((double)ack.V12[bnr] * (2.5 / 192.)),	// MAX6652 pin-2:  2.5 / 192	= 0.0130 / count
						((double)ack.V25[bnr] * (3.3 / 192.)),	// MAX6652 pin-3:  3.3 / 192	= 0.0172 / count
						((double)ack.V33[bnr] * (5.0 / 192.)),	// MAX6652 pin-1:  5.0 / 192	= 0.0625 / count
						ack.Tpcb[bnr],
						ack.Ttp[bnr],
						ack.Tmp0[bnr],
						ack.Tmp1[bnr],
						ack.Tmp2[bnr],
						ack.Tmp3[bnr] ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
      }
  	}
	}
	
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- CLEAR -------------------------------------------------------------------
ClearCmd::ClearCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB =============================================== clear in progress ====\n");
}

//-----------------------------------------------------------------------------
void ClearCmd::send()
{
	TBBClearEvent event;
	event.boardmask = getBoardMask();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ClearCmd::ack(GCFEvent& e)
{
	TBBClearackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  board is cleared",bnr ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- RESET -------------------------------------------------------------------
ResetCmd::ResetCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB =============================================== reset in progress ====\n");
}

//-----------------------------------------------------------------------------
void ResetCmd::send()
{
	TBBResetEvent event;
	event.boardmask = getBoardMask();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ResetCmd::ack(GCFEvent& e)
{
	TBBResetackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  board is reset",bnr ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- CONFIG -------------------------------------------------------------------
ConfigCmd::ConfigCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ========================================= reconfigure TP and MP's ====\n");
}

//-----------------------------------------------------------------------------
void ConfigCmd::send()
{
	TBBConfigEvent event;
	event.boardmask = getBoardMask();
	event.imagenr = itsImage;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ConfigCmd::ack(GCFEvent& e)
{
	TBBConfigackEvent ack(e);
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  reconfigured TP and MP's",bnr ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()));
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- ERASE IMAGE --------------------------------------------------------------
ErasefCmd::ErasefCmd(GCFPortInterface& port) : Command(port)
	,itsPage(0)
{
	logMessage(cout,"\n== TBB ===================================================== erase flash ====\n");
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");  

}

//-----------------------------------------------------------------------------
void ErasefCmd::send()
{
	TBBEraseImageEvent event;
	event.board = getBoard();
	event.image = itsPage;
	logMessage(cout,formatString(" %2d  erasing flash memory of image %d\n", getBoard(), itsPage));
	logMessage(cout,"     erasing will take about 8 seconds");	
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ErasefCmd::ack(GCFEvent& e)
{
	TBBEraseImageAckEvent ack(e);
	  
	if (ack.status_mask & TBB_SUCCESS) {
		logMessage(cout,formatString(" %2d  image is erased",getBoard()));
	}	else {	
		logMessage(cout,formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()));
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READ IMAGE --------------------------------------------------------------
ReadfCmd::ReadfCmd(GCFPortInterface& port) : Command(port)
	,itsPage(0)
{
	//memset(itsFileName,'\0',sizeof(itsFileName));
	//memset(itsPageData,0,sizeof(itsPageData));
	logMessage(cout,"\n== TBB ====================================================== read flash ====\n");
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");  
}

//-----------------------------------------------------------------------------
void ReadfCmd::send()
{
	TBBReadImageEvent event;
	event.board = getBoard();
	event.image = itsPage;
	logMessage(cout,formatString(" %2d  reading flash memory of image %d\n", getBoard(), itsPage));
	logMessage(cout,"     reading will take about 12 seconds");	
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadfCmd::ack(GCFEvent& e)
{
	TBBReadImageAckEvent ack(e);

		if (ack.status_mask & TBB_SUCCESS) {
			logMessage(cout,formatString(" %2d  image is read",getBoard()));
		}	else {	
			logMessage(cout,formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()));
		}

		
		setCmdDone(true);
	

	return(GCFEvent::HANDLED);
}

//---- WRITE IMAGE -----------------------------------------------------------
WritefCmd::WritefCmd(GCFPortInterface& port) : Command(port)
	,itsPage(0)
{
	memset(itsFileNameTp,'\0',sizeof(itsFileNameTp));
	memset(itsFileNameMp,'\0',sizeof(itsFileNameMp));
	logMessage(cout,"\n== TBB ===================================================== write flash ====\n");
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");  
}

//-----------------------------------------------------------------------------
void WritefCmd::send()
{
	TBBWriteImageEvent event;
	event.board = getBoard();
	event.image = itsPage;
	event.version = static_cast<int32>(round(itsVersion * 10.));
	memcpy(event.filename_tp,itsFileNameTp,sizeof(itsFileNameTp));
	memcpy(event.filename_mp,itsFileNameMp,sizeof(itsFileNameMp));
	logMessage(cout,formatString(" %2d  writing flash memory of image %d\n", getBoard(), itsPage));
	logMessage(cout,"     writing will take about 25 seconds");	
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult WritefCmd::ack(GCFEvent& e)
{
	TBBWriteImageAckEvent ack(e);
	if (ack.status_mask & TBB_SUCCESS) {
		logMessage(cout,formatString(" %2d  image is written",getBoard()));
	}	else {	
		logMessage(cout,formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()));
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- IMAGE INFO -------------------------------------------------------------
ImageInfoCmd::ImageInfoCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ===================================================== image info ====\n");
}

//-----------------------------------------------------------------------------
void ImageInfoCmd::send()
{
	TBBImageInfoEvent event;
	event.board = getBoard();
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ImageInfoCmd::ack(GCFEvent& e)
{
	TBBImageInfoAckEvent ack(e);
	logMessage(cout,formatString("Reading image information from TBB %d\n", getBoard()));
	logMessage(cout,"IMAGE  Version  Flash date");
	logMessage(cout,"-----  -------  -------------------");  
	if (ack.status_mask & TBB_SUCCESS) {
		for (int image = 0; image < 32; image++) {	
			if (ack.image_version[image] > 0xF0000000) {
				logMessage(cout,formatString("  %2d      -          -                 no image information",image));
			} else {
				time_t write_time;
				struct tm t;
				double version;
				
				write_time = static_cast<time_t>(ack.write_date[image]);
				t = *gmtime(&write_time);
				version = static_cast<double>(ack.image_version[image] / 10.);
				logMessage(cout,formatString("  %2d   %5.1f    %d-%d-%d  %d:%02d:%02d",image, version, 
									 t.tm_mday,t.tm_mon,t.tm_year+1900,t.tm_hour,t.tm_min,t.tm_sec));
			}
		}
	}	else {	
		logMessage(cout,formatString("TBB %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()));
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}



//---- READW -------------------------------------------------------------------
ReadwCmd::ReadwCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ======================================================= read DDR2 ====\n");
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");  
}

//-----------------------------------------------------------------------------
void ReadwCmd::send()
{
	TBBReadwEvent event;
	event.board = getBoard();
	event.mp = itsMp;
	event.addr = itsAddr;
	itsPort.send(event);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadwCmd::ack(GCFEvent& e)
{
	TBBReadwackEvent ack(e);
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  MP[%u] Addr[0x%08X]  [0x%08X] [0x%08X]"
						,bnr, itsMp, itsAddr, ack.wordlo, ack.wordhi ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s"
						,bnr, getDriverErrorStr(ack.status_mask).c_str()));
				itsAddr = itsStopAddr - 1;
			}
		}
	}
	if (itsAddr == (itsStopAddr - 1))
	
	
	itsAddr++;
	if (itsAddr >= itsStopAddr) { 
		setCmdDone(true); 
	}	else { 
		itsPort.setTimer(0.01);
	}
	return(GCFEvent::HANDLED);
}

//---- WRITEW -------------------------------------------------------------------
WritewCmd::WritewCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n== TBB ====================================================== write DDR2 ====\n");
}

//-----------------------------------------------------------------------------
void WritewCmd::send()
{
	TBBWritewEvent event;
	event.board = getBoard();
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
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  DDR2 write TBB_SUCCESS",bnr ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask).c_str()));
			}
		}
	}
	
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READR -------------------------------------------------------------------
ReadrCmd::ReadrCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n==== TBB-board, read register ================================================\n");	
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
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  read register",bnr ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask).c_str()));
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- WRITER -------------------------------------------------------------------
WriterCmd::WriterCmd(GCFPortInterface& port) : Command(port)
{
	logMessage(cout,"\n==== TBB-board, write register ===============================================\n");	
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
	logMessage(cout,"TBB  Info");
	logMessage(cout,"---  -------------------------------------------------------");    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				logMessage(cout,formatString(" %2d  write register",
						bnr ));
			}	else {	
				logMessage(cout,formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask).c_str()));
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READPAGE ------------------------------------------------------------------
ReadPageCmd::ReadPageCmd(GCFPortInterface& port) : Command(port),
	itsRcu(0),itsPages(1),itsCmdStage(0),itsPage(0),itsAddr(0),itsStartAddr(0),itsSize(0),itsBoard(0),itsMp(0),
	itsStationId(0),itsRspId(0),itsRcuId(0),itsSampleFreq(0),itsTime(0),itsSampleNr(0),itsSamplesPerFrame(0),
	itsFreqBands(0),itsTotalSamples(0),itsTotalBands(0)
{
	for (int i = 0; i < 512; i++) itsData[i] = 0;
	logMessage(cout,"\n==== TBB-board, readx register ================================================\n");
}
//-----------------------------------------------------------------------------
void ReadPageCmd::send()
{
	
	switch (itsCmdStage) {
		case 0: {
			TBBRcuinfoEvent send;
			itsPort.send(send);
		} break;
		
		case 1: {
			TBBWriterEvent send;
			send.board = itsBoard;
			send.mp = itsMp;
			send.pid = PID6;
			send.regid = REGID1;
			send.data[0] = itsAddr;
			send.data[1] = 0; 
			send.data[2] = 0;
			itsPort.send(send);
			
			LOG_DEBUG_STR(formatString("Writer[%x][%x][%x][%x]", 
				send.mp,send.pid,send.regid,send.data[0]));
		} break;
		
		case 2: {
			TBBWriterEvent send;
			send.board = itsBoard;
			send.mp = itsMp;
			send.pid = PID6;
			send.regid = REGID0;
			send.data[0] = PAGEREAD;
			send.data[1] = 0; 
			send.data[2] = 0;
			itsPort.send(send);
			
			LOG_DEBUG_STR(formatString("Writer[%x][%x][%x][%x]", 
				send.mp,send.pid,send.regid,send.data[0]));
		} break;
		
		case 3: {
			TBBReadxEvent send;
			send.board = itsBoard;	
			send.mp = itsMp;
			send.pid = PID6;
			send.regid = REGID2;
			send.pagelength = 256;
			send.pageaddr = 0;
			
			itsPort.send(send);
			LOG_DEBUG_STR(formatString("Readx[%x][%x][%x][%x][%x]", 
				send.mp,send.pid,send.regid,send.pagelength,send.pageaddr));
		} break;
		
		case 4: {
			TBBReadxEvent send;
			send.board = itsBoard;	
			send.mp = itsMp;
			send.pid = PID6;
			send.regid = REGID2;
			send.pagelength = 256;
			send.pageaddr = 256;
			
			itsPort.send(send);
			LOG_DEBUG_STR(formatString("Readx[%x][%x][%x][%x][%x]", 
				send.mp,send.pid,send.regid,send.pagelength,send.pageaddr));
		} break;
		
		default:{
		} break;
	}
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadPageCmd::ack(GCFEvent& e)
{
	int16 val[1400];
		
	switch (itsCmdStage) {
		case 0: {
			int rcu;
			rcu = getRcu();
			TBBRcuinfoackEvent ack(e);
			itsState = ack.rcu_state[rcu];
			itsStartAddr = ack.rcu_start_addr[rcu];
			itsSize = ack.rcu_pages[rcu];
			itsBoard = ack.rcu_on_board[rcu];
			itsMp = (int32)((int32)ack.rcu_on_input[rcu] / 4);
			logMessage(cout,formatString("Rcu-%d Board[%d] Mp[%d]",rcu,itsBoard,itsMp));
			
			itsAddr = itsStartAddr;
			if (itsState == 'F') {
				logMessage(cout,"Rcu not allocated");
				itsCmdStage = 10;
			}
		} break;
				
		case 1: {
			TBBWriterackEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				logMessage(cout,formatString("%s",getDriverErrorStr(ack.status_mask).c_str()));
				itsCmdStage = 10;
			}
		} break;
		
		case 2: {
			TBBWriterackEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				logMessage(cout,formatString("%s",getDriverErrorStr(ack.status_mask).c_str()));
				itsCmdStage = 10;
			}
		} break;
		
		case 3: {
			TBBReadxackEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				logMessage(cout,formatString("%s", getDriverErrorStr(ack.status_mask).c_str()));
				itsCmdStage = 10;
		  }
		  
		  // memcpy(itsData, ack.pagedata, sizeof(ack.pagedata));
			for (int32 dn = 0; dn < 256; dn++) { 
				itsData[dn] = ack.pagedata[dn];
			}
		} break;
		
		case 4: {
			TBBReadxackEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				logMessage(cout,formatString("%s", getDriverErrorStr(ack.status_mask).c_str()));
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
	if (itsCmdStage < 5) {
		itsPort.setTimer(0.01);
	} else { 
		
				
		if (itsPage == 0) {
			itsStationId = (int)(itsData[0] & 0xFF);
			itsRspId = (int)((itsData[0] >> 8) & 0xFF);
			itsRcuId = (int)((itsData[0] >> 16) & 0xFF);
			itsSampleFreq = (int)((itsData[0] >> 24) & 0xFF);
			itsTime = (time_t)itsData[2]-1;
			itsSampleNr = (int)itsData[3];
		}
		itsSamplesPerFrame = (int)(itsData[4] & 0xFFFF);
		itsFreqBands = (int)((itsData[4] >> 16) & 0xFFFF);
						
		int sample_cnt = 0;
		int val_cnt = 0;
		int data_cnt = 22; // 22 = startadress of data in frame
		
		
		if (itsFreqBands > 0) {
			// its SPECTRAL data
			if (itsSamplesPerFrame < 975) {		
				itsTotalSamples += itsSamplesPerFrame;
				itsTotalBands += itsFreqBands;
				logMessage(cout,formatString("Samples[%d] Bands[%d]",itsTotalSamples,itsTotalBands));
				// convert uint32 to complex int16
				while (sample_cnt < itsSamplesPerFrame) {
					// get complex sample
					val[val_cnt++] = (int16) (itsData[data_cnt] & 0xFFFF);	// re part
					val[val_cnt++] = (int16)((itsData[data_cnt++] >> 16) & 0xFFFF);	// im part
					sample_cnt++;
				}
			}
		}	else {
			// its RAW data
			if (itsSamplesPerFrame < 1299) {		
				itsTotalSamples += itsSamplesPerFrame;
				itsTotalBands += itsFreqBands;
				logMessage(cout,formatString("Samples[%d] Bands[%d]",itsTotalSamples,itsTotalBands));
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
		}
		if (val_cnt > 0) {
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
			if (itsPage < itsPages) {
				itsCmdStage = 1;
				itsAddr = itsStartAddr + itsPage;
				itsPort.setTimer(0.05);
			}	else {
				// print page information
				strftime(timestring, 255, "%Y-%m-%d  %H:%M:%S", gmtime(&itsTime));
				
				sprintf(line[0],"Station ID      : %d",itsStationId);
				sprintf(line[1],"RSP ID          : %d",itsRspId);
				sprintf(line[2],"RCU ID          : %d",itsRcuId);
				sprintf(line[3],"Sample freq     : %d MHz",itsSampleFreq);
				if (itsTime < 0) {
					sprintf(line[4],"Time            : invalid");
				} else {
					sprintf(line[4],"Time            : %s (%u)",timestring,(uint32)itsTime);
				}
				sprintf(line[5],"SampleNr        : %u",itsSampleNr);
				if (itsTotalBands) {
					sprintf(line[6],"FreqBands       : %u",itsTotalBands);
					sprintf(line[7],"Data file format: binary complex(int16 Re, int16 Im)");
				}	else {
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
				fclose(file);	
			}
		}	else {
			logMessage(cout,"No data in Frame");
		}
	}
	if (itsPage == itsPages) {
		
		setCmdDone(true);
	}

	return(GCFEvent::HANDLED);
}

//====END OF TBB COMMANDS========================================================================== 


//---- HELP --------------------------------------------------------------------
void TBBCtl::help()
{
	logMessage(cout,"\n==== tbbctl command usage =========================================================================================\n");
	
	logMessage(cout,"#  --command                : all boards or active rcu's are selected, and will be displayed");
	logMessage(cout,"#  --command --select=<set> : only information for all selected boards or rcu's is displayed\n"
									"#    Example: --select=0,1,4  or  --select=0:6  or  --select=0,1,2,8:11\n");
	logMessage(cout,"tbbctl --alloc [--select=<set>]                                    # allocate memmory locations for selected rcu's");
	logMessage(cout,"tbbctl --free [--select=<set>]                                     # free memmory locationsfor selected rcu's");
	logMessage(cout,"tbbctl --record [--select=<set>]                                   # start recording on selected rcu's");
	logMessage(cout,"tbbctl --stop [--select=<set>]                                     # stop recording on all selected rcu's");
	logMessage(cout,"tbbctl --rcuinfo [--select=<set>]                                  # list rcu info for all allocated rcu's\n");
	
	logMessage(cout,"tbbctl --trigclr=channel                                           # clear tigger message for all selected channel");
	logMessage(cout,"tbbctl --read=rcunr,secondstime,sampletime,prepages,postpages      # transfer recorded data from rcunr to CEP");
	logMessage(cout,"tbbctl --mode=board,[transient | subbands]                         # set mode to configure UDP/IP header for CEP"); 
	logMessage(cout,"tbbctl --version [--select=<set>]                                  # get version information from selected boards");
	logMessage(cout,"tbbctl --status [--select=<set>]                                   # get status information from selected boards");	
	logMessage(cout,"tbbctl --size [--select=<set>]                                     # get installed memory size from selected boards\n");
	
	logMessage(cout,"tbbctl --eraseimage=board,image                                    # erase image from flash");
	logMessage(cout,"tbbctl --readimage=board,image                                     # read image from flash to file");
	logMessage(cout,"tbbctl --writeimage=boardnr,imagenr,version,tpfilename,mpfilename  # write tp and mp file to imagenr on boardnr");
	logMessage(cout,"                                                                   # version is the version of the image stored");
	logMessage(cout,"tbbctl --imageinfo=board                                           # read info from all images on board");
	logMessage(cout,"tbbctl --readddr=board,mp,addr,size                                # read 2 words from DDR2 memory");
	logMessage(cout,"tbbctl --writeddr=board,mp,addr,wordL,wordH                        # write 2 words to DDR2 memory at addr");
	logMessage(cout,"tbbctl --readpage=board,mp,addr,pages                              # read n pages from DDR2 memory starting at addr");
	logMessage(cout,"tbbctl --clear [--select=<set>]                                    # clear selected board");
	logMessage(cout,"tbbctl --reset [--select=<set>]                                    # reset to factory images on selected boards");
	logMessage(cout,"tbbctl --config=imagenr [--select=<set>]                           # reconfigure TP and MP's with imagenr [0..31] on ");
	logMessage(cout,"                                                                   # selected boards");
	logMessage(cout,"tbbctl --help                                                      # this help screen\n");
	logMessage(cout,"===================================================================================================================");
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
      itsActiveBoards	= ack.active_boards_mask;
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
    case TBB_RCUINFOACK:
    case TBB_FREEACK:
    case TBB_RECORDACK:
    case TBB_STOPACK:
    case TBB_TRIGCLRACK:
    case TBB_READACK:
    case TBB_MODEACK:
    case TBB_VERSIONACK:
    case TBB_SIZEACK:
    case TBB_STATUSACK: 
    case TBB_CLEARACK:
    case TBB_RESETACK:
    case TBB_CONFIGACK:
    case TBB_ERASE_IMAGE_ACK:
    case TBB_READ_IMAGE_ACK:
    case TBB_WRITE_IMAGE_ACK:
		case TBB_IMAGE_INFO_ACK:
    case TBB_READWACK:
    case TBB_WRITEWACK:
    case TBB_READRACK:
    case TBB_WRITERACK:
    case TBB_READXACK: {	
    	status = itsCommand->ack(e); // handle the acknowledgement
    	if (itsCommand->isCmdDone()) {
    		GCFTask::stop();
    	}
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
  std::list<int> select;
  
  optind = 0; // reset option parsing
  //opterr = 0; // no error reporting to stderr
  
  while(1) {
  	static struct option long_options[] = {
			{ "select",			required_argument,	0,	'l' }, //ok
			{ "alloc",			no_argument,				0,	'a' }, //ok ??
			{ "rcuinfo",		no_argument,				0,	'i' }, //ok ??
			{ "free",				no_argument,				0,	'f' }, //ok ??
		  { "record",			no_argument,				0,	'r' }, //ok ??
		  { "stop",				no_argument,				0,	's' }, //ok ??
		  { "trigclr",		required_argument,	0,	't' }, //ok ??
			{ "read",				required_argument,	0,	'R' }, //ok ??
			{ "mode",				required_argument,	0,	'm' }, //ok ??
			{ "version",		no_argument,				0,	'v' }, //ok
			{ "size",				no_argument,				0,	'z' }, //ok
			{ "status",			no_argument,				0,	'A' }, //ok
			{ "readpage",		required_argument,	0,	'p' }, //ok ??
			{ "clear",			no_argument,				0,	'C' }, //ok ??
			{ "reset",			no_argument,				0,	'Z' }, //ok ??
			{ "config",			required_argument,	0,	'S' }, //ok ??
		  { "eraseimage",	required_argument,	0,	'1' }, //ok
		  { "readimage",	required_argument,	0,	'2' }, //ok
		  { "writeimage",	required_argument,	0,	'3' }, //ok
			{ "imageinfo",	required_argument,	0,	'8' }, //ok
		  { "readddr",		required_argument,	0,	'4' }, //ok
		  { "writeddr",		required_argument,	0,	'5' }, //ok
		  { "readreg",		required_argument,	0,	'6' }, //ok not in help
		  { "writereg",		required_argument,	0,	'7' }, //ok not in help
			{ "help",				no_argument,				0,	'h' }, //ok ??
		  { 0, 					  0, 									0,	0 },
		};

    int option_index = 0;
    int c = getopt_long(argc, argv,
												"l:afrst:R:m:vzAp:CZS:1:2:3:8:4:5:6:7:h",
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
  					select = strtolist(optarg, command->getMaxSelections());
  											
  	      	if (select.empty()) {
  		  			logMessage(cerr,"Error: invalid or missing '--select' option");
  		  			exit(EXIT_FAILURE);
  					}	else {
  						command->setSelected(true);
  					}
  	    	}	else {
  	      	logMessage(cerr,"Error: option '--select' requires an argument");
  	      	exit(EXIT_FAILURE);
  	      }
				} else {
	      	logMessage(cerr,"Error: channels already selected");
				}
	    } break;
	    
	    case 'a': { 	// --alloc
				if (command) delete command;
				AllocCmd* alloccmd = new AllocCmd(itsServerPort);
				command = alloccmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'i': { 	// --channelinfo
				if (command) delete command;
				ChannelInfoCmd* channelinfocmd = new ChannelInfoCmd(itsServerPort);
				command = channelinfocmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'f': { 	// --free
				if (command) delete command;
				FreeCmd* freecmd = new FreeCmd(itsServerPort);
				command = freecmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'r': { 	// --record
				if (command) delete command;
				RecordCmd* recordcmd = new RecordCmd(itsServerPort);
				command = recordcmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 's': { 	// --stop
				if (command) delete command;
				StopCmd* stopcmd = new StopCmd(itsServerPort);
				command = stopcmd;
				command->setCmdType(RCUCMD);
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
								"'--trigclr=rcu' ");  
						exit(EXIT_FAILURE);
					}
					select.clear();
		  		select.push_back(channel);
					command->setSelected(true);
				}
				
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'R': { 	// --read
				if (command) delete command;
				ReadCmd* readcmd = new ReadCmd(itsServerPort);
				command = readcmd;
				
				if (optarg) {
					int channel = 0;
					uint32 secondstime = 0;
					uint32 sampletime = 0;
					uint32 prepages = 0;
					uint32 postpages = 0;
					int numitems = sscanf(optarg, "%d,%u,%u,%u,%u",
						&channel, &secondstime, &sampletime, &prepages, &postpages);
					if (numitems < 5 || numitems == EOF) {
						logMessage(cerr,"Error: invalid number of arguments. Should be of the format "
								"'--read=rcu,secondstime,sampletime,prepages,postpages' ");  
						exit(EXIT_FAILURE);
					}
					readcmd->setSecondsTime(secondstime);
					readcmd->setSampleTime(sampletime);
					readcmd->setPrePages(prepages);
					readcmd->setPostPages(postpages);
					
					select.clear();
		  		select.push_back(channel);
					command->setSelected(true);
				}
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'm': { 	// --mode
				if (command) delete command;
				ModeCmd* modecmd = new ModeCmd(itsServerPort);
				command = modecmd;
				if (optarg) {
					int board = 0;
					char rec_mode[64];
					int numitems = sscanf(optarg, "%d,%s",
						&board, rec_mode);
					if (numitems < 2 || numitems == EOF) {
						logMessage(cerr,"Error: invalid number of arguments. Should be of the format "
								"'--udp=board,rec_mode' ");  
						exit(EXIT_FAILURE);
					}
					
					if (strcmp(rec_mode,"transient") == 0)
						modecmd->setRecMode(TBB_MODE_TRANSIENT);
					if (strcmp(rec_mode,"subbands") == 0)
						modecmd->setRecMode(TBB_MODE_SUBBANDS);
							
					select.clear();
		  		select.push_back(board);
					command->setSelected(true);
				}
				command->setCmdType(BOARDCMD);
			}	break;									
						
			case 'v': { 	// --version
	  		if (command) delete command;
	    	VersionCmd* versioncmd = new VersionCmd(itsServerPort);
	  		command = versioncmd;
				command->setCmdType(BOARDCMD);
	  	}	break;	
	  	
	  	case 'z': { 	// --size
	  		if (command) delete command;
	    	SizeCmd* sizecmd = new SizeCmd(itsServerPort);
	  		command = sizecmd;
				command->setCmdType(BOARDCMD);
	  	}	break;
	  	
	  	case 'A': { 	// --status
	  		if (command) delete command;
	    	StatusCmd* statuscmd = new StatusCmd(itsServerPort);
	  		command = statuscmd;
				command->setCmdType(BOARDCMD);
	  	}	break;
			
			case 'C': { 	// --clear
				if (command) delete command;
				ClearCmd* clearcmd = new ClearCmd(itsServerPort);
				command = clearcmd;
				command->setCmdType(BOARDCMD);
			}	break;
			
			case 'Z': { 	// --reset
				if (command) delete command;
				ResetCmd* resetcmd = new ResetCmd(itsServerPort);
				command = resetcmd;
				command->setCmdType(BOARDCMD);
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
				command->setCmdType(BOARDCMD);
			}	break;
			
			case 'p': { 	// --readpage
				if (command) delete command;
				ReadPageCmd* readddrcmd = new ReadPageCmd(itsServerPort);
				command = readddrcmd;
				
				if (optarg) {
					int32 rcu = 0;
					uint32 pages = 0;
					
					int numitems = sscanf(optarg, "%d,%u", &rcu,&pages);
					
					if (numitems < 2 || numitems == EOF || rcu < 0 || rcu >= MAX_N_RCUS) {
						logMessage(cerr,"Error: invalid read ddr value. Should be of the format "
								"'--readw=board,mp,addr' where rcu= 0..191");  
						exit(EXIT_FAILURE);
					}
					readddrcmd->setPages(pages);
					select.clear();
		  		select.push_back(rcu);
		  		command->setSelected(true);
				}	else {
					logMessage(cerr,"Error: invalid read ddr value. Should be of the format "
								"'--readw=board,mp,addr' where board= 0..11, mp= 0..3 and addr= 0x..");  
						exit(EXIT_FAILURE);
				}
				command->setCmdType(BOARDCMD);
			}	break;			
						
			case '1': { 	// --erasef
				if (command) delete command;
				ErasefCmd* erasefcmd = new ErasefCmd(itsServerPort);
				command = erasefcmd;
				if (optarg) {
					int board = 0;
					int page = 0;
					int numitems = sscanf(optarg, "%d,%d", &board, &page);
					
					if (numitems < 2 || numitems == EOF 
							|| page < 0 || page >= FL_N_PAGES 
							|| board < 0 || board >= MAX_N_TBBBOARDS) {
						logMessage(cerr,"Error: invalid page value. Should be of the format "
								"'--eraseimage=board,page' where board= 0 .. 11, image= 0..31");  
						exit(EXIT_FAILURE);
					}
					erasefcmd->setPage(page);
					select.clear();
					select.push_back(board);
					command->setSelected(true);
				}
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '2': { 	// --readf
				if (command) delete command;
				ReadfCmd* readfcmd = new ReadfCmd(itsServerPort);
				command = readfcmd;
				if (optarg) {
					int board = 0;
					int page = 0;
					int numitems = sscanf(optarg, "%d,%d", &board, &page);
					
					if (numitems < 2 || numitems == EOF 
							|| page < 0 || page >= FL_N_PAGES 
							|| board < 0 || board >= MAX_N_TBBBOARDS) {
						logMessage(cerr,"Error: invalid page value. Should be of the format "
								"'--readimage=board,page' where board= 0 .. 11, image= 0..31");  
						exit(EXIT_FAILURE);
					}
					readfcmd->setPage(page);
					select.clear();
					select.push_back(board);
					command->setSelected(true);
				}
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '3': { 	// --writef
				if (command) delete command;
				WritefCmd* writefcmd = new WritefCmd(itsServerPort);
				command = writefcmd;
				if (optarg) {
					int board = 0;
					int page = 0;
					double version = 0;
					char filename_tp[64];
					char filename_mp[64];
					memset(filename_tp,0,64);
					memset(filename_mp,0,64);
					
					int numitems = sscanf(optarg, "%d,%d,%lf,%63[^,],%63[^,]", &board, &page, &version, filename_tp, filename_mp);
					if (numitems < 5 || numitems == EOF 
							|| page < 0 || page >= FL_N_PAGES 
							|| board < 0 || board >= MAX_N_TBBBOARDS) {
						logMessage(cerr,"Error: invalid values. Should be of the format "
								"'--writeimage=board,page,filename tp,filename mp' where board= 0 .. 11, image= 0..31");  
						exit(EXIT_FAILURE);
					}
					writefcmd->setPage(page);
					writefcmd->setVersion(version);
					writefcmd->setFileNameTp(filename_tp);
					writefcmd->setFileNameMp(filename_mp);

					select.clear();
					select.push_back(board);
					command->setSelected(true);
				}
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '8': { // --imageinfo
				if (command) delete command;
				ImageInfoCmd* imageinfocmd = new ImageInfoCmd(itsServerPort);
				command = imageinfocmd;
				if (optarg) {
					int board = 0;
					int numitems = sscanf(optarg, "%d", &board);
					if (numitems < 1 || numitems == EOF || board < 0 || board >= MAX_N_TBBBOARDS) {
						logMessage(cerr,"Error: invalid values. Should be of the format "
								"'--imageinfo=board' where board= 0 .. 11");  
						exit(EXIT_FAILURE);
					}
					select.clear();
					select.push_back(board);
					command->setSelected(true);
				}
				command->setCmdType(BOARDCMD);
			} break;
			
			case '4': { 	// --readw
				if (command) delete command;
				ReadwCmd* readwcmd = new ReadwCmd(itsServerPort);
				command = readwcmd;
				
				if (optarg) {
					int board = 0;
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
				} else {
					logMessage(cerr,"Error: invalid read ddr value. Should be of the format "
								"'--readw=board,mp,addr' where board= 0..11, mp= 0..3 and addr= 0x..");  
						exit(EXIT_FAILURE);
				}
				command->setCmdType(BOARDCMD);
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
				}	else {
					logMessage(cerr,"Error: invalid write ddr value. Should be of the format "
								"'--writew=board,mp,addr,wordlo,wordhi' where board= 0..11, mp= 0..3 and addr= 0..x.");  
						exit(EXIT_FAILURE);
				}
				command->setCmdType(BOARDCMD);
			}	break;
			/*
			case '6': { 	// --readr
				if (command) delete command;
				ReadrCmd* readrcmd = new ReadrCmd(itsServerPort);
				command = readrcmd;
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '7': { 	// --writer
				if (command) delete command;
				WriterCmd* writercmd = new WriterCmd(itsServerPort);
				command = writercmd;
				command->setCmdType(BOARDCMD);
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
			for (int i = 0; i < command->getMaxSelections(); i++) {
				select.push_back(i);	
			}
			command->setSelected(true);
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
	std::list<int> resultset;
			
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
          } else {
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
  } else {
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

