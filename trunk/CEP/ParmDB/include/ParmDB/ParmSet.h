//# ParmSet.h: Set of parameters to be used
//#
//# Copyright (C) 2008
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

// @file
// @brief Set of parameters to be used
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_PARMSET_H
#define LOFAR_PARMDB_PARMSET_H

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

namespace LOFAR {
namespace BBS {

  //# Forward Declarations.
  class ParmDB;
  class ParmValueSet;
  class Box;


  // Define the type of a parmId.
  typedef uint ParmId;


  // @ingroup ParmDB
  // @{

  // @brief Set of parameters to be used
  class ParmSet
  {
  public:
    // Create an empty ParmSet.
    // It can be filled using function addParm.
    ParmSet();

    // Add a parm for the given ParmDB. If not existing in the parmDB,
    // it will be added to the ParmDB when its values are written.
    // <br>It returns a unique parmId.
    // <br>If the parm has been added before to the ParmSet, it returns
    // the already known parmid.
    ParmId addParm (ParmDB&, const string& name);

    // Does the parm already exist in the ParmDB?
    bool isInParmDB (ParmId parmid) const
      { return itsParms[parmid].getNameId() >= 0; }

    // Add zero or more existing parms.
    // A vector of unique parmIds is returned.
    // <group>
    //    vector<ParmId> addParms (ParmDB&, const vector<string>& names);
    //    vector<ParmId> addParms (ParmDB&, const string& namePattern);
    // </group>

    // Get the nr of parameters.
    size_t size() const
      { return itsParms.size(); }

    // Find the parmid of a previously added parm.
    // An exception is thrown if not existing in the ParmSet.
    ParmId find (const string& name) const;

    // Get the ParmDBs used in the ParmSet.
    const vector<ParmDB*> getDBs() const
      { return itsDBs; }

    // Get the values for the given work domain for all parameters that are
    // not part of the given vector.
    // It means that the vector gets extended for all parameters at the end
    // of itsParms.
    void getValues (vector<ParmValueSet>&, const Box& workDomain) const;

    // Write the parm values for the given parmid.
    // For parms with a new name, ParmKey::itsNameId will get filled in.
    // For new values, the rowId in the ParmValueSet will be filled in.
    void write (uint parmId, ParmValueSet&);

    // Clear the ParmSet.
    void clear();

  private:
    // The nested class ParmKey holds the basic info for a parameter.
    class ParmKey
    {
    public:
      // Create a parameter key.
      ParmKey (ParmDB* parmdb, const string& name, int nameId, ParmId parmId)
        : itsParmDB (parmdb),
          itsName   (name),
          itsNameId (nameId),
          itsParmId (parmId)
      {}

      // Get the name.
      const string& getName() const
        { return itsName; }

      // Get the ParmDB used for the parameter.
      // <group>
      ParmDB* getParmDBPtr()
        { return itsParmDB; }
      const ParmDB* getParmDBPtr() const
        { return itsParmDB; }
      // </group>

      // Get the name id.
      // >= 0 means name is stored in name table;
      // <0   means not stored.
      // <group>
      int& getNameId()
        { return itsNameId; }
      int getNameId() const
        { return itsNameId; }
      // </group>

      // Get he unique parm id.
      ParmId getParmId() const
        { return itsParmId; }

    private:
      ParmDB* itsParmDB;
      string  itsName;
      int     itsNameId;
      ParmId  itsParmId;
    };

    //# Data members of ParmSet.
    vector<ParmDB*> itsDBs;
    vector<ParmKey> itsParms;
    map<string,int> itsNames;
  };

  // @}

} //# end namespace BBS
} //# end namspace LOFAR

#endif
