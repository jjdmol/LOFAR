//#  NenuFarMsg.h: interface of the NenuFarMsg class
//#
//#  Copyright (C) 2002-2009
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

#ifndef BEAMSERVER_NENUFARMSG_H_
#define BEAMSERVER_NENUFARMSG_H_

#include <lofar_config.h>
#include <Common/lofar_list.h>
#include <Common/lofar_vector.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/StationDatatypes.h>
#include <APL/IBS_Protocol/Pointing.h>

namespace LOFAR {
  namespace BS {

const uint	NEW_BEAM_MSG		= 1;
const uint	NEW_BEAM_ACK_MSG	= 2;
const uint	STOP_BEAM_MSG		= 3;
const uint	STOP_BEAM_ACK_MSG	= 4;
const uint	ABORT_BEAM_MSG		= 5;
const uint	ABORT_BEAM_ACK_MSG	= 6;
const uint	ABORT_ALL_BEAMS_MSG	= 7;

const uint	HDR_SIZE	 		= 8;
const uint	VERSION_SIZE 		= 14;
const uint	MSGTYPE_KEY_SIZE	= 8;

// Class representing a single beam allocated by a client
// using a BEAMALLOC event.
class NenuFarMsg {
public:
	// Constructors
	NenuFarMsg(const uint16		version,
			   uint				msgType,
			   vector<string>	data);
	NenuFarMsg(const uint16		version,
			   uint				msgType,
			   ParameterSet&	data);

	// constructor for converting received TCP data to a NenuFarMsg.
	NenuFarMsg(const char*		buf,
			   size_t			nrBytes);

	// Default destructor.
	virtual ~NenuFarMsg();

	// Access to the buffer (only needed for unit test)
	char*	data() { return (itsPackedMsg); }
	size_t	size() { return (itsMsgSize); }

	// Remove a beam from the admin
	static vector<string>	unpack2vector(char*	TCPbuffer, size_t	nrBytes);
	static ParameterSet		unpack2parset(char*	TCPbuffer, size_t	nrBytes);

	// print function for operator<<
	ostream& print (ostream& os) const;

	// operator definitions
	inline bool operator==(const NenuFarMsg& that) const {
		return((itsMsgSize == that.itsMsgSize) && memcmp(itsPackedMsg, that.itsPackedMsg, itsMsgSize) == 0);
	}
	inline bool operator!=(const NenuFarMsg& that) const {
		return (!(*this==that));
	}

	// Don't allow copying this object.
	//NenuFarMsg (const NenuFarMsg&) {};
	//NenuFarMsg& operator= (const NenuFarMsg&) {};

	// Handy functions for visualising the msgtypes.
	int 	msgTypeSize  (uint	msgType) const;
	string	msgTypeString(uint	msgType) const;

private:
	//# ----- DATAMEMBERS -----
	// queue of future pointings as delivered by the user.
	char*		itsPackedMsg;
	size_t		itsMsgSize;
};

//# -------------------- inline functions --------------------
//
// operator <<
//
inline ostream& operator<<(ostream& os, const NenuFarMsg& nnfm)
{
	return (nnfm.print(os));
}


  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* BEAMSERVER_NENUFARMSG_H_ */
