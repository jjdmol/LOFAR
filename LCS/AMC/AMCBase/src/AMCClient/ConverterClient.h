//#  ConverterClient.h: Client side of the AMC client/server implementation.
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

#ifndef LOFAR_AMCBASE_AMCCLIENT_CONVERTERCLIENT_H
#define LOFAR_AMCBASE_AMCCLIENT_CONVERTERCLIENT_H

// \file ConverterClient.h
// Client side of the AMC client/server implementation

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <AMCBase/Converter.h>
#include <AMCBase/AMCClient/DH_Request.h>
#include <AMCBase/AMCClient/DH_Result.h>
#include <Transport/Connection.h>
#include <Transport/TH_Socket.h>

namespace LOFAR
{
  namespace AMC
  {
    // \addtogroup AMCClient
    // @{

    // This class represents the client side of the client/server
    // implementation of the AMC. It implements the Converter interface. 
    //
    // The client acts as a proxy. It sends a conversion request to the
    // server; the server performs the actual conversion and returns the
    // result to the client.
    class ConverterClient : public Converter
    {
    public:
      ConverterClient(const string& server = "localhost", 
                      uint16 port = 31337);

      virtual ~ConverterClient();

      virtual SkyCoord j2000ToAzel(const SkyCoord& radec, 
                                   const EarthCoord& pos, 
                                   const TimeCoord& time);

      virtual vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& radec,
                                            const EarthCoord& pos,
                                            const TimeCoord& time);

      virtual vector<SkyCoord> j2000ToAzel (const SkyCoord& radec,
                                            const vector<EarthCoord>& pos,
                                            const TimeCoord& time);

      virtual vector<SkyCoord> j2000ToAzel (const SkyCoord& radec,
                                            const EarthCoord& pos,
                                            const vector<TimeCoord>& time);

      virtual vector<SkyCoord> j2000ToAzel (const vector<SkyCoord>& radec,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time);

      virtual SkyCoord azelToJ2000 (const SkyCoord& azel,
                                    const EarthCoord& pos,
                                    const TimeCoord& time);

      virtual vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& azel,
                                            const EarthCoord& pos,
                                            const TimeCoord& time);

      virtual vector<SkyCoord> azelToJ2000 (const vector<SkyCoord>& azel,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time);

      virtual SkyCoord j2000ToItrf(const SkyCoord& radec, 
                                   const EarthCoord& pos, 
                                   const TimeCoord& time);
 
      virtual vector<SkyCoord> j2000ToItrf (const vector<SkyCoord>& radec,
                                            const EarthCoord& pos,
                                            const TimeCoord& time);
 
      virtual vector<SkyCoord> j2000ToItrf (const SkyCoord& radec,
                                            const vector<EarthCoord>& pos,
                                            const TimeCoord& time);
 
      virtual vector<SkyCoord> j2000ToItrf (const SkyCoord& radec,
                                            const EarthCoord& pos,
                                            const vector<TimeCoord>& time);
 
      virtual vector<SkyCoord> j2000ToItrf (const vector<SkyCoord>& radec,
                                            const vector<EarthCoord>& pos,
                                            const vector<TimeCoord>& time);
 

    private:
      //@{
      // Make this class non-copyable.
      ConverterClient(const ConverterClient&);
      ConverterClient& operator=(const ConverterClient&);
      //@}

      // Perform actual conversion by first sending a conversion request to
      // the server and then receiving the conversion result.
      vector<SkyCoord> doConvert(const ConverterCommand&,
                                 const vector<SkyCoord>&,
                                 const vector<EarthCoord>&,
                                 const vector<TimeCoord>&);
      
      // Send a conversion command to the server. The input data are stored in
      // three vectors.
      // \return \c true if send was successful; otherwise \c false.
      bool sendRequest(const ConverterCommand&,
                       const vector<SkyCoord>&,
                       const vector<EarthCoord>&,
                       const vector<TimeCoord>&);
      
      // Receive the converted data from the server.
      // \return \c true if receive was successful; otherwise \c false.
      // \note This method is blocking.
      bool recvResult(vector<SkyCoord>&);
        
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

    };

    // @}

  } // namespace AMC
  
} // namespace LOFAR

#endif
