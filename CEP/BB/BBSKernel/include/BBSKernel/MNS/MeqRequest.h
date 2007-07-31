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

// \file
// The request for an evaluation of an expression

//# Includes
#include <BBSKernel/MNS/MeqRequestId.h>
#include <BBSKernel/MNS/MeqDomain.h>
#include <BBSKernel/MNS/MeqMatrix.h>
#include <Common/lofar_vector.h>
#include <utility>

namespace LOFAR
{
namespace BBS
{
using std::pair;

// \ingroup BBSKernel
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
  // For y one can also give an ascending array of y-values, where the
  // first and last value have to match the domain boundaries.
  // <group>
  MeqRequest (const MeqDomain&, int nrx, int nry,
	      int nrSpid = 0);
  MeqRequest (const MeqDomain&, int nrx,
	      const vector<double>& y,
	      int nrSpid = 0);
  // </group>

  // Create a partial request for the given domain and values.
  MeqRequest (const MeqRequest&, uint stx, uint nrx, uint sty, uint nry);

  // Copy constructor.
  MeqRequest (const MeqRequest&);

  // Assignment
  MeqRequest& operator= (const MeqRequest&);

  // Set a new domain and number of cells.
  // <group>
  void setDomain (const MeqDomain&, int nrx, int nry);
  void setDomain (const MeqDomain&, int nrx,
		  const vector<double>& y);
  // </group>

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
  double y (int inx) const
    { return (itsYP[inx+1] + itsYP[inx]) / 2; }
  double sizeY (int inx) const
    { return itsYP[inx+1] - itsYP[inx]; }
  double startY (int inx) const
    { return itsYP[inx]; }
  double endY (int inx) const
    { return itsYP[inx+1]; }

  // Set or get the first X-value (i.e. first channel).
  // <group>
//  void setFirstX (int firstX)
//    { itsFirstX = firstX; }
//  int firstX() const
//    { return itsFirstX; }
  // </group>

  void setOffset(pair<size_t, size_t> offset)
  { itsOffset = offset; }

  pair<size_t, size_t> offset() const
  { return itsOffset; }

  // Get the request id.
  MeqRequestId getId() const
    { return itsRequestId; }

  // Initialize the static request id.
  static void initRequestId()
    { theirRequestId = 0; }

private:
  MeqRequestId   itsRequestId;
  MeqDomain      itsDomain;
  int            itsNx;
  int            itsNy;
  double         itsStepX;
  vector<double> itsY;     //# The vector of Y values.
  double*        itsYP;    //# The pointer to itsY for a partial request
//  int            itsFirstX;
  int            itsSourceNr;
  int            itsNspids;
  pair<size_t, size_t>  itsOffset;

  static MeqRequestId theirRequestId;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
