//#  NenuFarMsg.h: interface of the NenuFarMsg class
//#
//#  Copyright (C) 2014
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
//#  $Id: NenuFarMsg.h 22248 2012-10-08 12:34:59Z overeem $
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <Common/hexdump.h>
#include "NenuFarMsg.h"

using namespace LOFAR;
using namespace BS;


//
// NenuFarAdmin(version, type, vector)
//
NenuFarMsg::NenuFarMsg(const uint16	version, uint	msgType, vector<string>	data)
{
	// first calc the size of the buffer we need.
	itsMsgSize = HDR_SIZE + VERSION_SIZE + MSGTYPE_KEY_SIZE + msgTypeSize(msgType);
	for (size_t i = 0; i < data.size(); ++i) {
		itsMsgSize += data[i].size() + 1;
	}
	itsMsgSize++;		// trailing zero
	itsPackedMsg = new char[itsMsgSize];
	
	// construct message
	int offset = sprintf(itsPackedMsg, "%7.7d%cversion=%02d.%02d\nmsgtype=%s\n", itsMsgSize - HDR_SIZE, 0x00, 
						((version & 0xFF00) >> 8) % 100, (version & 0xFF) %100, msgTypeString(msgType).c_str());
	for (size_t i = 0; i < data.size(); ++i) {
		offset += sprintf(&itsPackedMsg[offset], "%s\n", data[i].c_str());
	}
	itsPackedMsg[itsMsgSize-1] = '\0';
}
	
//
// NenuFarAdmin(version, type, ParameterSet)
//
NenuFarMsg::NenuFarMsg(const uint16	version, uint	msgType, ParameterSet&	data)
{
	// first calc the size of the buffer we need.
	itsMsgSize = HDR_SIZE + VERSION_SIZE + MSGTYPE_KEY_SIZE + msgTypeSize(msgType);
	ParameterSet::iterator		iter = data.begin();
	ParameterSet::iterator		end  = data.end();
	while (iter != end) {
		itsMsgSize += iter->first.size() + 1 + iter->second.get().size() + 1;
		++iter;
	}
	itsMsgSize++;		// trailing zero
	itsPackedMsg = new char[itsMsgSize];
	
	// construct message
	int offset = sprintf(itsPackedMsg, "%7.7d%cversion=%02d.%02d\nmsgtype=%s\n", itsMsgSize - HDR_SIZE, 0x00, 
						((version & 0xFF00) >> 8) % 100, (version & 0xFF) %100, msgTypeString(msgType).c_str());
	iter = data.begin();
	end  = data.end();
	while (iter != end) {
		offset += sprintf(&itsPackedMsg[offset], "%s=%s\n", iter->first.c_str(), iter->second.get().c_str());
		++iter;
	}
	itsPackedMsg[itsMsgSize-1] = '\0';
}

//
// NenuFarMsg(bufPtr, bufsize)
//
NenuFarMsg::NenuFarMsg(const char*	buf, size_t		nrBytes)
{
	ParameterSet	content;
	content.adoptBuffer(string(buf, nrBytes));
	// minimal sanity check.
	ASSERTSTR(content.isDefined("version"), "Key 'version' not in the constructionbuffer");
	ASSERTSTR(content.isDefined("msgtype"), "Key 'msgtype' not in the constructionbuffer");

	itsPackedMsg = new char [nrBytes];
	itsMsgSize   = nrBytes;
	memcpy(&itsPackedMsg, buf, nrBytes);
}

NenuFarMsg::~NenuFarMsg()
{
	delete [] itsPackedMsg;
}

int NenuFarMsg::msgTypeSize(uint	msgType)
{
	return (msgTypeString(msgType).size() + 1);
}

string NenuFarMsg::msgTypeString(uint	msgType) 
{
	switch (msgType) {
	case NEW_BEAM_MSG:			return ("NEW_BEAM");
	case NEW_BEAM_ACK_MSG:		return ("NEW_BEAM_ACK");
	case STOP_BEAM_MSG:			return ("STOP_BEAM");
	case STOP_BEAM_ACK_MSG:		return ("STOP_BEAM_ACK");
	case ABORT_BEAM_MSG:		return ("ABORT_BEAM");
	case ABORT_BEAM_ACK_MSG:	return ("ABORT_BEAM_ACK");
	case ABORT_ALL_BEAMS_MSG:	return ("ABORT_ALL_BEAMS");
	default:
		ASSERTSTR(false, "Message type " << msgType << " is not a valid value.");
	}
}

vector<string>	NenuFarMsg::unpack2vector(char*	TCPbuffer, size_t	nrBytes)
{
	vector<string>	result;
	if (nrBytes <= HDR_SIZE) {
		return (result);
	}

	TCPbuffer[nrBytes-1] = '\0';						// better save than sorry
	char*	token = strtok(&TCPbuffer[HDR_SIZE], "\n");
	while (token) {
		result.push_back(token);
		token = strtok(NULL, "\n");
	}
	return(result);
}

ParameterSet	NenuFarMsg::unpack2parset(char*	TCPbuffer, size_t	nrBytes)
{
	ParameterSet	result;
	if (nrBytes <= HDR_SIZE) {
		return (result);
	}

	TCPbuffer[nrBytes-1] = '\0';	// better save than sorry
	result.adoptBuffer(&TCPbuffer[HDR_SIZE]);
	return (result);
}

ostream& NenuFarMsg::print (ostream& os) const
{
	string	s;
	hexdump(s, itsPackedMsg, itsMsgSize);
	os << s;
	return os;
}
