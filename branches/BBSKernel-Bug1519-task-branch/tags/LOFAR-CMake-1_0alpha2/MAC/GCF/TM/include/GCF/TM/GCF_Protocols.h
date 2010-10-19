//# GCF_Protocols.h: protocols used by the framework
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

#include <MACIO/ProtocolDefs.h>
#include <MACIO/GCF_Event.h>

namespace LOFAR {
 using namespace MACIO;
 namespace GCF {
  namespace TM {

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

/**
 * F_FSM_PROTOCOL signals
 */
enum { 
    F_ENTRY_ID = 1, // state entry; currently unused (encoded in lsb)
    F_EXIT_ID,      // state exit; currently unused
    F_INIT_ID,      // initial transition
    F_QUIT_ID,      // final run
	F_TRAN_ID,		// state transition (internal, never send to FSM)
};

// convenience macros
#define F_ENTRY F_SIGNAL(F_FSM_PROTOCOL, F_ENTRY_ID, F_IN)
#define F_EXIT  F_SIGNAL(F_FSM_PROTOCOL, F_EXIT_ID,  F_IN)
#define F_INIT  F_SIGNAL(F_FSM_PROTOCOL, F_INIT_ID,  F_IN)
#define F_QUIT  F_SIGNAL(F_FSM_PROTOCOL, F_QUIT_ID,  F_IN)
#define F_TRAN  F_SIGNAL(F_FSM_PROTOCOL, F_TRAN_ID,  F_IN)

extern const char* F_FSM_PROTOCOL_names[]; 
extern const struct protocolStrings F_FSM_PROTOCOL_STRINGS; 

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
    F_RAW_DATA_ID,      // no event!, only the data is sent (used with direct ports)
    F_ACCEPT_REQ_ID,    // indicatation of the port provider to the user about a client connect request (SAP)
};

#define F_CONNECT       F_SIGNAL(F_PORT_PROTOCOL, F_CONNECT_ID,       F_IN)
#define F_CONNECTED     F_SIGNAL(F_PORT_PROTOCOL, F_CONNECTED_ID,     F_IN)
#define F_DISCONNECTED  F_SIGNAL(F_PORT_PROTOCOL, F_DISCONNECTED_ID,  F_IN)
#define F_CLOSED        F_SIGNAL(F_PORT_PROTOCOL, F_CLOSED_ID,        F_IN)
#define F_TIMER         F_SIGNAL(F_PORT_PROTOCOL, F_TIMER_ID,         F_IN)
#define F_DATAIN        F_SIGNAL(F_PORT_PROTOCOL, F_DATAIN_ID,        F_IN)
#define F_DATAOUT       F_SIGNAL(F_PORT_PROTOCOL, F_DATAOUT_ID,       F_IN)
#define F_RAW_DATA      F_SIGNAL(F_PORT_PROTOCOL, F_RAW_DATA_ID,      F_INOUT)
#define F_ACCEPT_REQ    F_SIGNAL(F_PORT_PROTOCOL, F_ACCEPT_REQ_ID,    F_IN)

extern const char* F_PORT_PROTOCOL_names[];
extern const struct protocolStrings F_PORT_PROTOCOL_STRINGS; 

//
// Define GCFTimerEvent
//
struct GCFTimerEvent : public GCFEvent
{
  GCFTimerEvent() : GCFEvent(F_TIMER)
  {
    length = sizeof(GCFTimerEvent);
  }

  long        sec;
  long        usec;
  unsigned long id;
  void* arg;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
