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

#if !defined(MNS_MEQPARM_H)
#define MNS_MEQPARM_H

// \file MNS/MeqParm.h
// The base class for a parameter

//# Includes
#include <BBS3/MNS/MeqExpr.h>
#include <BBS3/MNS/MeqResult.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward declarations
class MeqDomain;
class MeqParmGroup;


// This class is the (abstract) base class for parameters.
// The constructor assigns a unique id to the parameter and adds
// it to a map to find the id from the name.

class MeqParm : public MeqExpr
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

  // Get the name and type of the database and table holding the parameter.
  // By default it returns an empty string.
  // <group>
  virtual string getTableName() const;
  virtual string getDBType() const;
  virtual string getDBName() const;
  // </group>

  // Get the parameter id.
  unsigned int getParmId() const
    { return itsParmId; }

  // Read the polcs for the given domain.
  virtual void readPolcs (const MeqDomain& domain) = 0;

  // Initialize the solvable parameter for the given domain.
  virtual int initDomain (const MeqDomain&, int spidIndex) = 0;

  // Make parameter solvable, thus perturbed values have to be calculated.
  // spidIndex is the index of the first spid of this parm.
  // It returns the number of spids in this parm.
  void setSolvable (bool solvable)
    { itsIsSolvable = solvable; }

  bool isSolvable() const
    { return itsIsSolvable; }

  // Get the result of the parameter for the given domain.
  virtual MeqResult getResult (const MeqRequest&) = 0;

  // Get the current values of the solvable parameter and store
  // them in the argument.
  virtual void getCurrentValue (MeqMatrix& value) const = 0;
  MeqMatrix getCoeffValues() const
    { MeqMatrix tmp; getCurrentValue(tmp); return tmp; }

  // Update the parameter with the new values.
  virtual void update (const MeqMatrix& value) = 0;

  // Make the new value persistent (for the given domain).
  virtual void save() = 0;

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
