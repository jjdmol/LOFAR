//# GCF_TMProtocols.h: protocols used by the framework
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

#ifndef GCF_PROTOCOLS_H
#define GCF_PROTOCOLS_H

#include <TM/GCF_Event.h>

/**
 * Macro to encode an event's signal from the signal id, protocal an in/out direction
 */
#define F_SIGNAL(prot, sig, inout) (   (((unsigned short)(inout) & 0x3) << 14) \
				     | (((unsigned short)(prot) & 0x3f) << 8)  \
				     | ((unsigned short)(sig) & 0xff)          \
				   )

/**
 * Define different types of signals
 */
#define F_IN    0x01
#define F_OUT   0x02
#define F_INOUT (F_IN | F_OUT)

/**
 * This enum lists all framework protocols. The application protocols should
 * start numbering enums at F_APPLICATION_PROTOCOL, e.g.:
 *
 * enum {
 *   MY_CONTROL_PROTOCOL = F_APPLICATION_PROTOCOL,
 *   MY_MONITORING_PROTOCOL,
 * };
 *
 * NOTE: All application protocols should be enumerated in the same
 * enum to guarantee application global uniqueness.
 *
 */
enum { 
    F_FSM_PROTOCOL = 1,     // state machine protocol (encoded in msb)
    F_PORT_PROTOCOL,        	// port connection protocol
    F_GCF_PROTOCOL, 			// GCF specific protocol numbers start here
    F_APL_PROTOCOL = 10, 	// APPlication specific protocol numbers start here
};

/**
 * F_FSM_PROTOCOL signals
 */
enum { 
    F_ENTRY_ID = 1, // state entry; currently unused (encoded in lsb)
    F_EXIT_ID,      // state exit; currently unused
    F_INIT_ID,      // initial transition
};

// convenience macros
#define F_ENTRY_SIG F_SIGNAL(F_FSM_PROTOCOL, F_ENTRY_ID, F_IN)
#define F_EXIT_SIG  F_SIGNAL(F_FSM_PROTOCOL, F_EXIT_ID,  F_IN)
#define F_INIT_SIG  F_SIGNAL(F_FSM_PROTOCOL, F_INIT_ID,  F_IN)

extern const char* F_FSM_PROTOCOL_names[]; // defined in GCF_Protocols.cc

/**
 * F_PORT_PROTOCOL signals
 */
enum {
    F_CONNECT_ID = 1,   // sent to the accept port to connect
                      // to a specific SPP port
    F_CONNECTED_ID,     // sent to the port to indicate completion
    F_DISCONNECTED_ID,  // sent to the port that is disconnected
    F_CLOSED_ID,        // to delay closing a port
    F_TIMER_ID,         // timer expired
    F_DATAIN_ID,        // data available for reading
    F_DATAOUT_ID,       // space available to write
    F_RAW_ID,           // no event!, only the data is sent (used with direct ports)
    F_ACCEPT_REQ_ID,           // this event will be used by the port provider to indicate the user that there is one more connect request of a client (SAP)
};

#define F_CONNECT_SIG				F_SIGNAL(F_PORT_PROTOCOL, F_CONNECT_ID,      			F_IN)
#define F_CONNECTED_SIG			F_SIGNAL(F_PORT_PROTOCOL, F_CONNECTED_ID,    		F_IN)
#define F_DISCONNECTED_SIG 	F_SIGNAL(F_PORT_PROTOCOL, F_DISCONNECTED_ID, 	F_IN)
#define F_CLOSED_SIG       			F_SIGNAL(F_PORT_PROTOCOL, F_CLOSED_ID,       			F_IN)
#define F_TIMER_SIG        				F_SIGNAL(F_PORT_PROTOCOL, F_TIMER_ID,        				F_IN)
#define F_DATAIN_SIG       			F_SIGNAL(F_PORT_PROTOCOL, F_DATAIN_ID,       				F_IN)
#define F_DATAOUT_SIG      			F_SIGNAL(F_PORT_PROTOCOL, F_DATAOUT_ID,      			F_IN)
#define F_RAW_SIG          				F_SIGNAL(F_PORT_PROTOCOL, F_RAW_ID,          				F_INOUT)
#define F_ACCEPT_REQ_SIG        F_SIGNAL(F_PORT_PROTOCOL, F_ACCEPT_REQ_ID,		F_IN)

extern const char* F_PORT_PROTOCOL_names[]; // defined in GCF_Protocols.cxx

struct GCFTimerEvent : public GCFEvent
{
    GCFTimerEvent() : GCFEvent(F_TIMER_SIG)
    {
        length = sizeof(GCFTimerEvent);
    }

    long        sec;
    long        usec;
    const void* arg;
};

#endif
