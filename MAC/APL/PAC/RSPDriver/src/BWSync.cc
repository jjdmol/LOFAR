//#  BWSync.cc: implementation of the BWSync class
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

#include "BWSync.h"
#include "EPA_Protocol.ph"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;

BWSync::BWSync() : SyncAction((State)&BWSync::initial_state)
{
}

BWSync::~BWSync()
{
}

GCFEvent::TResult BWSync::initial_state(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  LOG_INFO("BWSync::initial_state");

  switch (event.signal)
  {
  case F_TIMER:
      {
	  EPABfxreEvent bfxre;
	  EPABfximEvent bfxim;
	  EPABfyreEvent bfyre;
	  EPABfyimEvent bfyim;

	  MEP_BFXRE(bfxre.hdr, MEPHeader::WRITE, 0);
	  MEP_BFXIM(bfxim.hdr, MEPHeader::WRITE, 0);
	  MEP_BFYRE(bfyre.hdr, MEPHeader::WRITE, 0);
	  MEP_BFYIM(bfyim.hdr, MEPHeader::WRITE, 0);

	  /* initial triggering of this synchronization action */
	  for (int i = 0; i < N_RSPBOARDS; i++)
	  {
	      bfxre.hdr.m_fields.addr.dstid = i;
	      port.send(bfxre);
	  }
      }
      break;

  default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}
