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


//##ModelId=3E2FCA9002D8
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
int OctoVisOutputAgent::putHeader (DataRecord::Ref &hdr)
{
  // suspended? wait or return
  header = hdr; // xfer ref
  AppAgent::postEvent(FHeaderEvent,header);
  return SUCCESS;
}

//##ModelId=3E2FC9CA01E6
int OctoVisOutputAgent::putNextTile (VisTile::Ref &tile)
{
  ObjRef ref = tile.ref_cast<BlockableObject>();
  OctoAgent::postEvent(FTileEvent,ref);
  return SUCCESS;
}
//##ModelId=3E2FC9F902FE
string OctoVisOutputAgent::sdebug(int detail, const string &prefix,const char *name) const
{
  return OctoAgent::sdebug(detail,prefix,name?name:"OctoVisInputAgent");
}

