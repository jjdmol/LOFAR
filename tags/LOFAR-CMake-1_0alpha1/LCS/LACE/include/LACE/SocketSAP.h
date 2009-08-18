//#  SocketSAP.h: Abstract base class for all kind of different sockets.
//#
//#  Copyright (C) 2008
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

#ifndef LOFAR_LACE_SOCKETSAP_H
#define LOFAR_LACE_SOCKETSAP_H

// \file SocketSAP.h
// Abstract base class for all kind of different sockets.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <LACE/InetAddress.h>
#include <LACE/ServiceAccessPoint.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace LACE {

// \addtogroup LACE
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// class_description
// ...
class SocketSAP : public ServiceAccessPoint
{
public:		// only for testing
//protected:
	SocketSAP();
	virtual ~SocketSAP();

	void close ();
	int  doOpen  (const Address&	anAddress, bool reuseAddres = true);
	bool isOpen() { return (itsAddress.getType() != UNDEFINED); }
	
	int  setOption(int	option, const void*	value, socklen_t	valueLen) const;
	int  getOption(int	option, void*		value, socklen_t*	valueLen) const;
	
	int	 read (void*		buffer, size_t	nrBytes);
	int	 write(const void*	buffer, size_t	nrBytes);

	friend class SocketAcceptor;

	enum {
		SK_OK			= 0,
		SK_SYS_ERROR	= -1,
		SK_ALLOC		= -2,
		SK_NOT_OPENED	= -3,
		SK_PEER_CLOSED	= -4,
		SK_BIND			= -5,
		SK_LISTEN		= -6,
		SK_TIMEOUT		= -7,
//		SK_CONNECT,
//		SK_ACCEPT,
//		SK_INCOMPLETE,
	};
	
private:
	//# --- Datamembers ---
	InetAddress		itsAddress;		// installed after succesful 'open'
};


//# ----- inline functions -----
inline int	SocketSAP::setOption(int	option, const void*	value, socklen_t	valueLen) const
{
	return (setsockopt(getHandle(), SOL_SOCKET, option, (char *) value, valueLen));
}

inline int	SocketSAP::getOption(int	option, void*	value, socklen_t*	valueLen) const
{
	return (getsockopt(getHandle(), SOL_SOCKET, option, (char *) value, valueLen));
}


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
