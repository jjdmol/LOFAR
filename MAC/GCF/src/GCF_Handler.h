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


//# forward declaration
class GCFTask;

/**
 * This is the pure abstract (interface) class for all different kind of 
 * handlers, which wants to be added to the application main loop. All handlers 
 * together are responsible to guarantee that all incoming and outgoing events 
 * from/to outside of a task (or process) will be serviced. So each handler 
 * services a part of all possible events when their workProc method is called. 
 * With other words: In the main loop of the applications all specialised 
 * workProc operations of the registered handlers will be repeatedly invoked 
 * in the same order.
 * This concept is introduced because of the two facts:
 * 1) the PVSS API library for Linux is not thread safe and compiled with 
 *    GNU 2.96 (in case of RedHat 7.2). A compiler of version 3.0 or higher 
 *    cannot compile application or library code written for the GNU 2.96. 
 * 2) and other communication chancels should be possible. 
 * So it was very important to create a possibility to manage different message 
 * handlers, which runs in the same thread/process and supports together 
 * transparent event driven processing.
 * A new handler must be registered on the static register of the GCFTask class.
 * An other important issue is related to the priority of handling different 
 * event streams (PVSS or ports). From Applications point of view it is 
 * important to know that the event stream over ports will be handled as fast 
 * as possible. Two, SCADA and Socket, handlers are based on the socket pattern. 
 * They work with a max. timeout value to wait for incoming events. That's why 
 * the (max.) timeout of the SCADA handler must be very low and the timeout of 
 * Socket Handler significant higher than of the SCADA handler. Here an overview 
 * what the timeout could be (for now; these values has to be fine tuned during 
 * testing the performance).
 * 
    Handler   Timeout
    SCADA     1us
    Socket    10ms
 * 
 */

class GCFHandler
{
  protected:
    GCFHandler() {;}
    virtual ~GCFHandler() {;}

    virtual void workProc () = 0;
    virtual void stop () = 0;
    friend class GCFTask;

  private:
    //@{ 
    /// Copy contructors. Don't allow copying this object.
    GCFHandler (const GCFHandler&);
    GCFHandler& operator= (const GCFHandler&);  
    //@}
};
#endif
