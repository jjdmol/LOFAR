//  OctoVisOutputAgent.h: agent for output of visibilities via OCTOPUSSY
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

#ifndef OCTOAGENT_SRC_OCTOVISOUTPUTAGENT_H_HEADER_INCLUDED_E9B1E216
#define OCTOAGENT_SRC_OCTOVISOUTPUTAGENT_H_HEADER_INCLUDED_E9B1E216
    
#include <OctoAgent/OctoAgent.h>
#include <OctoAgent/OctoVisAgentVocabulary.h>
#include <VisAgent/VisOutputAgent.h>
    
class OctoMultiplexer;

//##ModelId=3E0AA82F0086
class OctoVisOutputAgent : public VisOutputAgent, public OctoAgent
{
  public:
    //##ModelId=3E2FC9EA02DC
    OctoVisOutputAgent(const HIID &mapfield = OctoAgentVocabulary::FDefaultMapField);

    //##ModelId=3E2FCA9002D8
    OctoVisOutputAgent(OctoMultiplexer &pxy, const HIID &mapfield = OctoAgentVocabulary::FDefaultMapField);

    //##ModelId=3E2FCA040044
    //##Documentation
    //## Agent initialization method. Called by the application to initialize
    //## or reinitializean agent. Agent parameters are supplied via a
    //## DataRecord.
    virtual bool init(const DataRecord::Ref &data);

    //##ModelId=3E2FC9C700A9
    //##Documentation
    //## Puts visibilities header onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end
    //##          CLOSED    stream closed
    virtual int putHeader(DataRecord::Ref &hdr);

    //##ModelId=3E2FC9CA01E6
    //##Documentation
    //## Puts next tile onto output stream. If stream has been
    //## suspended (i.e. from other end), returns WAIT (wait=False), or
    //## blocks until it is resumed (wait=True)
    //## Returns: SUCCESS   on success
    //##          WAIT      stream has been suspended from other end    //##
    //## Gets next available tile from input stream. If wait=True, blocks until
    //##          CLOSED    stream closed
    virtual int putNextTile(VisTile::Ref &tile);



    //##ModelId=3E2FC9F902FE
    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;

  private:
    //##ModelId=3E427454028A
    OctoVisOutputAgent(const OctoVisOutputAgent& right);

    //##ModelId=3E2FCA900327
    OctoVisOutputAgent& operator=(const OctoVisOutputAgent& right);


    //##ModelId=3E2FD6650332
    DataRecord::Ref header;

};



#endif /* OCTOAGENT_SRC_OCTOVISOUTPUTAGENT_H_HEADER_INCLUDED_E9B1E216 */
