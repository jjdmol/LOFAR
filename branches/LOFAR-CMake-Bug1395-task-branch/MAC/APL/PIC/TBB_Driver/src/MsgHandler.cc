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

#include "MsgHandler.h"

namespace LOFAR {
	namespace TBB {


MsgHandler::MsgHandler()
{
	TS = TbbSettings::instance();
		
	/*
	if (TS->saveTriggersToFile()) {
		time_t timenow;
		timenow = time(NULL);
		
		strftime(itsTimeString, 255, "%Y-%m-%d", gmtime(&timenow));
		snprintf(itsFileName, PATH_MAX, "/opt/lofar/log/%s_TRIGGER.dat", itsTimeString);
		itsFile = fopen(itsFileName,"a");
	}
	else {
		memset(itsFileName, 12, '\0');
		memset(itsTimeString, 12, '\0');
		itsFile = 0;
	}
	*/
	memset(itsFileName, 12, '\0');
	memset(itsTimeString, 12, '\0');
	itsFile = 0;
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
void MsgHandler::sendTrigger(GCFEvent& event, int boardnr)
{
	TPTriggerEvent	tp_event(event);
	TBBTriggerEvent tbb_event;
	
	int channel = tp_event.trigger.channel + (boardnr * TS->nrChannelsOnBoard());	
	TS->convertCh2Rcu(channel, &tbb_event.rcu);
	tbb_event.sequence_nr     = tp_event.trigger.sequence_nr;
	tbb_event.time            = tp_event.trigger.time;
	tbb_event.sample_nr       = tp_event.trigger.sample_nr;
	tbb_event.trigger_sum     = tp_event.trigger.sum;
	tbb_event.trigger_samples = tp_event.trigger.samples;
	tbb_event.peak_value      = tp_event.trigger.peak;
	tbb_event.power_before    = tp_event.trigger.pwr_bt_at & 0x0000FFFF;
	tbb_event.power_after     = (tp_event.trigger.pwr_bt_at & 0xFFFF0000) >> 16;
				
	sendTriggerMessage(tbb_event);
	
	// save trigger messages to a file
	
	
	if (TS->saveTriggersToFile()) {
		int err;
		if (itsFile != 0) {
			time_t timenow;
			timenow = time(NULL);
			char timestring[12];
			strftime(timestring, 255, "%Y-%m-%d", gmtime(&timenow));
			
			if (strcmp(timestring, itsTimeString) != 0) {
				strcpy(itsTimeString, timestring);
				fclose(itsFile);
				snprintf(itsFileName, PATH_MAX, "/opt/lofar/log/%s_TRIGGER.dat", itsTimeString);
				itsFile = fopen(itsFileName,"a");
			}
		
			err = fprintf(itsFile,"%d %u %u %u %u %u %u %u %u\n",
					tbb_event.rcu,
					tbb_event.sequence_nr,
					tbb_event.time,
					tbb_event.sample_nr,
					tbb_event.trigger_sum,
					tbb_event.trigger_samples,
					tbb_event.peak_value,
					tbb_event.power_before,
					tbb_event.power_after);
			fflush(itsFile);
		}
	}
	TS->setChTriggered(channel, true);
}
//-----------------------------------------------------------------------------
void MsgHandler::openTriggerFile()
{
	if (itsFile == 0) {
		time_t timenow;
		timenow = time(NULL);
		char timestring[12];
		strftime(timestring, 255, "%Y-%m-%d", gmtime(&timenow));
		strcpy(itsTimeString, timestring);
		snprintf(itsFileName, PATH_MAX, "/opt/lofar/log/%s_TRIGGER.dat", itsTimeString);
		itsFile = fopen(itsFileName,"a");
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
