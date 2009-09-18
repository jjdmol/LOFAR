//# ConverterServer.cc: implementation of the Converter server.
//#
//# Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCImpl/ConverterServer.h>
#include <AMCImpl/ConverterProcess.h>
#include <AMCBase/Exceptions.h>
#include <Common/StringUtil.h>
#include <cstdlib>

namespace LOFAR
{
  namespace AMC
  {

    ConverterServer::ConverterServer(uint16 port) :
      itsListenSocket("server", toString(port), Socket::TCP)
    {
      if (!itsListenSocket.ok()) {
        THROW (IOException,
               formatString("Failed to create listen socket on port %d - %s",
                            port, itsListenSocket.errstr().c_str()));
      }
    }


    ConverterServer::~ConverterServer()
    {
    }


    void ConverterServer::run()
    {
      while(true) {
        handleConnections();
      }
    }


    void ConverterServer::handleConnections()
    {
      // Listen for incoming client connections.
      Socket* dataSocket = itsListenSocket.accept();

      if (!dataSocket || !dataSocket->ok()) {
        LOG_ERROR(formatString("Accept failed: %s", 
                               itsListenSocket.errstr().c_str()));
        return;
      }

      {
	// The converter process will handle all client conversion requests.
	// Note: we must create the ConverterProcess object within a new
	// scope, otherwise proc will outlive dataSocket!
	ConverterProcess proc(dataSocket);
      
	// Let the converter process run as a separate process. We don't
	// want to fetch the child's exit status, so add \c true to avoid
	// zombies.
	proc.spawn(true);
      
	// If we are the child process ...
	if (proc.isChild()) {

	  LOG_TRACE_FLOW("This is the child process");
        
	  // we should close the listen socket ...
	  LOG_TRACE_STAT("Closing listen socket ...");
	  itsListenSocket.close();
        
	  // and handle all client requests, until client disconnects.
	  LOG_TRACE_STAT("Start handling client requests ...");
	  proc.handleRequests();
        
	  // The child should NEVER return, but ALWAYS exit.
          exit(0);
	}
      }

      // If we get here, we're the parent process; we should delete the
      // dataSocket object.
      delete dataSocket;

    }

  } // namespacd AMC

} // namespace LOFAR
