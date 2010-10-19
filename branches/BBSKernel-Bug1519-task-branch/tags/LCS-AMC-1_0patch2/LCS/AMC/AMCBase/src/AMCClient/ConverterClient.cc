//#  ConverterClient.cc: one line description
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
#include <AMCBase/AMCClient/ConverterClient.h>
#include <AMCBase/AMCClient/ConverterCommand.h>
#include <AMCBase/Exceptions.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
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
        THROW (ClientError, 
               "Failed to connect to " << server << " at port " << port);
      }
    }


    ConverterClient::~ConverterClient()
    {
    }


    SkyCoord 
    ConverterClient::j2000ToAzel (const SkyCoord& skyCoord, 
                                  const EarthCoord& earthCoord, 
                                  const TimeCoord& timeCoord)
    {
      return j2000ToAzel(vector<SkyCoord>  (1, skyCoord),
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord)) [0];
    }


    vector<SkyCoord> 
    ConverterClient::j2000ToAzel (const vector<SkyCoord>& skyCoord,
                                  const EarthCoord& earthCoord,
                                  const TimeCoord& timeCoord)
    {
      return j2000ToAzel(skyCoord, 
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord));
    }


    vector<SkyCoord> 
    ConverterClient::j2000ToAzel (const SkyCoord& skyCoord,
                                  const vector<EarthCoord>& earthCoord,
                                  const TimeCoord& timeCoord)
    {
      return j2000ToAzel(vector<SkyCoord> (1, skyCoord),
                         earthCoord, 
                         vector<TimeCoord>(1, timeCoord));
    }


    vector<SkyCoord> 
    ConverterClient::j2000ToAzel (const SkyCoord& skyCoord,
                                  const EarthCoord& earthCoord,
                                  const vector<TimeCoord>& timeCoord)
    {
      return j2000ToAzel(vector<SkyCoord>  (1, skyCoord),
                         vector<EarthCoord>(1, earthCoord),
                         timeCoord);
    }


    vector<SkyCoord> 
    ConverterClient::j2000ToAzel (const vector<SkyCoord>& skyCoord,
                                  const vector<EarthCoord>& earthCoord,
                                  const vector<TimeCoord>& timeCoord)
    {
      // Set the value of ConverterCommand equal to J2000->AZEL
      ConverterCommand cmd(ConverterCommand::J2000toAZEL);
                                
      // Send the request to the server
      sendRequest(cmd, skyCoord, earthCoord, timeCoord);

      // Vectors to hold the conversion result.
      vector<SkyCoord> sc;

      // Receive the result from the server.
      // \note This method is blocking.
      recvResult(sc);

      // Return the result of the conversion.
      return sc;
    }


    SkyCoord 
    ConverterClient::azelToJ2000 (const SkyCoord& skyCoord,
                                  const EarthCoord& earthCoord,
                                  const TimeCoord& timeCoord)
    {
      return azelToJ2000(vector<SkyCoord>  (1, skyCoord),
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord)) [0];
    }


    vector<SkyCoord>
    ConverterClient::azelToJ2000 (const vector<SkyCoord>& skyCoord,
                                  const EarthCoord& earthCoord,
                                  const TimeCoord& timeCoord)
    {
      return azelToJ2000(skyCoord, 
                         vector<EarthCoord>(1, earthCoord),
                         vector<TimeCoord> (1, timeCoord));
    }


    vector<SkyCoord>
    ConverterClient::azelToJ2000 (const vector<SkyCoord>& skyCoord,
                                  const vector<EarthCoord>& earthCoord,
                                  const vector<TimeCoord>& timeCoord)
    {
      // Set the value of ConverterCommand equal to AZEL->J2000
      ConverterCommand cmd(ConverterCommand::AZELtoJ2000);
                                
      // Send the request to the server
      sendRequest(cmd, skyCoord, earthCoord, timeCoord);

      // Vectors to hold the conversion result.
      vector<SkyCoord> sc;

      // Receive the result from the server.
      // \note This method is blocking.
      recvResult(sc);

      // Return the result of the conversion.
      return sc;
    }


    void ConverterClient::sendRequest(const ConverterCommand& cmd,
                                      const vector<SkyCoord>& skyCoord,
                                      const vector<EarthCoord>& earthCoord,
                                      const vector<TimeCoord>& timeCoord)
    {
      // Write the conversion request into the data holder's I/O buffer.
      itsRequest.writeBuf(cmd, skyCoord, earthCoord, timeCoord);

      // Write the request from the data holder's I/O buffer to the server.
      if (itsSendConn.write() == Connection::Error) {
        THROW (ClientError,
               "Error sending data to server. Connection lost?");
      }

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsSendConn.waitForWrite();

    }

    void ConverterClient::recvResult(vector<SkyCoord>& skyCoord)
    {
      // Read the result from the server into the data holder's I/O buffer.
      if (itsRecvConn.read() == Connection::Error) {
        THROW (ClientError, 
               "Error receiving data from server. Connection lost?");
      }

      // Always make this call, even though it only has effect when doing
      // asynchronous communication.
      itsRecvConn.waitForRead();

      // Read the conversion result from the I/O buffer into skyCoord
      itsResult.readBuf(skyCoord);
    }


  } // namespace AMC

} // namespace LOFAR
