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
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>

namespace LOFAR
{
  namespace AMC
  {

    
    ConverterClient::ConverterClient(const string& server, ushort port)
    {
    }


    SkyCoord 
    ConverterClient::j2000ToAzel (const SkyCoord& radec, 
                                  const EarthCoord& pos, 
                                  const TimeCoord& time)
    {
      vector<SkyCoord> sc;
      vector<EarthCoord> ec;
      vector<TimeCoord> tc;

      sc.push_back(radec);
      ec.push_back(pos);
      tc.push_back(time);

      return j2000ToAzel(sc, ec, tc)[0];
    }


    vector<SkyCoord> 
    ConverterClient::j2000ToAzel (const vector<SkyCoord>& radec,
                                  const EarthCoord& pos,
                                  const TimeCoord& time)
    {
      vector<EarthCoord> ec;
      vector<TimeCoord> tc;
      
      ec.push_back(pos);
      tc.push_back(time);
      
      return j2000ToAzel(radec, ec, tc);
    }


    vector<SkyCoord> 
    ConverterClient::j2000ToAzel (const SkyCoord& radec,
                                  const vector<EarthCoord>& pos,
                                  const TimeCoord& time)
    {
      vector<SkyCoord> sc;
      vector<TimeCoord> tc;

      sc.push_back(radec);
      tc.push_back(time);
      
      return j2000ToAzel(sc, pos, tc);
    }


    vector<SkyCoord> 
    ConverterClient::j2000ToAzel (const SkyCoord& radec,
                                  const EarthCoord& pos,
                                  const vector<TimeCoord>& time)
    {
      vector<SkyCoord> sc;
      vector<EarthCoord> ec;

      sc.push_back(radec);
      ec.push_back(pos);

      return j2000ToAzel(sc, ec, time);
    }


    vector<SkyCoord> 
    ConverterClient::j2000ToAzel (const vector<SkyCoord>& radec,
                                  const vector<EarthCoord>& pos,
                                  const vector<TimeCoord>& time)
    {
      // Set the value of ConverterOperation equal to J2000->AZEL
      ConverterOperation op; /* (to be done) */
                                
      // Send the request to the server
      itsRequest.send(op, radec, pos, time);

      // Vectors to hold the conversion result.
      vector<SkyCoord> sc;
      vector<EarthCoord> ec; // dummy
      vector<TimeCoord> tc; // dummy

      // Receive the result from the server.
      // \note This method is blocking.
      itsResult.receive(sc, ec, tc);

      // Return the result of the conversion.
      return sc;
    }


    SkyCoord 
    ConverterClient::azelToJ2000 (const SkyCoord& azel,
                                  const EarthCoord& pos,
                                  const TimeCoord& time)
    {
      vector<SkyCoord> sc;
      vector<EarthCoord> ec;
      vector<TimeCoord> tc;

      sc.push_back(azel);
      ec.push_back(pos);
      tc.push_back(time);

      return azelToJ2000(sc, ec, tc)[0];
    }


    vector<SkyCoord>
    ConverterClient::azelToJ2000 (const vector<SkyCoord>& azel,
                                  const EarthCoord& pos,
                                  const TimeCoord& time)
    {
      vector<EarthCoord> ec;
      vector<TimeCoord> tc;

      ec.push_back(pos);
      tc.push_back(time);

      return azelToJ2000(azel, ec, tc);
    }


    vector<SkyCoord>
    ConverterClient::azelToJ2000 (const vector<SkyCoord>& azel,
                                  const vector<EarthCoord>& pos,
                                  const vector<TimeCoord>& time)
    {
      // Set the value of ConverterOperation equal to AZEL->J2000
      ConverterOperation op; /* (to be done) */
                                
      // Send the request to the server
      itsRequest.send(op, azel, pos, time);

      // Vectors to hold the conversion result.
      vector<SkyCoord> sc;
      vector<EarthCoord> ec; // dummy
      vector<TimeCoord> tc; // dummy

      // Receive the result from the server.
      // \note This method is blocking.
      itsResult.receive(sc, ec, tc);

      // Return the result of the conversion.
      return sc;
    }


//     void ConverterClient::sendRequest()
//     {
//     }

//     void ConverterClient::recvResult()
//     {
//     }


  } // namespace AMC

} // namespace LOFAR
