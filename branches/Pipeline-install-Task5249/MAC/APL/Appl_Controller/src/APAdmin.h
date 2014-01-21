//#  APAdmin.h: Internal administration of an AP for the Appl. Controller.
//#
//#  Copyright (C) 2004
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_APADMIN_H
#define LOFAR_ACCBIN_APADMIN_H

// \file
// Internal information of an Application Process used by the
// Application Controller.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/Net/Socket.h>
#include <PLC/DH_ProcControl.h>

using namespace LOFAR::ACC::PLC;

namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

// The APAdmin class uses a internal variable to register the state of the
// Socket.
enum APAState {
		APSconn, APSdiscon, APSread, APSwrite, APSfail
};

// The APAdmin class is a collection of information the Application Controller
// needs for its administration of the Application Processes (AP's). 
// It consists of a dataholder for reading data from the AP, a Socket the AP 
// is connected to, and some less important flags and values.
class APAdmin
{
public:
	// An APAdmin is always needed after a new Socket was created after a
	// call on a listener socket. So the constructor always needs this Socket.
	// An empty dataholder for the AP is constructed automatically.
	explicit APAdmin (Socket*	aSocket);
	
	~APAdmin();

	// \name Functions for reading and writing to the AP.
	// @{

	// Tries to read the missing bytes from the Socket into the DataHolder.
	// When the dataholder is full \c true is returned, otherwise \c false.
	bool 	read();

	// Writes the contents of the dataHolder to the Socket. Returns \c false
	// if the Socket is/turns out to be disconnected.
	bool 	write(void*		aBuffer,
				  int32		aSize);
	// @}

	// \name Accessor functions.
	// @{
	void				setName(const string&	aName);
	string				getName()     const;
	DH_ProcControl*		getDH()		  const;
	int32				getSocketID() const;
	APAState			getState()	  const;
	// @}

private:
	// Not default constructable;
	APAdmin();

	// Copying is not allowed
	APAdmin(const APAdmin&	that);

	// Copying is not allowed
	APAdmin& operator=(const APAdmin& that);

	//# --- datamembers ---
	// name of application process
	string				itsName;			

	// The dataholder used for reading from the AP.
	DH_ProcControl*		itsDHPC;			

	// Socket to AP is connected to
	Socket*				itsSocket;			

	// Number of bytes still to read
	uint32				itsBytesToRead;		

	// Readoffset in the buffer
	uint32				itsReadOffset;		

	// Reading header or data part
	bool				itsReadingHeader;	

	// State of APAdmin
	APAState			itsState;			
};

//# -------------------- inline functions --------------------
//#
//# getDH()
//#
inline DH_ProcControl* APAdmin::getDH() const
{
	return (itsDHPC);
}

//#
//# getSocketID()
//#
inline int32 APAdmin::getSocketID() const
{
	return (itsSocket->getSid());
}

//#
//# getState()
//#
inline APAState APAdmin::getState() const
{
	return (itsState);
}

//#
//# setName(name)
//#
inline void APAdmin::setName(const string&	aName) 
{
	itsName = aName;
	itsState = APSconn;
}

//#
//# getName()
//#
inline string	APAdmin::getName() const
{
	return (itsName);
}

// @} addgroup
  } // namespace ACC
} // namespace LOFAR
#endif
