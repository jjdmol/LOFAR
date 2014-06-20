//#  MsgHandler.cc: implementation of the ClientMsgHandler class
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
//#  $Id: MsgHandler.cc,v 1.1 2006/07/13 14:22:09 donker Exp 

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <climits>
#include <cstdio>

#include "MsgHandler.h"

namespace LOFAR {
	namespace TBB {


MsgHandler::MsgHandler()
{
	TS = TbbSettings::instance();
	memset(itsFileName, 0, sizeof(itsFileName));
	memset(itsTimeString, 0, sizeof(itsTimeString));
	itsFile = 0;
	itsStartFilePos = 0;
}

MsgHandler::~MsgHandler()
{
	if (itsFile != 0) { fclose(itsFile); }
}

//-----------------------------------------------------------------------------
void MsgHandler::addTriggerClient(GCFPortInterface& port)
{
	itsClientTriggerMsgList.push_back(&port);	// add client to list
	
	itsClientTriggerMsgList.sort();	// and remove double inputs
	itsClientTriggerMsgList.unique();
}

//-----------------------------------------------------------------------------
void MsgHandler::removeTriggerClient(GCFPortInterface& port)
{
	itsClientTriggerMsgList.remove(&port);	// remove client from list
}

//-----------------------------------------------------------------------------
void MsgHandler::addHardwareClient(GCFPortInterface& port)
{
	itsClientHardwareMsgList.push_back(&port);	// add client to list
	
	itsClientHardwareMsgList.sort();	// and remove double inputs
	itsClientHardwareMsgList.unique();
}

//-----------------------------------------------------------------------------
void MsgHandler::removeHardwareClient(GCFPortInterface& port)
{
	itsClientHardwareMsgList.remove(&port);	// remove client from list
}

//-----------------------------------------------------------------------------
void MsgHandler::sendSavedTrigger()
{
	//LOG_DEBUG_STR(formatString("send saved trigger from board %d to client", boardnr));
	TBBTriggerEvent tbb_event;
	TriggerInfo *triggerInfo = TS->getTriggerInfo();
		
	tbb_event.rcu             = triggerInfo->rcu;
	tbb_event.nstimestamp     = triggerInfo->ns_timestamp;
	tbb_event.trigger_sum     = triggerInfo->trigger_sum;
	tbb_event.trigger_samples = triggerInfo->trigger_samples;
	tbb_event.peak_value      = triggerInfo->peak_value;
	tbb_event.power_before    = triggerInfo->power_before;
	tbb_event.power_after     = triggerInfo->power_after;
	tbb_event.missed          = triggerInfo->missed & 0x00FFFFFF;
				
	sendTriggerMessage(tbb_event);
	
	// save trigger messages to a file
	if (TS->saveTriggersToFile()) {
	    //LOG_DEBUG_STR(formatString("write saved trigger from board %d to file", boardnr));
		writeTriggerToFile(&tbb_event);
	}
}
//-----------------------------------------------------------------------------

void MsgHandler::openTriggerFile()
{
	if (itsFile == 0) {
		time_t timenow;
		timenow = time(NULL);
		strftime(itsTimeString, sizeof(itsTimeString), "%Y-%m-%d", gmtime(&timenow));
		snprintf(itsFileName, sizeof(itsFileName), "/localhome/data/%s_TRIGGER.dat", itsTimeString);
		itsFile = fopen(itsFileName,"a");
		itsStartFilePos = ftell(itsFile);
	}
}

//-----------------------------------------------------------------------------
void MsgHandler::closeTriggerFile()
{
	if (itsFile != 0) {
		fclose(itsFile);
		itsFile = 0;
	}
}

//-----------------------------------------------------------------------------
void MsgHandler::writeTriggerToFile(TBBTriggerEvent *trigger_event)
{
	if (itsFile != 0) {
	    // if file to big, open a new one and ad file number
	    // the highest number is the oldest file
		if ((ftell(itsFile) - itsStartFilePos) > 1000000000) {
			fclose(itsFile);
			char fileName1[256];
			char fileName2[256];
			FILE* f;
			int fileNr = 0;
			
			// find first free file number
			while(true) {
			    snprintf(fileName1, sizeof(fileName1), "/localhome/data/%s_TRIGGER_%d.dat", itsTimeString, fileNr);
			    f = fopen(fileName1,"r");
			    if (f) {
			        fclose(f);
			        ++fileNr;
			    }
			    else {
			        break;
			    }
			}
			
			// shift all files one number, so number 0 is free
			while (fileNr > 0) {
			    snprintf(fileName1, sizeof(fileName1), "/localhome/data/%s_TRIGGER_%d.dat", itsTimeString, fileNr-1);
			    snprintf(fileName2, sizeof(fileName2), "/localhome/data/%s_TRIGGER_%d.dat", itsTimeString, fileNr);
			    rename(fileName1, fileName2);
			    --fileNr;
			}
			
			rename(itsFileName, fileName1);
			itsFile = fopen(itsFileName,"a");
		}
		
		time_t timenow;
		timenow = time(NULL);
		char timestring[12];
		strftime(timestring, 255, "%Y-%m-%d", gmtime(&timenow));
		
		// a new day, a new file
		if (strcmp(timestring, itsTimeString) != 0) {
			strcpy(itsTimeString, timestring);
			fclose(itsFile);
			snprintf(itsFileName, sizeof(itsFileName), "/localhome/data/%s_TRIGGER.dat", itsTimeString);
			itsFile = fopen(itsFileName,"a");
			itsStartFilePos = ftell(itsFile);
		}
	
		(void)fprintf(itsFile,"%d %lu %lu %u %u %u %u %u %u \n",
				trigger_event->rcu,
				trigger_event->nstimestamp.sec(),
				trigger_event->nstimestamp.nsec(),
				trigger_event->trigger_sum,
				trigger_event->trigger_samples,
				trigger_event->peak_value,
				trigger_event->power_before,
				trigger_event->power_after,
				trigger_event->missed);
		fflush(itsFile);
	}
}

//-----------------------------------------------------------------------------
void MsgHandler::sendError(GCFEvent& event)
{
	TPErrorEvent tp_event(event);
	TBBErrorEvent tbb_event;
	
	tbb_event.code	= tp_event.code;
	
	sendHardwareMessage(tbb_event);
}

//-----------------------------------------------------------------------------
void MsgHandler::sendBoardChange(uint32 activeboards)
{
	TBBBoardchangeEvent tbb_event;
	
	tbb_event.activeboards = activeboards;
	
	sendHardwareMessage(tbb_event);
}

//-----------------------------------------------------------------------------
void MsgHandler::sendTriggerMessage(GCFEvent& event)
{
	if (!itsClientTriggerMsgList.empty()) {
		for (list<GCFPortInterface*>::iterator it = itsClientTriggerMsgList.begin();
				it != itsClientTriggerMsgList.end();
				it++)
		{
			if ((*it)->isConnected()) { (*it)->send(event); }
		}
	}
}

//-----------------------------------------------------------------------------
void MsgHandler::sendHardwareMessage(GCFEvent& event)
{ 
	if (!itsClientHardwareMsgList.empty()) {
		for (list<GCFPortInterface*>::iterator it = itsClientHardwareMsgList.begin();
				it != itsClientHardwareMsgList.end();
				it++)
		{
			if ((*it)->isConnected()) { (*it)->send(event); }
		}
	}
}
	} // end namespace TBB
} // end namespace LOFAR
