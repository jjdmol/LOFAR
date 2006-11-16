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
void MsgHandler::addClient(GCFPortInterface& port)
{
	itsClientMsgList.insert(&port);

	//itsClientMsgList.push_back(&port);
	//itsClientMsgList.sort();
	//itsClientMsgList.unique();		
}

//-----------------------------------------------------------------------------
void MsgHandler::removeClient(GCFPortInterface& port)
{
	itsClientMsgList.erase(&port);
	//itsClientMsgList.remove(&port);		
}

//-----------------------------------------------------------------------------
void MsgHandler::sendTrigger(GCFEvent& event)
{
	TPTriggerEvent	*TPE;
	TPE	= new TPTriggerEvent(event);
		
	itsTriggerE->channel	= TPE->channel;
	itsTriggerE->time			=	TPE->time;
	itsTriggerE->sample		= TPE->sample;
	
	sendMessage(*itsTriggerE);
	
	delete TPE;		
}

//-----------------------------------------------------------------------------
void MsgHandler::sendError(GCFEvent& event)
{
	TPErrorEvent	*TPE;
	TPE	= new TPErrorEvent(event);
		
	itsErrorE->code	= TPE->code;
	
	sendMessage(*itsErrorE);
	
	delete TPE;		
}

//-----------------------------------------------------------------------------
void MsgHandler::sendBoardChange(uint32 activeboards)
{
	itsBoardchangeE->activeboards = activeboards;
	
	sendMessage(*itsBoardchangeE);
}

//-----------------------------------------------------------------------------
void MsgHandler::sendMessage(GCFEvent& event)
{
  if (!itsClientMsgList.empty()) {
    for (set<GCFPortInterface*>::iterator it = itsClientMsgList.begin();
         it != itsClientMsgList.end();
         it++)
    {
  		(*it)->send(event);
    }
  }
}

	} // end namespace TBB
} // end namespace LOFAR
