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

#include <boost/scoped_array.hpp>
#include <GCF/GCF_RawPort.h>

// forward declaration
class GCFTask;

/**
 * This class represents a protocol port that is used to exchange events defined 
 * in a protocol betweem two tasks in the same process. Events are dispatched
 * to the state machine of the client task.
 */
class APLInterTaskPort : public GCFRawPort
{
  public:

    /**
    * constructors
    * @param protocol NOT USED
    */
    APLInterTaskPort(GCFTask& slaveTask, 
                     GCFTask& containertask, 
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
    virtual int open ();
    virtual int close ();
    
    /**
    * send/recv functions
    */
    virtual ssize_t send (const GCFEvent& event, 
                          void* buf = 0, 
                          size_t count = 0);                          
    virtual ssize_t sendv (const GCFEvent& event,
                           const iovec buffers[], 
                           int n);
    virtual ssize_t recv (void* buf, 
                          size_t count);
    virtual ssize_t recvv (iovec buffers[], 
                           int n);
                           
    virtual ssize_t sendBack(const GCFEvent& e, void* buf=0, size_t count=0);

  protected:
    virtual GCFEvent::TResult   dispatch (GCFEvent& event);
  
  private:

    /**
    * Don't allow copying this object.
    */
    APLInterTaskPort ();
    APLInterTaskPort (const APLInterTaskPort&);
    APLInterTaskPort& operator= (const APLInterTaskPort&);

  private:

    GCFTask&                            m_slaveTask;
    boost::scoped_array<unsigned char>  m_toClientBuffer;
    boost::scoped_array<unsigned char>  m_toServerBuffer;
};
#endif
