//# ParmPolcStored.h: Stored parameter with polynomial coefficients
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

#ifndef MEQ_PARMPOLCSTORED_H
#define MEQ_PARMPOLCSTORED_H

//# Includes
#include <MEQ/ParmPolc.h>
#include <MEQ/ParmTable.h>


namespace MEQ {

// This class contains the coefficients of a 2-dim polynomial.
// The order in time and frequency must be given.
// The nr of coefficients is (1+order(time)) * (1+order(freq)).
// The coefficients are numbered 0..N with the time as the most rapidly
// varying axis.

class ParmPolcStored: public ParmPolc
{
public:
  // Create a stored paramater with the given name and type.
  ParmPolcStored (const string& name, ParmTable* table);

  virtual ~ParmPolcStored();

  // Initialize the parameter for the given domain.
  virtual int initDomain (const Domain&, int spidIndex);

  // Make the new value persistent (for the given domain).
  virtual void save();

private:
  ParmTable* itsTable;
};


} // namespace MEQ

#endif
