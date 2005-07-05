//#  APLInterTaskPort.h: Direct communication between two tasks in the same process
//#
//#  Copyright (C) 2002-2003
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

#ifndef APLInterTaskPort_H
#define APLInterTaskPort_H

#include <set>
#include <GCF/TM/GCF_RawPort.h>

namespace LOFAR
{
  namespace APLCommon
  {
    
// forward declaration
class GCF::TM::GCFTask;

/**
 * This class represents a protocol port that is used to exchange events defined 
 * in a protocol betweem two tasks in the same process. Events are dispatched
 * to the state machine of the client task.
 */
class APLInterTaskPort : public GCF::TM::GCFRawPort
{
  public:

    /**
    * constructors
    * @param protocol NOT USED
    */
    APLInterTaskPort(GCF::TM::GCFTask& slaveTask, 
                     GCF::TM::GCFTask& containertask, 
                     string& name, 
                     TPortType type, 
                     int protocol);
    
    
    /**
    * destructor
    */
    virtual ~APLInterTaskPort ();
    
    /**
    * open/close functions
    */
    virtual bool open ();
    virtual bool close ();
    
//    virtual bool setRemoteAddr(const string& remotetask, const string& remoteport);
    /**
    * send/recv functions
    */
    virtual ssize_t send (GCF::TM::GCFEvent& event);                          
    virtual ssize_t recv (void* buf, 
                          size_t count);
                           
    virtual ssize_t sendBack(GCF::TM::GCFEvent& e);

  protected:
    virtual GCF::TM::GCFEvent::TResult   dispatch (GCF::TM::GCFEvent& event);
  
  private:

    /**
    * Don't allow copying this object.
    */
    APLInterTaskPort ();
    APLInterTaskPort (const APLInterTaskPort&);
    APLInterTaskPort& operator= (const APLInterTaskPort&);

  private:

    GCF::TM::GCFTask&       m_slaveTask;
    std::set<long> m_toClientTimerId;
    std::set<long> m_toServerTimerId;

    ALLOC_TRACER_CONTEXT  
};

  }; // APL
}; // LOFAR
#endif
