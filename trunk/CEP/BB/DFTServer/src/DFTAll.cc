//# DFTAll.cc: Class to process all DFTs
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

#include <DFTServer/DFTAll.h>
#include <Transport/TH_Socket.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{


  DFTAll::DFTAll()
  {
    itsRequest.preprocess();
    itsResult.preprocess();
    itsRequest.setID (0);
    itsResult.setID (1);
    itsStub.connect (itsRequest, itsResult);
  }

  DFTAll::~DFTAll()
  {}

  // Set the values.
  void DFTAll::set (double startFreq, double stepFreq, int nFreq,
		    double startTime, double stepTime, int nTime,
		    int nAnt, int nBaseline)
  {
    itsRequest.set (startFreq, stepFreq, nFreq, startTime, stepTime, nTime,
		    nAnt, nBaseline);
  }

  void DFTAll::receive()
  {
    itsResult.read();
    ASSERT (itsResult.getNFreq() == itsRequest.getNFreq());
    ASSERT (itsResult.getNTime() == itsRequest.getNTime());
    ASSERT (itsResult.getNBaseline() == itsRequest.getNBaseline());
  }

  void DFTAll::quit()
  {
    itsRequest.set (0., 0., 0, 0., 0., 0, 0, 0);
    itsRequest.write();
  }
}
