//#  GCF_Port.h: connection to a remote process
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

#ifndef GCF_PORT_H
#define GCF_PORT_H

#include "GCF_PortInterface.h"
#include <GCF_Event.h>
#include <GCF_Defines.h>
#include "GCF_PeerAddr.h"

// forward declaration
class GCFTask;
class GCFRawPort;

/**
 *
 * The GCFPort class implements the GCFFPortInterface abstraction in a way that allows the 
 * underlying implementation in terms of transport to vary. In this way a protocol
 * can be defined and be communicated over TCP or Serial Port or dedicated interface
 * to an FPGA without changing the code!
 * 
 */
class GCFPort : public GCFPortInterface
{
    public:

        /**
        * constructors
        */
        GCFPort(GCFTask& containertask,
                        string& name,
                        TPortType type,
                        int protocol);
        
        GCFPort();

        /**
        * destructor
        */
        virtual ~GCFPort();

        /* initialize function, to follow-up default constructor */
        void init(GCFTask& containertask, string& name, TPortType type, int protocol);

        /**
        * open/close functions
        */
        virtual int open();
        virtual int close();

        /**
        * send/recv functions
        */
        virtual ssize_t send(const GCFEvent& event,
		                                 void* buf = 0, size_t count = 0);
        virtual ssize_t sendv(const GCFEvent& event,
			                             const iovec buffers[], int n);
        virtual ssize_t recv(void* buf,     size_t count);
        virtual ssize_t recvv(iovec buffers[], int n);

        /**
        * Timer functions.
        * Upon expiration of a timer a F_TIMER_SIG will be
        * received on the port.
        */
  
        virtual long setTimer(long  delay_sec,
                                            long  delay_usec    = 0,
                                            long  interval_sec  = 0,
                                            long  interval_usec = 0,
                                            const void* arg     = 0);

        virtual long setTimer(double delay_seconds, 
                                			double interval_seconds = 0.0,
                                			const void*  arg        = 0);

        virtual int  cancelTimer(long   timerid,
			                                 const void** arg = 0);

        /*
        * @note This function is suspect of hanging machines. It
        * relies directly on the ACE implementation and there is
        * a possible bug in the ACE implementation.
        * DO NOT USE THIS FUNCTION.
        */
        virtual int  cancelAllTimers();

        virtual int  resetTimerInterval(long timerid,
                                                long sec,
                                                long usec = 0);

    private:

        /**
        * Don't allow copying of the FPort object.
        */
        GCFPort(const GCFPort&);
        GCFPort& operator=(const GCFPort&);
        /// print debug string of event
        void debug_signal(const GCFEvent& e);
        void debug_send(const GCFEvent& e);
        void debug_dispatch(const GCFEvent& e);

    private:
        /**
        * Dispatch method is called by implementation subclasses.
        *
        * Should return:
        * 0 on success
        * > 0 on handled
        * -1 to close the connection
        */
        friend class GCFRawPort; // to access dispatch
        int dispatch(GCFEvent& event);
        //inline void setIsConnected(bool connected) {_isConnected = connected;}
 
    private:

        GCFPeerAddr     _localAddr;
        GCFPeerAddr     _remoteAddr;

        GCFPortInterface* _pSlave;
};
#endif
