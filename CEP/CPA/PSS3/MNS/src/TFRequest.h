//# TFRequest.h: The request for an evaluation of an expression
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

#if !defined(MNS_TFREQUEST_H)
#define MNS_TFREQUEST_H

//# Includes
#include <MNS/TFDomain.h>
#include <MNS/MnsMatrix.h>
#include <Common/lofar_vector.h>


// This class represents a request for which an expression has to be
// evaluated. It contains the domain and the spids for which the
// derivative has to calculated.

class TFRequest
{
public:
  // Create the request from the domain only.
  // No derivatives have to be calculated.
  TFRequest (const TFDomain&);

  // Create the request from the domain and number of spids for which
  // the derivatives have to be calculated.
  TFRequest (const TFDomain&, int nrSpid);

  // Get the domain.
  const TFDomain& domain() const
    { return itsDomain; }

  // Get the number of parameters.
  int nspid() const
    { return itsNspids; }

  // Get the values of the crossterms for a particular element in the domain.
  const MnsMatrix& getCrossTerms (int x, int y) const
    { return itsPowers[y*itsDomain.nx() + x]; }

private:
  // Fill in all possible powers (up to 9) of x and y and crossterms.
  void fillPowers();

  TFDomain    itsDomain;
  int         itsNspids;
  vector<MnsMatrix> itsPowers;
};


#endif
