//#  ConverterProcess.cc: implementation of the Converter process.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCImpl/AMCServer/ConverterProcess.h>
#include <AMCImpl/Exceptions.h>
#include <AMCBase/AMCClient/ConverterCommand.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>

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
          
        // ConverterImpl may throw a ConverterError; we don't want to let this
        // exception escape.
        try {
          
          ConverterCommand cmd;
          vector<SkyCoord> skyCoord;
          vector<EarthCoord> earthCoord;
          vector<TimeCoord> timeCoord;
          
          // Receive conversion request. If the receive fails, the client
          // probably hung up.
          if (!recvRequest(cmd, skyCoord, earthCoord, timeCoord)) break;
          
          // Process the conversion request, invoking the right conversion
          // method.
          switch(cmd.get()) {
          case ConverterCommand::J2000toAZEL:
            skyCoord = 
              itsConverter.j2000ToAzel(skyCoord, earthCoord, timeCoord);
            break;
          case ConverterCommand::J2000toITRF:
            skyCoord = 
              itsConverter.j2000ToItrf(skyCoord, earthCoord, timeCoord);
            break;
          case ConverterCommand::AZELtoJ2000:
            skyCoord = 
              itsConverter.azelToJ2000(skyCoord, earthCoord, timeCoord);
            break;
          default:
            LOG_DEBUG_STR("ConverterProcess::handleRequests() - "
                          << "Received invalid converter command (" 
                          << cmd << ")");
          }
          
          // Send the conversion result to the client. If the send fails, the
          // client probably hung up.
          if (!sendResult(skyCoord)) break;

        } 
        catch (ConverterError& e) {
          LOG_DEBUG_STR(e);
        }   
      }
    }
    

    bool ConverterProcess::recvRequest(ConverterCommand& cmd,
                                       vector<SkyCoord>& skyCoord,
                                       vector<EarthCoord>& earthCoord,
                                       vector<TimeCoord>& timeCoord)
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
      // skyCoord, \a earthCoord, and \a timeCoord.
      itsRequest.readBuf(cmd, skyCoord, earthCoord, timeCoord);

      // Everything went well.
      return true;
    }


    bool ConverterProcess::sendResult(const vector<SkyCoord>& skyCoord)
    {
      // Write the conversion result into the data holder's I/O buffer.
      itsResult.writeBuf(skyCoord);

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

