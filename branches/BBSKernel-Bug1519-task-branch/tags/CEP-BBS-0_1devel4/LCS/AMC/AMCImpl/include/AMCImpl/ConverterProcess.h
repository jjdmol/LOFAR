//#  ConverterProcess.h: process client conversion requests.
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

#ifndef LOFAR_AMCIMPL_CONVERTERPROCESS_H
#define LOFAR_AMCIMPL_CONVERTERPROCESS_H

// \file
// Process client conversion requests

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <AMCImpl/ConverterImpl.h>
#include <AMCBase/DH_Request.h>
#include <AMCBase/DH_Result.h>
#include <Transport/Connection.h>
#include <Transport/TH_Socket.h>
#include <Common/Process.h>

namespace LOFAR
{
  //# Forward declarations.
  class ConverterCommand;
  class Direction;
  class Position;
  class Epoch;

  namespace AMC
  {

    // \addtogroup AMCImpl
    // @{

    // This class processes the conversion requests it receives from a
    // ConverterClient. Whenever the ConverterServer accepts an incoming
    // client connection, it creates a new ConverterProcess object and spawns
    // a new process. In the new process the handleRequests() method of the
    // newly create ConverterProcess object is called; this method will handle
    // all client requests until the client disconnects.
    class ConverterProcess : public Process
    {
    public:
      // Constructor. \a aSocket must be a data socket 
      explicit ConverterProcess(Socket* aSocket);

      // Destructor.
      virtual ~ConverterProcess();

      // Handle all incoming conversion requests until the clients disconnects.
      void handleRequests();

    private:
      // Receive the conversion request from the client.
      bool recvRequest(ConverterCommand&, RequestData&);

      // Send the conversion result to the client.
      bool sendResult(const ConverterStatus&, const ResultData&);
        
      // Data holder holding the request data to be sent to the server.
      DH_Request itsRequest;

      // Data holder holding the result data to be received from the server.
      DH_Result itsResult;

      // The transport holder (which is, of course, a TH_Socket)
      TH_Socket itsTH;

      // Connection for sending data to the server.
      Connection itsSendConn;

      // Connection for receiving data from the server.
      Connection itsRecvConn;

      // Implementation of the Converter interface.
      ConverterImpl itsConverter;

    };

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
