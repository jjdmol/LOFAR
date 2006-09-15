//# MeqPolc.h: Ordinary polynomial with coefficients valid for a given domain
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

#ifndef MNS_MEQPOLC_H
#define MNS_MEQPOLC_H

// \file
// Ordinary polynomial with coefficients valid for a given domain

//# Includes
#include <BBSKernel/MNS/MeqFunklet.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBS
// \addtogroup MNS
// @{

// This class defines a polynomial as a specialization of a funklet.
//
// It scales the X and Y values between 0 and 1. So the value to use for X is:
//        (X - domain.startX()) / (domain.endX() - domain.startX())
// Y is to be calculated similarly.

class MeqPolc: public MeqFunklet
{
public:
  // Create an empty 2-dim polynomial.
  MeqPolc()
    {}

  // Create a polc from a ParmValue object.
  MeqPolc (const ParmDB::ParmValue&);

  // Convert a polc to a ParmValue object.
  ParmDB::ParmValue toParmValue() const;

  virtual ~MeqPolc();

  // Clone the polc.
  virtual MeqPolc* clone() const;

  // Calculate the value and possible perturbations.
  virtual MeqResult getResult (const MeqRequest&,
			       int nrpert, int pertInx);
  virtual MeqResult getAnResult (const MeqRequest&,
				 int nrpert, int pertInx);
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
