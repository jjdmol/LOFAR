//#  SocketConnector.h: Class that make connects a SAP to an addres.
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
