//#  GTM_TCPSocket.h: base class for all sockets
//#
//#  Copyright (C) 2002-2003
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

#ifndef GTM_TCPSOCKET_H
#define GTM_TCPSOCKET_H

#include "GTM_File.h"
#include <Common/lofar_string.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

// forward declaration
class GCFTCPPort;

/**
 * This class consists of the basic implementation of a TCP socket. Beside that it 
 * is the base class for the GTMTCPServerSocket class.
 */

class GTMTCPSocket : public GTMFile
{
public: 
	// constructors, destructors and default operators
    GTMTCPSocket (GCFTCPPort& port);
    virtual ~GTMTCPSocket ();
  
	// GTMTCPSocket specific member methods
    // open/close methods
    virtual bool open (unsigned int portNumber);
    virtual bool connect (unsigned int portNumber, const string& host);
  
    // send/recv methods
    virtual ssize_t send (void* buf, size_t count);
    virtual ssize_t recv (void* buf, size_t count, bool raw = false);

private:
    GTMTCPSocket ();
    /// Don't allow copying of the GTMTCPSocket object.
    GTMTCPSocket (const GTMTCPSocket&);
    GTMTCPSocket& operator= (const GTMTCPSocket&);

};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
