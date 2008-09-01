//# SocketListener.h: Class that creates a socket and accepts connections
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_MWCOMMON_SOCKETLISTENER_H
#define LOFAR_MWCOMMON_SOCKETLISTENER_H

// @file
// @brief Class that creates a socket and accepts connections.
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/SocketConnection.h>
#include <Common/Net/Socket.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Class that creates a socket and accepts connections.

  // This class sets up a socket listener. It is used by SocketConnectionSet
  // to accept connection requests from workers.
  //
  // Internally the class uses a shared pointer to a socket object.
  // It means that a copy of a SocketListener object can be made, but that
  // copies share the same underlying socket object.

  class SocketListener
  {
  public:
    // Set up the server side of a listener.
    explicit SocketListener (const std::string& port);

    // Listen to a connection and accept it.
    // It blocks until another process wants to connect.
    SocketConnection::ShPtr accept();

  private:
    boost::shared_ptr<LOFAR::Socket> itsConnSocket;
  };

}} //# end namespaces

#endif
