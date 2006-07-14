//#  GetVersions.cc: implementation of the GetVersions class
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

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <APL/TBB_Protocol/TP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "VersionCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace TBB;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace RTC;


bool VersionCmd::isVallid(GCFEvent& event)
{
	TBBVersionEvent tbbe(event);
	
	if((tbbe.signal == TBBVERSION)||(tbbe.signal == TPVERSION))
	{
		return true;
	}
	return false;
}

void VersionCmd::sendTpEvent(GCFEvent& event, GCFPortInterface& boardport[])
{
	TBBVersionEvent tbbe(event);
	TPVersionEvent tpe();
	
	m_SendMask = tbbe.tbbmask; // for some commands boardid ???
	m_RecvMask = 0;
	m_ClientPort = port;
	
	tpe.opcode = 0x00000701;
	tpe.boardid = 0;
	tpe.swversion = 0;
	tpe.boardversion = 0;
	tpe.tpversion = 0;
	tpe.mpversion[0] = 0;
	tpe.mpversion[1] = 0;
	tpe.mpversion[2] = 0;
	tpe.mpversion[3] = 0;
	
	for (int boardnr = 0;boardnr < m_maxboards;boardnr++)
	{
		if(m_SendMask & (1 << boardnr))
		{
			boardport[boardnr].send(tpe);
		}
	}
	
}

bool VersionCmd::saveTpAckEvent(GCFEvent& event, int boardnr)
{
	TPVersionEvent tpe(event);
	m_RecvMask |= (1 << boardnr);
	
	m_boardid[boardnr] = tpe.boardid;
	m_swversion[boardnr] = tpe.swversion;
	m_boardversion[boardnr] = tpe.boardversion;
	m_tpversion[boardnr] = tpe.tpversion;
	m_mpversion[boardnr][0] = tpe.mpversion[0];
	m_mpversion[boardnr][1] = tpe.mpversion[1];
	m_mpversion[boardnr][2] = tpe.mpversion[2];
	m_mpversion[boardnr][3] = tpe.mpversion[3];
	
	if(m_RecvMask == m_SendMask) 
	{
		return true;
	}
	return false;
}

void VersionCmd::sendTbbAckEvent(void)
{
	TBBVersionackEvent tbbe();
	
	for(int boardnr = 0;boardnr < 12;boardnr++)
	{
		tpe.board[boardnr].boardid = m_boardid[boardnr];
		tpe.board[boardnr].swversion = m_bswversion[boardnr];
		tpe.board[boardnr].boardversion = m_boardversion[boardnr];
		tpe.board[boardnr].tpversion = m_tpversion[boardnr];
		tpe.board[boardnr].mpversion[0] = m_mpversion[boardnr][0];
		tpe.board[boardnr].mpversion[1] = m_mpversion[boardnr][1];
		tpe.board[boardnr].mpversion[2] = m_mpversion[boardnr][2];
		tpe.board[boardnr].mpversion[3] = m_mpversion[boardnr][3];		
	}
	m_ClientPort.send(tpe);
}
