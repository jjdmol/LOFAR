//#  APAdmin.cc: Enables a poll on a collection of dataholders.
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <ACC/APAdmin.h>

namespace LOFAR {
  namespace ACC {


//
// APAdmin(Socket*)
//
// Constructs a new APAdmin object including a blank DH_ProcControl. The given
// Socket is attached to the APAdmin.
//
APAdmin::APAdmin(Socket*	aSocket) :
	itsDHPC         (new DH_ProcControl),
	itsSocket       (aSocket),
	itsBytesToRead  (0),
	itsReadOffset   (0),
	itsReadingHeader(true),
	itsState        (APSconn)
{
	itsBytesToRead = itsDHPC->getHeaderSize();	// always start with the header
	itsDHPC->init();							// construct DH layout
}

//
// ~APAdmin()
//
APAdmin::~APAdmin()
{
	if (itsDHPC) {				// the 'if's are superfluous but safer.
		delete	itsDHPC;
	}
	if (itsSocket) {
		delete	itsSocket;
	}
}


//
// bool read ()
//
// Tries to read the missing bytes from the Socket into the DataHolder.
// When the DataHolder is full true is returned, otherwise false.
//
bool APAdmin::read()
{
	LOG_TRACE_RTTI ("APAdmin:read");

	if (itsState == APSdiscon || itsState == APSfail) {
		return(false);
	}

	itsState = APSread;

	int32	newBytes = itsSocket->read(
				static_cast<char*>(itsDHPC->getDataPtr()) + itsReadOffset, 
				itsBytesToRead);

	if (newBytes < 0) {
		if (newBytes != Socket::INCOMPLETE) { 	// serious error
			itsState = APSfail;
		}
		return (false);
	}

	itsBytesToRead -= newBytes;				// update admin
	itsReadOffset  += newBytes;

	if (itsBytesToRead > 0) {				// still bytes missing?
		return (false);
	}

	if (!itsReadingHeader) {							// Last part read?
		itsBytesToRead   = itsDHPC->getHeaderSize();	// prepare for next msg
		itsReadingHeader = true;
		itsReadOffset    = 0;
		itsDHPC->unpack();								// unpack the data
		return (true);									// tell msg is avail.
	}

	// prepare for reading the remaining datapart.
	itsReadingHeader     = false;
	int32	totalMsgSize = DataHolder::getDataLength (itsDHPC->getDataPtr());
	itsDHPC->resizeBuffer (totalMsgSize);
	itsBytesToRead       = totalMsgSize - itsDHPC->getHeaderSize();

	return (read());						// call ourself for last part
}

//
// bool write (buffer, size)
//
// Write the given bytes to the Socket.
// Returns false if socket is/turns_out_to_be disconnected.
//
bool APAdmin::write(void*		aBuffer,
					int32		aSize)
{
	LOG_TRACE_RTTI ("APAdmin:write");

	if (itsState == APSdiscon || itsState == APSfail) {		// check current state
		return(false);
	}

	int32 result = itsSocket->write(aBuffer, aSize);		// do write

	if ((result < 0 ) && (result != Socket::INCOMPLETE)) {	// serious error?
		itsState = APSfail;									// set failure
		return (false);
	}
	
	itsState = APSwrite;									// seems ok
	return (true);
}


  } // namespace ACC
} // namespace LOFAR
