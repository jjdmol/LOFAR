//# AxisMapping.h: Map the cells of one axis to another
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

#ifndef LOFAR_PARMDB_AXISMAPPING_H
#define LOFAR_PARMDB_AXISMAPPING_H

#include <ParmDB/Axis.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

namespace LOFAR {
namespace BBS {

  // This class defines the mapping of one axis to another.
  // It is meant for mapping the grid axes of a predict to the axes
  // of the domain grid, so it has to be calculated only once per predict.
  class AxisMapping
  {
  public:
    // Create the mapping.
    AxisMapping (const Axis& from, const Axis& to);

    // Iterator to get the next interval.
    // <group>
    typedef vector<int>::const_iterator const_iterator;
    const_iterator begin() const
      { return itsMapping.begin(); }
    const_iterator end() const
      { return itsMapping.end(); }
    // </group>

    // Get the number of elements.
    size_t size() const
      { return itsMapping.size(); }

    // Get the to interval for the i-th from interval.
    int operator[] (int i) const
      { return itsMapping[i]; }

    // Get a pointer to the scaled center values.
    // The center value of each interval in the from axis is scaled for
    // its interval in the to axis.
    const double* getScaledCenters() const
    { return &(itsCenters[0]); }

    // Get the borders telling which from-cells map to the same to-cell.
    // For example: borders (5,8,12) mean that from-cells 0-4 map to the same
    // to-cell, and so do from-cells 5-7 and 8-11.
    const vector<int>& getBorders() const
      { return itsBorders; }

  private:
    vector<int>    itsMapping;   //# cellnr in to for from-cell i
    vector<double> itsCenters;   //# center of from-cell i scaled to to-cell
    vector<int>    itsBorders;   //# last from-cell mapped to same to-cell
  };


  // This class caches axis mappings. It uses the unique id of the from-axis
  // and to-axis as the key in the cache.
  // <br>It is meant to avoid creating many identical mappings, because the
  // same axes are used for many parameters.
  class AxisMappingCache
  {
  public:
    // Define the key consisting of both id-s.
    struct AxisKey {
      AxisKey (uint fromId, uint toId)
	: itsFrom(fromId), itsTo(toId) {}
      uint itsFrom;
      uint itsTo;

      bool operator== (const AxisKey that) const
        { return itsFrom==that.itsFrom  &&  itsTo==that.itsTo; }
      bool operator!= (const AxisKey that) const
        { return itsFrom!=that.itsFrom  ||  itsTo!=that.itsTo; }
      bool operator< (const AxisKey that) const
        { return itsFrom<that.itsFrom  ||
	    itsFrom==that.itsFrom && itsTo < that.itsTo; }
    };

    // Get the number of elements.
    size_t size() const
      { return itsCache.size(); }

    // Find the possible mapping of axis 'from' to axis 'to'.
    // If no existing, create it.
    const AxisMapping& get (const Axis& from,const Axis& to) const
    {
      map<AxisKey,AxisMapping>::const_iterator iter =
	itsCache.find(AxisKey(from.getId(), to.getId()));
      return (iter == itsCache.end()  ?  makeMapping(from,to) : iter->second);
    }

  private:
    // Create a mapping of axis from to axis to and add it to the cache.
    const AxisMapping& makeMapping (const Axis& from, const Axis& to) const;

    //# Data members
    mutable map<AxisKey,AxisMapping> itsCache;
  };

} //# namespace BBS
} //# namespace LOFAR

#endif
