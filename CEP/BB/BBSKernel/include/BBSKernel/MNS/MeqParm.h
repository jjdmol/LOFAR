//# MeqParm.h: The base class for a parameter
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

#ifndef MNS_MEQPARM_H
#define MNS_MEQPARM_H

// \file
// The base class for a parameter

//# Includes
#include <BBS/MNS/MeqExpr.h>
#include <BBS/MNS/MeqResult.h>
#include <ParmDB/ParmValue.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR {

// \ingroup BBS
// \addtogroup MNS
// @{

//# Forward declarations
class MeqDomain;
class MeqParmGroup;
class MeqFunklet;

// This class is the (abstract) base class for parameters.
// The constructor assigns a unique id to the parameter and adds
// it to a map to find the id from the name.

class MeqParm : public MeqExprRep
{
public:
  // Create a parameter with the given name.
  // Assign a parameter id (a sequence number) to it.
  explicit MeqParm (const string& name, MeqParmGroup*);

  virtual ~MeqParm();

  // Get the parameter name.
  const string& getName() const
    { return itsName; }
  // Set the parameter name.
  void setName (const string& name)
    { itsName = name; }

  // Get the ParmDB seqnr.
  virtual int getParmDBSeqNr() const;

  // Get the parameter id.
  unsigned int getParmId() const
    { return itsParmId; }

  // Read the polcs for the given domain.
  virtual void fillFunklets (const std::map<std::string,ParmDB::ParmValueSet>&,
			     const MeqDomain&);

  // Initialize the solvable parameter for the given domain.
  virtual int initDomain (const vector<MeqDomain>&, int& pertIndex,
			  vector<int>& scidIndex);

  // Make parameter solvable, thus perturbed values have to be calculated.
  // spidIndex is the index of the first spid of this parm.
  // It returns the number of spids in this parm.
  void setSolvable (bool solvable)
    { itsIsSolvable = solvable; }

  bool isSolvable() const
    { return itsIsSolvable; }

  // Get the result of the parameter for the given domain.
  virtual MeqResult getResult (const MeqRequest&) = 0;

  // Get the current coefficients of the parameter.
  virtual const vector<MeqFunklet*>& getFunklets() const;

  // Make the new value persistent (for the given domain).
  virtual void save();

private:
  // A parm cannot be copied, otherwise problems arise with theirParms.
  MeqParm (const MeqParm&);
  MeqParm& operator= (const MeqParm&);

  string        itsName;
  unsigned int  itsParmId;
  bool          itsIsSolvable;
  MeqParmGroup* itsGroup;

  // A static vector of pointers to parms.
};



class MeqParmGroup
{
public:
  MeqParmGroup();

  // Add a Parm object to the list and return its id (seqnr in the list).
  int add (MeqParm*);

  // Remove a parm from the list.
  void remove (int index);

  // Get the nr of parms in the group.
  unsigned int nparms() const
    { return itsNparm; }

  // Get access to the parm list.
  const vector<MeqParm*>& getParms() const
    { return itsParms; }

  // Get a particular parm.
  const MeqParm* getParm (int index) const
    { return itsParms[index]; }

  // Clear the group and delete all its parms.
  void clear();

private:
  unsigned int     itsNparm;
  vector<MeqParm*> itsParms;
};

// @}

}

#endif
