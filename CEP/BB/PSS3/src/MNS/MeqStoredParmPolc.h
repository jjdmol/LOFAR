//# MeqStoredParmPolc.h: Stored parameter with polynomial coefficients
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

#if !defined(MNS_MEQSTOREDPARMPOLC_H)
#define MNS_MEQSTOREDPARMPOLC_H

//# Includes
#include <PSS3/MNS/MeqParmPolc.h>
#include <PSS3/MNS/ParmTable.h>


// This class contains the coefficients of a 2-dim polynomial.
// The order in time and frequency must be given.
// The nr of coefficients is (1+order(time)) * (1+order(freq)).
// The coefficients are numbered 0..N with the time as the most rapidly
// varying axis.

class MeqStoredParmPolc: public MeqParmPolc
{
public:
  // Create a stored paramater with the given name and type.
  MeqStoredParmPolc (const string& name, int sourceNr, int station,
		     ParmTable* table);

  virtual ~MeqStoredParmPolc();

  int getSourceNr() const
    { return itsSrcnr; }

  int getStation() const
    { return itsStatnr; }

  // Read the polcs for the given domain.
  virtual void readPolcs (const MeqDomain& domain);

  // Initialize the solvable parameter for the given domain.
  // It checks if the given domain matches the one used for the last readPolcs.
  virtual int initDomain (const MeqDomain&, int spidIndex);

  // Make the new value persistent (for the given domain).
  virtual void save();

private:
  int        itsSrcnr;
  int        itsStatnr;
  ParmTable* itsTable;
  MeqDomain  itsDomain;
};


#endif
