//#  SparseSet.h: portable <bitset> adaptation
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$


// #include "lofar_config.h"

#include <CS1_Interface/SparseSet.h>

#include <cstring>
#include <iostream>


namespace LOFAR
{

SparseSet &SparseSet::include(unsigned first, unsigned last)
{
  ++ last;

  if (ranges.size() == 0) {
    struct range range = { first, last };
    ranges.push_back(range);
  } else {
    std::vector<range>::iterator last_involved = ranges.end();

    while (-- last_involved >= ranges.begin() && last < last_involved->begin)
      ;

    std::vector<range>::iterator first_involved = last_involved + 1;

    while (first_involved > ranges.begin() && first <= first_involved[-1].end)
      -- first_involved;

    if (first_involved > last_involved) {
      // insert new range
      struct range range = { first, last };
      ranges.insert(first_involved, range);
    } else {
      // merge with existing range(s)
      first_involved->begin = std::min(first, first_involved->begin);
      first_involved->end   = std::max(last,  last_involved->end);

      ranges.erase(first_involved + 1, last_involved + 1);
    }
  }

  return *this;
}


unsigned SparseSet::count() const
{
  unsigned count = 0;

  for (std::vector<range>::const_iterator it = ranges.begin(); it != ranges.end(); it ++)
    count += it->end - it->begin;

  return count;
}


bool SparseSet::test(unsigned index) const
{
  std::vector<range>::const_iterator it = ranges.begin();

  while (it != ranges.end() && index >= it->begin)
    it ++;

  return it != ranges.begin() && index < it[-1].end;
}


SparseSet SparseSet::operator | (const SparseSet &other) const
{
  if (ranges.size() == 0) {
    return other;
  } else if (other.ranges.size() == 0) {
    return *this;
  } else {
    SparseSet union_set;
    std::vector<range>::const_iterator range1 = ranges.begin(), range2 = other.ranges.begin();

    while (range1 != ranges.end() && range2 != other.ranges.end()) {
      if (range1->end < range2->begin) {
	union_set.ranges.push_back(*range1 ++);
      } else if (range2->end < range1->begin) {
	union_set.ranges.push_back(*range2 ++);
      } else { // there is overlap, or range1 and range2 are contiguous
	struct range new_range;
	new_range.begin = std::min(range1->begin, range2->begin);

	// check if subsequent ranges from set1 and set2 must be joined as well
	while (1) {
	  if (range1 + 1 != ranges.end() && range1[1].begin <= range2->end) {
	    ++ range1;
	  } else if (range2 + 1 != other.ranges.end() && range2[1].begin <= range1->end) {
	    ++ range2;
	  } else {
	    break;
	  }
	}

	new_range.end = std::max(range1->end, range2->end);
	union_set.ranges.push_back(new_range);
	++ range1, ++ range2;
      }
    }

    union_set.ranges.insert(union_set.ranges.end(), range1, ranges.end());
    union_set.ranges.insert(union_set.ranges.end(), range2, other.ranges.end());
    return union_set;
  }
}


SparseSet &SparseSet::operator -= (size_t count)
{
  std::vector<range>::iterator src = ranges.begin(), dst = src;

  while (src != ranges.end() && src->end <= count)
    ++ src;

  if (src->begin < count)
    src->begin = count;

  for (; src != ranges.end(); src ++, dst ++) {
    dst->begin = src->begin - count;
    dst->end   = src->end   - count;
  }

  ranges.resize(ranges.size() + dst - src);

  return *this;
}


void SparseSet::write(BlobOStream &bos) const
{
  bos << (uint32) ranges.size();

  for (std::vector<range>::const_iterator it = ranges.begin(); it != ranges.end(); it ++)
    bos << (uint32) it->begin << (uint32) it->end;
}


void SparseSet::read(BlobIStream &bis)
{
  uint32 size, begin, end;

  bis >> size;
  ranges.resize(size);

  for (std::vector<SparseSet::range>::iterator it = ranges.begin(); it != ranges.end(); it ++) {
    bis >> begin >> end;
    it->begin = begin;
    it->end   = end;
  }
}



#if 0
SparseSet SparseSet::subset(unsigned first, unsigned last) const
{
  SparseSet subset;
  size_t    size = ranges.size();

  if (size > 0)
    for (const struct range *range = &ranges[0], *range_end = range + size; range < range_end; range ++)
      if (first < range->end && last > range->begin) {
	struct range new_range = {
	  std::max(first, range->begin) - first,
	  std::min(last, range->end) - first
	};

	subset.ranges.push_back(new_range);
      }

  return subset;
}
#endif

}
