//  OctoVisInputAgent.h: agent for input of visibilities via OCTOPUSSY  
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

#ifndef OCTOAGENT_SRC_OCTOVISINPUTAGENT_H_HEADER_INCLUDED_9F6B4F03
#define OCTOAGENT_SRC_OCTOVISINPUTAGENT_H_HEADER_INCLUDED_9F6B4F03
    
#include <OctoAgent/OctoAgent.h>
#include <OctoAgent/OctoVisAgentVocabulary.h>
#include <VisAgent/VisInputAgent.h>
    
class OctoMultiplexer;

//##ModelId=3E0AA80E021D
class OctoVisInputAgent : public VisInputAgent, public OctoAgent
{
  public:
    //##ModelId=3E0AAB220065
    OctoVisInputAgent (const HIID &mapfield = OctoAgentVocabulary::FDefaultMapField);
  
    //##ModelId=3E27CEC50344
    OctoVisInputAgent (OctoMultiplexer &pxy,const HIID &mapfield = OctoAgentVocabulary::FDefaultMapField);

    //##ModelId=3E0AAB7601A7
    //##Documentation
    //## Agent initialization method. Called by the application to initialize
    //## or reinitializean agent. Agent parameters are supplied via a
    //## DataRecord.
    virtual bool init(const DataRecord::Ref &data);

    //##ModelId=3E0AAB7D00C3
    //##Documentation
    //## Gets visibilities header from input stream. If wait=True, blocks until
    //## one has arrived.
    //## Returns: SUCCESS   on success
    //##          WAIT      header has not yet arrived (only for wait=False)
    //##          CLOSED    stream closed
    //##          OUTOFSEQ  next object in stream is not a header (i.e. a tile)
    //## 
    virtual int getHeader(DataRecord::Ref &hdr, bool wait = True);

    //##ModelId=3E0AAB810240
    //##Documentation
    //## Gets next available tile from input stream. If wait=True, blocks until
    //## one has arrived.
    //## Returns: SUCCESS   on success
    //##          WAIT      a tile has not yet arrived (only for wait=False)
    //##          CLOSED    stream closed
    //##          OUTOFSEQ  next object in stream is not a tile (i.e. a header)
    virtual int getNextTile(VisTile::Ref &tile, bool wait = True);

    //##ModelId=3E0AAB8602A8
    //##Documentation
    //## Checks if a header is waiting in the stream. Return value: same as
    //## would have been returned by getHeader(wait=False).
    virtual int hasHeader();

    //##ModelId=3E0AAB8B01A0
    //##Documentation
    //## Checks if a tile is waiting in the stream. Return value: same as would
    //## have been returned by getNextTile(wait=False).
    virtual int hasTile();

    //##ModelId=3E0AAB920304
    //##Documentation
    //## Tells the agent to suspend the input stream. Agents are not obliged to
    //## implement this. An application can use suspend() to "advise" the input
    //## agent that it is not keeping up with the input data rate; the agent
    //## can use this to determine it queuing or load balancing strategy.
    virtual void suspend();

    //##ModelId=3E0AAB98032A
    //##Documentation
    //## Resumes a stream after a suspend(). 
    virtual void resume();
    
    //##ModelId=3E2807C500A1
    virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const
    { return OctoAgent::sdebug(detail,prefix,name?name:"OctoVisInputAgent"); }

  private:
    //##ModelId=3E0AAB2200CB
    OctoVisInputAgent(const OctoVisInputAgent& right);

    //##ModelId=3E0AAB22024F
    OctoVisInputAgent& operator=(const OctoVisInputAgent& right);
    
    //##ModelId=3E2821060207
    bool suspend_posted;    
};



#endif /* OCTOAGENT_SRC_OCTOVISINPUTAGENT_H_HEADER_INCLUDED_9F6B4F03 */
