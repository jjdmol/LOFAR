//# Ranges.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: $

#ifndef LOFAR_INPUT_PROC_RANGES_H
#define LOFAR_INPUT_PROC_RANGES_H

#include <ostream>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include "BufferSettings.h"

namespace LOFAR
{
  namespace Cobalt
  {

    //
    // Thread-safe, lock-free set of value_type [from,to) ranges.
    //
    // This implementation is thread safe for one writer and any number
    // of readers.
    //
    // To maintain integrity, we maintain a fixed set of [from,to)
    // ranges. For any range, if to == 0, the range is either unused
    // or in the process of being updated. The calling process needs
    // to make sure that it updates ranges in an order that ensures
    // integrity w.r.t. the data that is represented. That is,
    // exclude ranges that will be overwritten before writing and
    // including the new data.

    class Ranges
    {
    public:
      typedef BufferSettings::range_type value_type;

      Ranges();
      Ranges( void *data, size_t numBytes, value_type minHistory, bool create );
      ~Ranges();

      // Remove [0,to)
      void excludeBefore( value_type to );

      // Add a range [from,to), and return whether the addition
      // was succesful.
      bool include( value_type from, value_type to );

      // Returns whether there is anything set in [first, last)
      bool anythingBetween( value_type first, value_type last ) const;

      // Returns [first, last) as a SparseSet
      BufferSettings::flags_type sparseSet( value_type first, value_type last ) const;

      // The size of a single [from,to) pair.
      static size_t elementSize()
      {
        return sizeof(struct Range);
      }

    private:
      struct Range {
        // Write 'from' before 'to' to allow the following invariant:
        //
        // from <  to   : a valid range
        // from >= to   : invalid range (being written)
        // from = to = 0: an unused range
        volatile value_type from, to;

        Range() : from(0), to(0)
        {
        }
      };

      bool create;
      size_t len;
      Range *ranges;
      Range *begin;
      Range *end;
      Range *head;

      // minimal history to maintain (samples newer than this
      // will be maintained in favour of newly added ranges)
      value_type minHistory;

    public:
      // The size of this object for a given number of [from,to) pairs.
      static size_t size(size_t numElements)
      {
        return numElements * sizeof(struct Range);
      }

      friend std::ostream& operator<<( std::ostream &str, const Ranges &r );
    };

    std::ostream& operator<<( std::ostream &str, const Ranges &r );

  }
}

#endif

