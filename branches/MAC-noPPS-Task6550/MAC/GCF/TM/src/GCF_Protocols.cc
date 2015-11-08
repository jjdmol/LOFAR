//#  GCF_Protocols.cc: protocols used by the framework
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_map.h>
#include <Common/StringUtil.h>
#include <GCF/TM/GCF_Protocols.h>

namespace LOFAR {
	namespace GCF {
		namespace TM {

typedef map<unsigned short, const struct protocolStrings*> protStringsMap;
static protStringsMap 	_protNameTable;

//
// F_FSM_PROTOCOL signal names
//
const char* F_FSM_PROTOCOL_names[] =
{
  "F_FSM_PROTOCOL: invalid signal",
  "F_ENTRY",
  "F_EXIT",
  "F_INIT",
  "F_QUIT",
  "F_TRAN"
};

//
// F_PORT_PROTOCOL signal names
//
const char* F_PORT_PROTOCOL_names[] =
{
  "F_PORT_PROTOCOL: invalid signal",
  "F_CONNECT",
  "F_CONNECTED",
  "F_DISCONNECTED",
  "F_CLOSED",
  "F_TIMER",
  "F_DATAIN",
  "F_DATAOUT",
  "F_RAW_DATA",
  "F_ACCEPT_REQ",
};

const struct protocolStrings F_FSM_PROTOCOL_STRINGS = {
	6, 0, F_FSM_PROTOCOL_names, 0
};

const struct protocolStrings F_PORT_PROTOCOL_STRINGS = {
	10, 0, F_PORT_PROTOCOL_names, 0
};

//
// registerProtocol(protID, protocolStrings)
//
void registerProtocol (unsigned short					protID, 
					   const struct protocolStrings&	protDef)
{
	_protNameTable[protID] = &protDef;
	LOG_DEBUG_STR ("registered protocol: " << protID);
}

//
// eventName(event&)
//
string eventName(const GCFEvent& e)
{
	protStringsMap::const_iterator iter = _protNameTable.find(F_EVT_PROTOCOL(e));
	if ((iter != _protNameTable.end()) && (F_EVT_SIGNAL(e) <= iter->second->nrSignals)) {
		return ((iter->second->signalNames)[F_EVT_SIGNAL(e)]);
	}

	return (formatString("unknown signal(protocol=%d, signal=%d)", 
							F_EVT_PROTOCOL(e), F_EVT_SIGNAL(e)));
}

//
// errorName(errorNr)
//
string errorName(unsigned short	errorID)
{
	protStringsMap::const_iterator iter = _protNameTable.find(F_ERR_PROTOCOL(errorID));
	if ((iter != _protNameTable.end()) && (F_ERR_NR(errorID) <= iter->second->nrErrors)) {
		return ((iter->second->errorNames)[F_ERR_NR(errorID)]);
	}

	return (formatString("unknown error(protocol=%d, error=%d)", 
							F_ERR_PROTOCOL(errorID), F_ERR_NR(errorID)));
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
