//# TFParmSingle.h: The class for a single parameter
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

#if !defined(MNS_TFPARMSINGLE_H)
#define MNS_TFPARMSINGLE_H

//# Includes
#include <MNS/TFParm.h>


// This class is the (abstract) base class for parameters.
// The constructor assigns a unique id to the parameter and adds
// it to a map to find the id from the name.

class TFParmSingle : public TFParm
{
public:
  // Create a parameter with the given name.
  explicit TFParmSingle (unsigned int type, double initValue = 1);

  // Initialize the parameter for the given domain.
  void init (const TFDomain&);

  // Make parameter solvable, thus perturbed values have to be calculated.
  // spidIndex is index the first spid of this parm.
  // It returns the number of spids in this parm.
  virtual int setSolvable (int spidIndex);

  // Get the range of the parameter for the given domain.
  virtual TFRange getRange (const TFRequest&);

  // Update the solvable parameter with the new value.
  virtual void update (const MnsMatrix& value);

  // Make the new value persistent (for the given domain).
  virtual void save (const TFDomain&);

private:
  double  itsInitialValue;
  double  itsNewValue;
  int     itsSolveIndex;
};


#endif
