//# TFParm.h: The base class for a parameter
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

#if !defined(MNS_TFPARM_H)
#define MNS_TFPARM_H

//# Includes
#include <MNS/TFExpr.h>
#include <MNS/TFRange.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

//# Forward declarations
class TFDomain;


// This class is the (abstract) base class for parameters.
// The constructor assigns a unique id to the parameter and adds
// it to a map to find the id from the name.

class TFParm : public TFExpr
{
public:
  // Create a parameter with the given type.
  // Assign a parameter id (a sequence number) to it.
  // Add the parameter name to the static vector of parms.
  explicit TFParm (unsigned int type);

  virtual ~TFParm();

  // Get the parameter type or id.
  unsigned int getType() const
    { return itsType; }
  unsigned int getParmId() const
    { return itsParmId; }

  // Set the parameter type.
  void setType (unsigned int type)
    { itsType = type; }

  // Initialize the parameter for the given domain.
  virtual void init (const TFDomain&) = 0;

  // Make parameter solvable, thus perturbed values have to be calculated.
  // spidIndex is index the first spid of this parm.
  // It returns the number of spids in this parm.
  virtual int setSolvable (int spidIndex) = 0;

  // Make the parameter non-solvable.
  virtual void clearSolvable() = 0;

  // Get the range of the parameter for the given domain.
  virtual TFRange getRange (const TFRequest&) = 0;

  // Update the parameter with the new value.
  virtual void update (const MnsMatrix& value) = 0;

  // Make the new value persistent (for the given domain).
  virtual void save (const TFDomain&) = 0;

  // Get the list of all TFParm objects.
  static const vector<TFParm*>& getParmList();

private:
  unsigned int itsType;
  unsigned int itsParmId;

  // A static vector of pointers to parms.
  static vector<TFParm*>* theirParms;
};


#endif
