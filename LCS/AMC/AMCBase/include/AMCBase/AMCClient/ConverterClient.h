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

// \file
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

      virtual Direction j2000ToAzel(const Direction& dir, 
                                   const Position& pos, 
                                   const Epoch& time);

      virtual vector<Direction> j2000ToAzel (const vector<Direction>& dir,
                                            const Position& pos,
                                            const Epoch& time);

      virtual vector<Direction> j2000ToAzel (const Direction& dir,
                                            const vector<Position>& pos,
                                            const Epoch& time);

      virtual vector<Direction> j2000ToAzel (const Direction& dir,
                                            const Position& pos,
                                            const vector<Epoch>& time);

      virtual vector<Direction> j2000ToAzel (const vector<Direction>& dir,
                                            const vector<Position>& pos,
                                            const vector<Epoch>& time);

      virtual Direction azelToJ2000 (const Direction& dir,
                                    const Position& pos,
                                    const Epoch& time);

      virtual vector<Direction> azelToJ2000 (const vector<Direction>& dir,
                                            const Position& pos,
                                            const Epoch& time);

      virtual vector<Direction> azelToJ2000 (const vector<Direction>& dir,
                                            const vector<Position>& pos,
                                            const vector<Epoch>& time);

      virtual Direction j2000ToItrf(const Direction& dir, 
                                   const Position& pos, 
                                   const Epoch& time);

      virtual vector<Direction> j2000ToItrf (const vector<Direction>& dir,
                                            const Position& pos,
                                            const Epoch& time);

      virtual vector<Direction> j2000ToItrf (const Direction& dir,
                                            const vector<Position>& pos,
                                            const Epoch& time);

      virtual vector<Direction> j2000ToItrf (const Direction& dir,
                                            const Position& pos,
                                            const vector<Epoch>& time);

      virtual vector<Direction> j2000ToItrf (const vector<Direction>& dir,
                                            const vector<Position>& pos,
                                            const vector<Epoch>& time);

      virtual Direction itrfToJ2000 (const Direction& dir,
                                    const Position& pos,
                                    const Epoch& time);

      virtual vector<Direction> itrfToJ2000 (const vector<Direction>& dir,
                                            const Position& pos,
                                            const Epoch& time);

      virtual vector<Direction> itrfToJ2000 (const vector<Direction>& dir,
                                            const vector<Position>& pos,
                                            const vector<Epoch>& time);


    private:
      //@{
      // Make this class non-copyable.
      ConverterClient(const ConverterClient&);
      ConverterClient& operator=(const ConverterClient&);
      //@}

      // Perform actual conversion by first sending a conversion request to
      // the server and then receiving the conversion result.
      vector<Direction> doConvert(const ConverterCommand&,
                                 const vector<Direction>&,
                                 const vector<Position>&,
                                 const vector<Epoch>&);
      
      // Send a conversion command to the server. The input data are stored in
      // three vectors.
      // \return \c true if send was successful; otherwise \c false.
      bool sendRequest(const ConverterCommand&,
                       const vector<Direction>&,
                       const vector<Position>&,
                       const vector<Epoch>&);
      
      // Receive the converted data from the server.
      // \return \c true if receive was successful; otherwise \c false.
      // \note This method is blocking.
      bool recvResult(vector<Direction>&);
        
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
