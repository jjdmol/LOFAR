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
    }


    ConverterProcess::~ConverterProcess()
    {
    }


    void ConverterProcess::handleRequests()
    {
      // While the client is connected, handle incoming requests.
      while(itsRecvConn.isConnected()) {

        ConverterCommand cmd;
        vector<SkyCoord> skyCoord;
        vector<EarthCoord> earthCoord;
        vector<TimeCoord> timeCoord;

        // receive conversion request.
        recvRequest(cmd, skyCoord, earthCoord, timeCoord);

        // process the conversion request, invoking the right conversion
        // method.
        switch(cmd.get()) {
        case ConverterCommand::J2000toAZEL:
          skyCoord = itsConverter.j2000ToAzel(skyCoord, earthCoord, timeCoord);
          break;
        case ConverterCommand::AZELtoJ2000:
          skyCoord = itsConverter.azelToJ2000(skyCoord, earthCoord, timeCoord);
          break;
        default:
          THROW (ConverterError, 
                 "Received invalid converter command (" << cmd << ")");
        }
        
        // send the conversion result to the client.
        sendResult(skyCoord);
      }

    }


    void ConverterProcess::recvRequest(ConverterCommand& cmd,
                                      vector<SkyCoord>& skyCoord,
                                      vector<EarthCoord>& earthCoord,
                                      vector<TimeCoord>& timeCoord)
    {
      // Read the result from the client into the data holder's I/O buffer.
      itsRecvConn.read();

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsRecvConn.waitForRead();

      // Read the conversion request from the I/O buffer into \a cmd, \a
      // skyCoord, \a earthCoord, and \a timeCoord.
      itsRequest.readBuf(cmd, skyCoord, earthCoord, timeCoord);

    }


    void ConverterProcess::sendResult(const vector<SkyCoord>& skyCoord)
    {
      // Write the conversion result into the data holder's I/O buffer.
      itsResult.writeBuf(skyCoord);

      // Write the result from the data holder's I/O buffer to the client.
      itsSendConn.write();

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsSendConn.waitForWrite();

    }


  } // namespacd AMC

} // namespace LOFAR

