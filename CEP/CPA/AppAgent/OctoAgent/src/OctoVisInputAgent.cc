//  OctoVisInputAgent.cc: agent for input of visibilities via OCTOPUSSY
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

#include "OctoVisInputAgent.h"
#include "OctoMultiplexer.h"

using namespace VisAgentVocabulary;
       
//##ModelId=3E0AAB220065
OctoVisInputAgent::OctoVisInputAgent(const HIID &mapfield)
    : OctoAgent(mapfield)
{
  suspend_posted = False;
}

//##ModelId=3E27CEC50344
OctoVisInputAgent::OctoVisInputAgent(OctoMultiplexer &pxy,const HIID &mapfield)
    : OctoAgent(pxy,mapfield)
{
  suspend_posted = False;
}

//##ModelId=3E0AAB7601A7
bool OctoVisInputAgent::init (const DataRecord::Ref &data)
{
  suspend_posted = False;
  return OctoAgent::init(data);
}

//##ModelId=3E0AAB7D00C3
int OctoVisInputAgent::getHeader (DataRecord::Ref &hdr, bool wait)
{
  HIID dum;
  return getEvent(dum,hdr,FHeaderEvent,wait);
}

//##ModelId=3E0AAB810240
int OctoVisInputAgent::getNextTile (VisTile::Ref &tile, bool wait)
{
  for(;;)
  {
    HIID dum;
    ObjRef ref;
    int result = getEvent(dum,ref,FTileEvent,wait);
    // return if unsuccessful
    if( result != SUCCESS )
      return result;
    // got event: check data type. If it doesn't match, then we'll ignore
    // this event and go back for another one
    if( ref.valid() )
    {
      if( ref->objectType() == TpVisTile )
      {
        tile.xfer(ref.ref_cast<VisTile>());
        return SUCCESS;
      }
      else
      {
        cdebug(2)<<"ignoring tile event "<<dum<<": unexpected type "<<
           ref->objectType().toString()<<endl;
      }
    }
    else
    {
      cdebug(2)<<"ignoring tile event "<<dum<<": no payload\n";
    }
  }
}

//##ModelId=3E0AAB8602A8
int OctoVisInputAgent::hasHeader ()
{
  return hasEvent(FHeaderEvent);
}

//##ModelId=3E0AAB8B01A0
int OctoVisInputAgent::hasTile ()
{
  return hasEvent(FTileEvent);
}

//##ModelId=3E0AAB920304
void OctoVisInputAgent::suspend ()
{
  if( !suspend_posted )
  {
    postEvent(FSuspendEvent);
    suspend_posted = True; 
  }
}

//##ModelId=3E0AAB98032A
void OctoVisInputAgent::resume ()
{
  postEvent(FResumeEvent);
  suspend_posted = False;
}

