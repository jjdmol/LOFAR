//  VisFileInputAgent.h: common base class for VisInputAgents that read files
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

#ifndef _VISAGENT_VISFILEINPUTAGENT_H
#define _VISAGENT_VISFILEINPUTAGENT_H 1

#include <VisAgent/VisInputAgent.h>
#include <VisAgent/VisFileAgentBase.h>

//##ModelId=3DF9FECD0159
//##Documentation
//## VisFileInputAgent implements some common features & services for agents
//## that get their input from a static data file (e.g., AIPS++ MSs, BOIO
//## files).
class VisFileInputAgent : public VisInputAgent, public VisFileAgentBase
{
  protected:
      
  private:
    //##ModelId=3DF9FECE0055
      DataRecord::Ref  header_;
    //##ModelId=3E281884006E
      bool suspended_;
      
  protected:
      
    //##ModelId=3DF9FECE00FC
      VisFileInputAgent ()
          : suspended_(False)
          {}
  
      // returns header by reference
    //##ModelId=3DF9FECE0384
      DataRecord &  header     ()
      { return header_.dewr(); }

      // initializes an empty header
    //##ModelId=3DF9FECE03AB
      DataRecord &  initHeader ();
      
    //##ModelId=3E2819650278
      bool suspended () const
      { return suspended_; }
          
  public:
      
    //##ModelId=3DF9FECE03D1
      virtual int getHeader (DataRecord::Ref &hdr,int wait = AppAgent::WAIT);
  
    //##ModelId=3DF9FECF009D
      virtual int hasHeader ();
      
    //##ModelId=3DF9FECF00BF
      virtual int hasTile   ();
      
      //##ModelId=3E281894014E
    //##Documentation
    //## Tells the agent to suspend the input stream. Agents are not obliged to
    //## implement this. An application can use suspend() to "advise" the input
    //## agent that it is not keeping up with the input data rate; the agent
    //## can use this to determine it queuing or load balancing strategy.
      virtual void suspend()
      { suspended_ = True; }

      //##ModelId=3E28189901DD
      //##Documentation
      //## Resumes a stream after a suspend(). 
      virtual void resume()
      { suspended_ = False; }
      
      
      //##ModelId=3E2C299201D6
      //##Documentation
      //## Reports current state of agent. Default version always reports success
      virtual int state() const;

      //##ModelId=3E2C2999029A
      //##Documentation
      //## Reports current state as a string
      virtual string stateString() const;

};


#endif
    
