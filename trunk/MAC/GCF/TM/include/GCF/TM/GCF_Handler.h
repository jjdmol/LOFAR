//#  GCF_Handler.h: abstract baseclass for all concrete message handlers
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

#ifndef GCF_HANDLER_H
#define GCF_HANDLER_H

//# Includes


namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

//# forward declaration
class GCFTask;

/**
  This is the pure abstract (interface) class for all different kind of
  handlers. A handler is a placeholder for all services in an application,
  which want to be part of the application main loop. It was very important to
  create a possibility to manage different handlers, which run in the same
  thread/process and invoked repeatedly, as fast and often as possible.
  So this concept is introduced because of the following requirements of a GCF
  based application:
  1) It shall be possible to use the PVSS API library, which implies hidden
  communication channels for the application with the PVSS system. These
  channels have to be activated (dispatch) as fast and often as possible in a
  certain time to effectuate PVSS features in the application.
  2) It shall be possible to use communication channels (like TCP, ETH).
  3) It shall be possible to run all handlers in one thread (one main loop ==
  less complexity)
  4) It should support transparent event driven processing.
  
  At this moment the following main services/handlers in GCF are identified:
  1) handling incoming and outgoing messages on linux files/devices (like via
  a TCP socket or a parallel port driver)
  2) handling incoming and outgoing PVSS messages by means of a so called
  dispatch (polling mechanism)
  3) handling started timers on ports and generate messages for related task
  on timeout
  
  A nice side effect is that other handlers can be developed for GCF or APL
  (for instance application maintenance on timers), which also should be part
  of the main loop and invoked repeatedly, and as fast and often as possible.
  A new handler registers itself on the static register of the GCFTask class.
  
  Another important issue is related to the priority of handling different
  event streams (PVSS or ports). From Applications point of view it is
  important to know that the event stream over ports (other than PVSS) will be
  handled as fast and often as possible. Two handlers, PVSS and Socket, are
  based on the socket pattern. They work with a max. timeout value to wait for
  incoming events. That’s why the (max.) timeout of the PVSS handler must be
  very low and the timeout of FD Handler significant higher than of the PVSS
  handler. Here an overview what the timeout could be (for now; these values
  have to be fine tuned when testing the performance).

  Handler |Timeout
  --------+-------
  PVSS    | 1µs
  Socket  | 10ms 
 */

class GCFHandler
{
  public:
    /// @return true if no other objects use this handler anymore otherwise false
    bool mayDeleted() { return (_usecount == 0);}
    /// increments the uscount
    void use() { _usecount++;}
    /// decrements the uscount
    void leave() { _usecount--;}
  
  protected:
    GCFHandler();
    virtual ~GCFHandler() {;}

    friend class GCFTask;

    virtual void workProc () = 0;
    virtual void stop () = 0;

  private:
    //@{ 
    /// Don't allow copying this object.
    GCFHandler (const GCFHandler&);
    GCFHandler& operator= (const GCFHandler&);  
    //@}
    
    /// count the usage of this handler    
    unsigned int _usecount;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR


#include <GCF/TM/GCF_Task.h>

using namespace LOFAR::GCF::TM;

inline GCFHandler::GCFHandler() : _usecount(0) 
{
  GCFTask::registerHandler(*this);
}
#endif
