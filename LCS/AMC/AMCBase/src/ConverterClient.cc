//# ConverterClient.cc: one line description
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
#include <AMCBase/ConverterClient.h>
#include <AMCBase/ConverterCommand.h>
#include <AMCBase/ConverterStatus.h>
#include <AMCBase/Exceptions.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <Common/StringUtil.h>


namespace LOFAR
{
  namespace AMC
  {

    ConverterClient::ConverterClient(const string& server, uint16 port) :
      itsTH(server, toString(port)),
      itsSendConn("send", &itsRequest, 0, &itsTH),
      itsRecvConn("recv", 0, &itsResult, &itsTH)
    {
      if (!itsTH.init()) {
        THROW (IOException, 
               "Failed to connect to " << server << " at port " << port);
      }
    }


    ConverterClient::~ConverterClient()
    {
    }


    void 
    ConverterClient::j2000ToAzel(ResultData& result,
                                 const RequestData& request)
    {
      doConvert(result, request, ConverterCommand::J2000toAZEL);
    }
    
    
    void
    ConverterClient::azelToJ2000(ResultData& result,
                                 const RequestData& request)
    {
      doConvert(result, request, ConverterCommand::AZELtoJ2000);
    }
    
    
    void 
    ConverterClient::j2000ToItrf(ResultData& result,
                                 const RequestData& request)
    {
      doConvert(result, request, ConverterCommand::J2000toITRF);
    }
    
    
    void 
    ConverterClient::itrfToJ2000(ResultData& result,
                                 const RequestData& request)
    {
      doConvert(result, request, ConverterCommand::ITRFtoJ2000);
    }
    


    //####################  Private methods  ####################//

    void
    ConverterClient::doConvert(ResultData& result,
                               const RequestData& request,
                               const ConverterCommand& command)
    {
      ConverterStatus status;

      // Send the request to the server, receive the result from the server.
      // If successful, check the return status: throw on error.
      if (sendRequest(command, request) && recvResult(status, result)) {
        if (!status) THROW (ConverterException, status.text());
        else return;
      }

      // If the server has died, we will usually notice this only after the
      // receive fails. So, we will give it another try.
      if (sendRequest(command, request) && recvResult(status, result)) {
        if (!status) THROW (ConverterException, status.text());
        else return;
      }

      // When we get here, both tries failed. Trouble!
      THROW (IOException, "Server communication failure. Bailing out!");
    }


    bool ConverterClient::sendRequest(const ConverterCommand& cmd,
                                      const RequestData& req)
    {
      // Write the conversion request into the data holder's I/O buffer.
     itsRequest.writeBuf(cmd, req);

      // Write the request from the data holder's I/O buffer to the server.
      // If the write fails, we may have lost the connection to the server.
      if (itsSendConn.write() == Connection::Error) {
        LOG_DEBUG("ConverterClient::sendRequest() - "
                  "Connection error. Trying to reconnect ...");
        // Try to reconnect.
        if (!itsTH.init()) {
          THROW (IOException, "Failed to reconnect to server");
        }
        return false;
      }

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsSendConn.waitForWrite();

      // When we get here, everything went well.
      return true;
    }


    bool ConverterClient::recvResult(ConverterStatus& sts, ResultData& res)
    {
      // Read the result from the server into the data holder's I/O buffer.
      // If the read fails, we may have lost the connection to the server.
      if (itsRecvConn.read() == Connection::Error) {
        LOG_DEBUG("ConverterClient::recvResult() - "
                  "Connection error. Trying to reconnect ...");
        // Try to reconnect.
        if (!itsTH.init()) {
          THROW (IOException, "Failed to reconnect to server");
        }
        return false;
      }

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsRecvConn.waitForRead();

      // Read the conversion result from the I/O buffer into direction
      itsResult.readBuf(sts, res);

      // When we get here, everything went well.
      return true;
    }

  } // namespace AMC

} // namespace LOFAR
