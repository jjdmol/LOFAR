//# DFTStub.cc: Stub for connection to DFTServer and DFTRequest
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <DFTServer/DFTStub.h>
#include <Transport/TH_Socket.h>
#include <ACC/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

namespace LOFAR { 

  DFTStub::DFTStub (bool stubOnServer,
		    const std::string& parameterFileName)
    : itsStubOnServer (stubOnServer),
      itsParmFileName (parameterFileName)
  {}

  DFTStub::~DFTStub()
  {}

  void DFTStub::connect (DH_DFTRequest& req, DH_DFTResult& res)
  {
    const ParameterSet myPS(itsParmFileName);
    TH_Socket thReq(myPS.getString("DFTConnection.ServerHost"), // recvhost
		    myPS.getInt("DFTConnection.RequestPort"),   // port
		    false
		    );
    TH_Socket thRes(myPS.getString("DFTConnection.ServerHost"), // sendhost
		    myPS.getInt("DFTConnection.ResultPort"),    // port
		    true
		    );    
    itsRequest.setID(998);
    itsResult.setID(999);
    if (itsStubOnServer) {
      itsRequest.connectTo (req, thReq);
      res.connectTo (itsResult, thRes);
    } else {
      req.connectTo (itsRequest, thReq);
      itsResult.connectTo (res, thRes);
    }
  };

} //namespace

