//#  DH_AC_Connect.h: Implements the connection protocol with the ACmaster
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
//#	 Abstract:
//#	 TODO
//#
//#  $Id$

#ifndef ACC_DH_AC_CONNECT_H
#define ACC_DH_AC_CONNECT_H

#include <lofar_config.h>

//# Includes
#include <sys/time.h>
#if defined (__APPLE__)
# include <sys/param.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <rpc/types.h>
#include <Transport/DataHolder.h>

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;


//# Description of class.
// The ApplControl class implements the C/S communication of the service the
// the Application Controller supports.
//
class DH_AC_Connect : public DataHolder
{
public:
	// Constructor
	explicit DH_AC_Connect(const string&	hostID);

	// Destructor
	virtual ~DH_AC_Connect();

	DH_AC_Connect*		clone() const;
	// Redefines the preprocess function.
	virtual void 	preprocess();

	// The real data-accessor functions
	void	setHostname	 (const string&			theHostname);
	void	setServerIP	 (const in_addr_t&		theServerIP);
	void	setServerPort(const in_port_t&		theServerPort);

	string		getHostname	  () const;
	in_addr_t	getServerIP	  () const;
	string		getServerIPStr() const;
	in_port_t	getServerPort () const;

private:
	// forbit default construction and copying
	DH_AC_Connect();
	DH_AC_Connect(const DH_AC_Connect& that);
	DH_AC_Connect& 	operator=(const DH_AC_Connect& that);

	// Implement the initialisation of the pointers
	virtual void	fillDataPointers();

	// fields transferred between the server and the client
	uint16		*itsVersionNumber;
	char		*itsHostname;
	uint32		*itsServerIP;		// in_addr_t
	uint16		*itsServerPort;		// in_port_t

};

// The real data-accessor functions
inline void	DH_AC_Connect::setHostname	(const string&		theHostname)
{
	strncpy(itsHostname, theHostname.c_str(), MAXHOSTNAMELEN-1);
	itsHostname[MAXHOSTNAMELEN - 1] = '\0';
}

inline void	DH_AC_Connect::setServerIP	(const in_addr_t&		theServerIP)
{
	*itsServerIP = theServerIP;
}

inline void	DH_AC_Connect::setServerPort(const in_port_t&		theServerPort)
{
	*itsServerPort = theServerPort;
}

inline string	DH_AC_Connect::getHostname	() const
{
	// no version support necc. yet.
	return (string(itsHostname));
}

inline in_addr_t	DH_AC_Connect::getServerIP	() const
{
	// no version support necc. yet.
	return (*itsServerIP);
}

inline string	DH_AC_Connect::getServerIPStr	() const
{
	// no version support necc. yet.
	uint32	IPBytes = *itsServerIP;
	char	IPstr [16];
	sprintf (IPstr, "%d.%d.%d.%d", IPBytes >> 24, (IPBytes >> 16) & 0xFF,
									(IPBytes >> 8) & 0xFF, IPBytes & 0xFF);
	return (IPstr);
}

inline in_port_t	DH_AC_Connect::getServerPort() const
{
	// no version support necc. yet.
	return (*itsServerPort);
}


} // namespace ACC
} // namespace LOFAR

#endif
