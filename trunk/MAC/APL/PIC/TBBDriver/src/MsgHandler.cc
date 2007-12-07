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

#include "MsgHandler.h"

namespace LOFAR {
	namespace TBB {


MsgHandler::MsgHandler()
{
	TS						= TbbSettings::instance();
	itsTriggerE		= new TBBTriggerEvent();
	itsErrorE			=	new TBBErrorEvent();
	itsBoardchangeE	= new	TBBBoardchangeEvent();
}

MsgHandler::~MsgHandler()
{
	delete itsTriggerE;
	delete itsErrorE;
	delete itsBoardchangeE;
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
	TPTriggerEvent	*TPE;
	TPE	= new TPTriggerEvent(event);
	int channel = TPE->trigger.channel + (boardnr * TS->nrChannelsOnBoard());	
	TS->convertCh2Rcu(channel, &itsTriggerE->rcu);
	itsTriggerE->sequence_nr			=	TPE->trigger.sequence_nr;
	itsTriggerE->time							=	TPE->trigger.time;
	itsTriggerE->sample_nr				= TPE->trigger.sample_nr;
	itsTriggerE->trigger_sum 			= TPE->trigger.sum;
	itsTriggerE->trigger_samples	= TPE->trigger.samples;
	itsTriggerE->peak_value 			= TPE->trigger.peak;
				
	sendTriggerMessage(*itsTriggerE);
	TS->setChTriggered(channel, true);
		
	delete TPE;		
}

//-----------------------------------------------------------------------------
void MsgHandler::sendError(GCFEvent& event)
{
	TPErrorEvent	*TPE;
	TPE	= new TPErrorEvent(event);
		
	itsErrorE->code	= TPE->code;
	
	sendHardwareMessage(*itsErrorE);
	
	delete TPE;		
}

//-----------------------------------------------------------------------------
void MsgHandler::sendBoardChange(uint32 activeboards)
{
	itsBoardchangeE->activeboards = activeboards;
	
	sendHardwareMessage(*itsBoardchangeE);
}

//-----------------------------------------------------------------------------
void MsgHandler::sendTriggerMessage(GCFEvent& event)
{
  if (!itsClientTriggerMsgList.empty()) {
    for (list<GCFPortInterface*>::iterator it = itsClientTriggerMsgList.begin();
         it != itsClientTriggerMsgList.end();
         it++)
    {
  		(*it)->send(event);
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
  		(*it)->send(event);
    }
  }
}


	} // end namespace TBB
} // end namespace LOFAR
