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

#include <string>
#include <DFTServer/DH_DFTRequest.h>
#include <DFTServer/DH_DFTResult.h>

namespace LOFAR {

// This class is a stub which is used to make the connection of a DFTServer
// client to the server and vice-versa.

class DFTStub
{
public:
  // Create the stub. Get its parameters from the given file name.
  explicit DFTStub (bool onServer=false,
		    const std::string& parameterFileName = "DFTServer.param");

  ~DFTStub();

  // Connect the given objects to the stubs.
  void connect (DH_DFTRequest&, DH_DFTResult&);

private:
  bool          itsStubOnServer;
  std::string   itsParmFileName;
  DH_DFTRequest itsRequest;
  DH_DFTResult  itsResult;
};

} //namespace

