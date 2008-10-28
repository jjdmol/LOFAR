//# ParmMap.h: A map of parameter name to value set
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
// @brief A map of parameter name to value set.
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_PARMMAP_H
#define LOFAR_PARMDB_PARMMAP_H

//# Includes
#include <ParmDB/ParmValue.h>
#include <Common/lofar_map.h>

namespace LOFAR {
namespace BBS {

  //# Forward Declarations.
  class ParmDB;


  // @ingroup ParmDB
  // @{

  // @brief A map of parameter name to value set.
  // ParmMap holds a map of name to ParmValueSet.
  // It is meant to hold the default values, but could be used for
  // other purposes as well.
  class ParmMap
  {
  public:
    // Set up a map for the given domain in the ParmDB.
    ParmMap()
    {}

    // Add or replace a ParmValueSet.
    void define (const string& name, const ParmValueSet& pset)
      { itsValueSets[name] = pset; }

    // Is the map empty?
    bool empty() const 
      { return itsValueSets.empty(); }

    // Return the size of the map.
    uint size() const 
      { return itsValueSets.size(); }

    // Get the value belonging to the name.
    // An exception is thrown if the value does not exist.
    const ParmValueSet& operator[] (const string& name) const;

    // Iterator functionality.
    // <group>
    typedef map<string, ParmValueSet>::iterator       iterator;
    typedef map<string, ParmValueSet>::const_iterator const_iterator;
    typedef map<string, ParmValueSet>::value_type     valueType;
    iterator begin()
      { return itsValueSets.begin(); }
    const_iterator begin() const
      { return itsValueSets.begin(); }
    iterator end()
      { return itsValueSets.end(); }
    const_iterator end() const
      { return itsValueSets.end(); }
    iterator find (const string& name)
      { return itsValueSets.find(name); }
    const_iterator find (const string& name) const
      { return itsValueSets.find(name); }
    // </group>

    // Clear the map.
    void clear()
      { itsValueSets.clear(); }

  private:
    // Cannot copy.
    // <group>
    ParmMap (const ParmMap&);
    ParmMap& operator= (const ParmMap&);
    // </group>

    map<string, ParmValueSet> itsValueSets;
  };

  // @}

} //# end namespace BBS
} //# end namspace LOFAR

#endif
