//# MeqRequest.h: The request for an evaluation of an expression
//#
//# Copyright (C) 2002
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

#if !defined(MNS_MEQREQUEST_H)
#define MNS_MEQREQUEST_H

// \file MNS/MeqRequest.h
// The request for an evaluation of an expression

//# Includes
#include <BBS3/MNS/MeqRequestId.h>
#include <BBS3/MNS/MeqDomain.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <Common/lofar_vector.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

// This class represents a request for which an expression has to be
// evaluated. It contains the domain and the spids for which the
// derivative has to calculated.

class MeqRequest
{
public:
  // Create the request from the domain and number of spids for which
  // the derivatives have to be calculated.
  // Give the number of cells in the domain.
  explicit MeqRequest (const MeqDomain&, int nrx, int nry, int nrSpid = 0);

  // Set a new domain and number of cells.
  void setDomain (const MeqDomain&, int nrx, int nry);

  // Get the domain.
  const MeqDomain& domain() const
    { return itsDomain; }

  // Get the number of parameters.
  int nspid() const
    { return itsNspids; }

  // Get the number of cells.
  int nx() const
    { return itsNx; }
  int ny() const
    { return itsNy; }
  int ncells() const
    { return itsNx * itsNy; }

  double stepX() const
    { return itsStepX; }
  double stepY() const
    { return itsStepY; }

  // Set or get source nr.
  void setSourceNr (int sourceNr)
    { itsSourceNr = sourceNr; }
  int getSourceNr() const
    { return itsSourceNr; }

  // Get the request id.
  MeqRequestId getId() const
    { return itsRequestId; }

  // Initialize the static request id.
  static void initRequestId()
    { theirRequestId = 0; }

private:
  MeqRequestId itsRequestId;
  MeqDomain    itsDomain;
  int          itsNx;
  int          itsNy;
  double       itsStepX;
  double       itsStepY;
  int          itsSourceNr;
  int          itsNspids;

  static MeqRequestId theirRequestId;
};

// @}

}

#endif
