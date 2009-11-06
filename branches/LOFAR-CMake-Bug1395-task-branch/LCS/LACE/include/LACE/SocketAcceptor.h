//# SocketAcceptor.h: Class that make connects a SAP to an addres.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_LACE_SOCKETACCEPTOR_H
#define LOFAR_LACE_SOCKETACCEPTOR_H

// \file SocketAcceptor.h
// Class that make connects a SAP to an addres.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <LACE/SocketStream.h>
#include <LACE/InetAddress.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace LACE {

// \addtogroup LACE
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//#class ...;



// class_description
// ...
class SocketAcceptor : public SocketSAP
{
public:
	SocketAcceptor();
	~SocketAcceptor();
	
	int		open	(const Address&			anAddress, int backlog = 5);
	void	close	() { return (SocketSAP::close()); }

	// -1 : blocking
	// >= 0 : async, wait max n milliseconds
	int		accept	(SocketStream&			aStream,
					 int					waitMs = -1);

protected:

private:
	//# --- Datamembers ---
//	SocketSAP		itsListener;
};


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
