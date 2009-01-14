//# ProtocolDefs.h: protocols used by the framework
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

#ifndef PROTOCOLDEFS_H
#define PROTOCOLDEFS_H

#include <MACIO/GCF_Event.h>

namespace LOFAR {
  namespace MACIO {

/**
 * Macro to encode an event's signal from the signal id, protocal an in/out direction
 */
#define F_SIGNAL(prot, sig, inout) (   (((unsigned short)(inout) & 0x3) << 14) \
				     | (((unsigned short)(prot) & 0x3f) << 8)  \
				     | ((unsigned short)(sig) & 0xff)          \
				   )

// Macros for encoding and decoding errornumbers
#define F_ERROR(prot, num) ( (((unsigned short)(prot) & 0x3f) * 100) + (num) )
#define F_ERR_PROTOCOL(errID) ( ((unsigned short)(errID) / 100) & 0x3f )
#define F_ERR_NR(errID) ( (unsigned short)(errID) % 100 )

/**
 * Define different types of signals
 */
#define F_IN    0x01
#define F_OUT   0x02
#define F_INOUT (F_IN | F_OUT)

// Macros for getting  the direction from a signal
#define F_INDIR(signal)  ( ((unsigned short)signal >> 14) & F_IN)
#define F_OUTDIR(signal) ( ((unsigned short)signal >> 14) & F_OUT)

// convenience macros
#define F_ENTRY F_SIGNAL(F_FSM_PROTOCOL, F_ENTRY_ID, F_IN)
#define F_EXIT  F_SIGNAL(F_FSM_PROTOCOL, F_EXIT_ID,  F_IN)
#define F_INIT  F_SIGNAL(F_FSM_PROTOCOL, F_INIT_ID,  F_IN)
#define F_QUIT  F_SIGNAL(F_FSM_PROTOCOL, F_QUIT_ID,  F_IN)

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
    F_PORT_PROTOCOL,            // port connection protocol
    F_GCF_PROTOCOL,             // GCF specific protocol numbers start here
    F_APL_PROTOCOL = 10,    // APPlication specific protocol numbers start here
};

// structure for administration of signalnames and errornames.
struct protocolStrings {
	unsigned short		nrSignals;
	unsigned short		nrErrors;
	const char**		signalNames;
	const char**		errorNames;
};

void registerProtocol (unsigned short					protID, 
					   const struct protocolStrings&	protDef);
string eventName (const GCFEvent& 	e);
string errorName (unsigned short	errorID);

  } // namespace MACIO
} // namespace LOFAR

#endif
