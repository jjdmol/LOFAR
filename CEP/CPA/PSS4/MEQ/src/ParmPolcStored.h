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
#include <MEQ/Vells.h>

#pragma aidgroup MEQ
#pragma aid Tablename Default
#pragma types #MEQ::ParmPolcStored

namespace MEQ {

// This class contains the coefficients of a 2-dim polynomial.
// The order in time and frequency must be given.
// The nr of coefficients is (1+order(time)) * (1+order(freq)).
// The coefficients are numbered 0..N with the time as the most rapidly
// varying axis.

class ParmPolcStored: public ParmPolc
{
public:
  // The default constructor.
  // The object should be filled by the init method.
  ParmPolcStored();

  // Create a stored parameter with the given name and default value.
  // The default value is used if no suitable value can be found.
  // The parmtable can be undefined meaning that the parameter is temporary.
  ParmPolcStored (const string& name, ParmTable* table,
		  const Vells& defaultValue = Vells(0.));
  // This one opens the ParmTable if not open yet.
  ParmPolcStored (const string& name, const string& tableName,
		  const Vells& defaultValue = Vells(0.));

  virtual ~ParmPolcStored();

  // Initialize the parameter for the given domain.
  virtual int initDomain (const Domain&, int spidIndex);

  // Make the new value persistent (for the given domain).
  virtual void save();

  virtual void init (DataRecord::Ref::Xfer& initrec, Forest* frst);

  virtual void setState (const DataRecord& rec);

  //## Standard debug info method
  virtual string sdebug (int detail = 1, const string& prefix = "",
			 const char* name = 0) const;

private:
  ParmTable* itsTable;
  Vells      itsDefault;
};


} // namespace MEQ

#endif
