//# ConverterProcess.cc: implementation of the Converter process.
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
#include <AMCImpl/ConverterProcess.h>
#include <AMCBase/Exceptions.h>
#include <AMCBase/ConverterCommand.h>
#include <AMCBase/ConverterStatus.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>

namespace LOFAR
{
  namespace AMC
  {

    ConverterProcess::ConverterProcess(Socket* aSocket) :
      itsTH(aSocket),
      itsSendConn("send", &itsResult, 0, &itsTH),
      itsRecvConn("recv", 0, &itsRequest, &itsTH)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    ConverterProcess::~ConverterProcess()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    void ConverterProcess::handleRequests()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // While the client is connected, handle incoming requests.
      while(itsRecvConn.isConnected()) {

        ConverterCommand command;
        ConverterStatus status;
        RequestData request;
        ResultData result;

        // Receive conversion request. If the receive fails, the client
        // probably hung up.
        if (!recvRequest(command, request)) break;
          
        // ConverterImpl may throw a ConverterException; we don't want to let
        // this exception escape.
        try {
          
          // Process the conversion request, invoking the right conversion
          // method.
          switch(command.get()) {
          case ConverterCommand::J2000toAZEL:
            itsConverter.j2000ToAzel(result, request);
            break;
          case ConverterCommand::J2000toITRF:
            itsConverter.j2000ToItrf(result, request);
            break;
          case ConverterCommand::AZELtoJ2000:
            itsConverter.azelToJ2000(result, request);
            break;
          case ConverterCommand::ITRFtoJ2000:
            itsConverter.itrfToJ2000(result, request);
            break;
          default:
            LOG_DEBUG_STR("ConverterProcess::handleRequests() - "
                          << "Received invalid converter command (" 
                          << command << ")");
          } // switch

        } 
        catch (LOFAR::Exception& e) {
          LOG_DEBUG_STR(e);
          status = ConverterStatus(ConverterStatus::ERROR, 
                                   string("AMCServer error: ") + e.what());
        }   
        
        // Send the conversion result to the client. If the send fails, the
        // client probably hung up.
        if (!sendResult(status, result)) break;

      } // while

    }


    bool ConverterProcess::recvRequest(ConverterCommand& command,
                                       RequestData& request)
    {
      // Read the result from the client into the data holder's I/O buffer.
      // If the read fails, the client probably hung up.
      if (itsRecvConn.read() == Connection::Error) {
        LOG_DEBUG("ConverterProcess::recvRequest() - "
                  "Connection error. Client probably hung up.");
        return false;
      }

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsRecvConn.waitForRead();

      // Read the conversion request from the I/O buffer into \a cmd, \a
      // direction, \a position, and \a epoch.
      itsRequest.readBuf(command, request);

      // Everything went well.
      return true;
    }


    bool ConverterProcess::sendResult(const ConverterStatus& status,
                                      const ResultData& result)
    {
      // Write the conversion result into the data holder's I/O buffer.
      itsResult.writeBuf(status, result);

      // Write the result from the data holder's I/O buffer to the client.
      if (itsSendConn.write() == Connection::Error) {
        LOG_DEBUG("ConverterProcess::sendResult() - "
                  "Connection error. Client probably hung up.");
        return false;
      }

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsSendConn.waitForWrite();

      // Everything went well.
      return true;
    }


  } // namespacd AMC

} // namespace LOFAR

