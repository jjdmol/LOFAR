//#  ConverterServer.cc: implementation of the Converter server.
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
#include <AMCImpl/AMCServer/ConverterServer.h>

namespace LOFAR
{
  namespace AMC
  {

    ConverterServer::ConverterServer(uint16 /*port*/)
    {
      // The server will create a listener socket that will handle any
      // incoming client requests.
    }


    ConverterServer::~ConverterServer()
    {
    }


    void ConverterServer::recvRequest(const vector<SkyCoord>&,
                                      const vector<EarthCoord>&,
                                      const vector<TimeCoord>&)
    {
    }


    void ConverterServer::sendResult(vector<SkyCoord>&)
    {
    }

 
  } // namespacd AMC

} // namespace LOFAR
