/* Ranges.cc
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#include <lofar_config.h>

#include "Ranges.h"

#include <algorithm>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace Cobalt
  {

    std::ostream& operator<<( std::ostream &str, const Ranges &r )
    {
      for (struct Ranges::Range *i = r.begin; i != r.end; ++i)
        if (i->to != 0)
          str << "[" << i->from << ", " << i->to << ") ";

      return str;
    }

    Ranges::Ranges()
      :
      create(false),
      len(0),
      ranges(0),
      begin(0),
      end(begin),
      head(begin),
      minHistory(0)
    {
    }

    Ranges::Ranges( void *data, size_t numBytes, int64 minHistory, bool create )
      :
      create(create),
      len(numBytes / sizeof *ranges),
      ranges(create ? new(data)Range[len] : static_cast<Range*>(data)),
      begin(&ranges[0]),
      end(&ranges[len]),
      head(begin),
      minHistory(minHistory)
    {
      ASSERT( len > 0 );
    }

    Ranges::~Ranges()
    {
      if (create)
        for (struct Range *i = begin; i != end; ++i)
          i->~Range();
    }

    void Ranges::excludeBefore( int64 to )
    {
      for (struct Range *i = begin; i != end; ++i) {
        if (i->to <= to) {
          // erase; delete 'to' first!
          i->to = 0;
          i->from = 0;
          continue;
        }

        if (i->from < to) {
          // shorten
          i->from = to;
        }
      }
    }

    bool Ranges::include( int64 from, int64 to )
    {
      ASSERTSTR( from < to, from << " < " << to );
      ASSERTSTR( from >= head->to, from << " >= " << head->to );

      if (head->to == 0) {
        // *head is unused, set 'from' first!
        head->from = from;
        head->to = to;
        return true;
      }

      if (head->to == from) {
        // *head can be extended
        head->to = to;
        return true;
      }

      // new range is needed
      struct Range * const next = head + 1 == end ? begin : head + 1;

      if (next->to == 0 || next->to < to - minHistory) {
        // range at 'next' is either unused or old enough to toss away
        next->from = from;
        next->to = to;

        head = next;
        return true;
      }

      // no room -- discard
      return false;
    }

    bool Ranges::anythingBetween( int64 first, int64 last ) const
    {
      for(struct Range *i = begin; i != end; ++i) {
        // read in same order as writes occur
        int64 from = i->from;
        int64 to = i->to;

        if (to == 0) {
          // unused
          continue;
        }

        if (from >= to) {
          // read/write conflict
          continue;
        }

        from = std::max( from, first );
        to = std::min( to, last );

        if (from < to)
          return true;
      }

      return false;
    }

    SparseSet<int64> Ranges::sparseSet( int64 first, int64 last ) const
    {
      SparseSet<int64> result;

      if (first >= last)
        return result;

      for(struct Range *i = begin; i != end; ++i) {
        // read in same order as writes occur
        int64 from = i->from;
        int64 to = i->to;

        if (to == 0) {
          // unused
          continue;
        }

        if (from >= to) {
          // read/write conflict
          continue;
        }

        from = std::max( from, first );
        to = std::min( to, last );

        if (from < to)
          result.include(from, to);
      }

      return result;
    }

  }
}

