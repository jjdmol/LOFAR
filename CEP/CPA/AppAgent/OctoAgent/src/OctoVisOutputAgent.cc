//  OctoVisOutputAgent.cc: agent for output of visibilities via OCTOPUSSY
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include "OctoVisOutputAgent.h"
#include "OctoMultiplexer.h"

using namespace VisAgentVocabulary;

//##ModelId=3E2FC9EA02DC
OctoVisOutputAgent::OctoVisOutputAgent(const HIID &mapfield)
    : OctoAgent(mapfield)
{
  suspended = False;
}


//##ModelId=3E2FC9ED0378
OctoVisOutputAgent::OctoVisOutputAgent(OctoMultiplexer &pxy, const HIID &mapfield)
    : OctoAgent(pxy,mapfield)
{
  suspended = False;
}

//##ModelId=3E2FCA040044
bool OctoVisOutputAgent::init(const DataRecord::Ref &data)
{
  suspended = False;
  return OctoAgent::init(data);
}

//##ModelId=3E2FC9C700A9
int OctoVisOutputAgent::putHeader (DataRecord::Ref &hdr, bool wait)
{
  // suspended? wait or return
  if( isSuspended() )
  {
    if( !wait )
      return WAIT;
    int res = waitForResume();
    if( res != SUCCESS )
      return res;
  }
  header = hdr; // xfer ref
  postEvent(FHeaderEvent,header);
  return SUCCESS;
}

//##ModelId=3E2FC9CA01E6
int OctoVisOutputAgent::putNextTile (VisTile::Ref &tile, bool wait)
{
  // suspended? wait or return
  if( isSuspended() )
  {
    if( !wait )
      return WAIT;
    int res = waitForResume();
    if( res != SUCCESS )
      return res;
  }
  ObjRef ref = tile.ref_cast<BlockableObject>();
  postEvent(FTileEvent,ref);
  return SUCCESS;
}

//##ModelId=3E2FC9D0008D
bool OctoVisOutputAgent::isSuspended ()
{
  HIID id;
  ObjRef dum;
  // suspended? Check for a resume event
  if( suspended )
  {
    // swallow any spurios suspend events first
    while( getEvent(id,dum,FSuspendEvent,False) == SUCCESS );
    if( getEvent(id,dum,FResumeEvent,False) == SUCCESS )
      suspended = False;
  }
  // else not suspended? Check for a suspend event
  else
  {
    // swallow spurios resume events
    while( getEvent(id,dum,FResumeEvent,False) == SUCCESS );
    if( getEvent(id,dum,FSuspendEvent,False) == SUCCESS )
      suspended = True;
  }
  return suspended;
}

//##ModelId=3E2FC9D201E1
int OctoVisOutputAgent::waitForResume ()
{
  if( !suspended )
    return SUCCESS;
  // TODO: something intelligent here.
  // Because as soon as we slot an event into the agent, that's it, it
  // sits there, and can't be interrupted by a higher-priority event.
  return SUCCESS;
}

//##ModelId=3E2FC9F902FE
string OctoVisOutputAgent::sdebug(int detail, const string &prefix,const char *name) const
{
  return OctoAgent::sdebug(detail,prefix,name?name:"OctoVisInputAgent");
}

