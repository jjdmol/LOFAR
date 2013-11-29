//# SocketConnector.h: Class that make connects a SAP to an addres.
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

#ifndef LOFAR_LACE_SOCKETCONNECTOR_H
#define LOFAR_LACE_SOCKETCONNECTOR_H

// \file SocketConnector.h
// Class that make connects a SAP to an addres.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <LACE/SocketStream.h>
#include <LACE/Address.h>

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
class SocketConnector
{
public:
	SocketConnector();
	~SocketConnector();
	
	int		connect (SocketStream&			aStream,
					 const Address&			anAddress,
					 int					waitMs = -1);

protected:

private:
	//# --- Datamembers ---
};


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
