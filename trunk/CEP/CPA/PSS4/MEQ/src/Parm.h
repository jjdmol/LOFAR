//# Parm.h: The base class for a parameter
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

#ifndef MEQ_PARM_H
#define MEQ_PARM_H

//# Includes
#include <MEQ/Function.h>
#include <MEQ/Result.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>


namespace Meq {

//# Forward declarations
class Domain;


// This class is the (abstract) base class for parameters.
// The constructor assigns a unique id to the parameter and adds
// it to a map to find the id from the name.

class Parm : public Function
{
public:
  // Create a parameter with the given name.
  // Assign a parameter id (a sequence number) to it.
  explicit Parm (const string& name);

  virtual ~Parm();

  // Get the parameter name.
  const string& getName() const
    { return itsName; }
  // Set the parameter name.
  void setName (const string& name)
    { itsName = name; }

  // Get the parameter id.
  unsigned int getParmId() const
    { return itsParmId; }

  // Initialize the parameter for the given domain.
  virtual int initDomain (const Domain&, int spidIndex) = 0;

  // Make parameter solvable, thus perturbed values have to be calculated.
  // spidIndex is the index of the first spid of this parm.
  // It returns the number of spids in this parm.
  void setSolvable (bool solvable)
    { itsIsSolvable = solvable; }

  bool isSolvable() const
    { return itsIsSolvable; }

  // Get the result for the given request.
  virtual int getResultImpl (Result::Ref &resref, const Request&, bool) = 0;

  // Get the current values of the solvable parameter and store them
  // at their correct position in the argument.
  virtual void getInitial (Vells& values) const = 0;

  // Get the current values of the solvable parameter and store
  // them in the argument.
  // If needed, polynomial coefficients are denormalized.
  virtual void getCurrentValue(Vells& value, bool denormalize) const = 0;

  // Update the parameter with the new values.
  virtual void update (const Vells& value) = 0;

  // Make the new value persistent (for the given domain).
  virtual void save() = 0;

  // Get the list of all Parm objects.
  static const vector<Parm*>& getParmList();

  // Clear the list of all Parm objects.
  static void clearParmList();

private:
  // A parm cannot be copied, otherwise problems arise with theirParms.
  Parm (const Parm&);
  Parm& operator= (const Parm&);

  string       itsName;
  unsigned int itsParmId;
  bool         itsIsSolvable;

  // A static vector of pointers to parms.
  static unsigned int   theirNparm;
  static vector<Parm*>* theirParms;
};


} //namespace Meq

#endif
