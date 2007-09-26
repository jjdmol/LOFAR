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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>


namespace LOFAR
{

struct less {
  bool operator() (const SparseSet::range &x, const SparseSet::range &y)
  {
    return x.end < y.begin;
  }
};

struct less_equal {
  bool operator() (const SparseSet::range &x, const SparseSet::range &y)
  {
    return x.end <= y.begin;
  }
};


SparseSet &SparseSet::include(unsigned first, unsigned last)
{
  if (first < last) {
    // find two iterators that mark the first resp. last involved ranges
    range r(first, last);
    std::pair<std::vector<range>::iterator, std::vector<range>::iterator> iters = equal_range(ranges.begin(), ranges.end(), r, less());

    if (iters.first == iters.second) {
      // insert new tuple
      ranges.insert(iters.first, r);
    } else {
      // combine with existing tuple(s)
      iters.first->begin = std::min(first, iters.first->begin);
      iters.first->end   = std::max(last , iters.second[-1].end);

      ranges.erase(iters.first + 1, iters.second);
    } 
  }

  return *this;
}


SparseSet &SparseSet::exclude(unsigned first, unsigned last)
{
  if (first < last) {
    // find two iterators that mark the first resp. last involved ranges
    // unlike in include(), a range that is adjacent to first or last is not
    // considered to be involved, hence the use of less_equal()
    std::pair<std::vector<range>::iterator, std::vector<range>::iterator> iters = equal_range(ranges.begin(), ranges.end(), range(first, last), less_equal());

    if (iters.first != iters.second) { // check if there are tuples involved
      if (iters.second - iters.first == 1 && first > iters.first->begin && last < iters.first->end) {
	// split tuple
	range r(last, iters.first->end);
	iters.first->end = first;
	ranges.insert(iters.second, r);
      } else {
	// possibly erase tuples
	if (first > iters.first->begin)
	  (iters.first ++)->end = first; // adjust first tuple; do not erase

	if (last < iters.second[-1].end)
	  (-- iters.second)->begin = last; // adjust last tuple; do not erase

	ranges.erase(iters.first, iters.second);
      }
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
  std::vector<range>::const_iterator it = lower_bound(ranges.begin(), ranges.end(), range(index, index + 1), less_equal());
  return it != ranges.end() && index >= it->begin;
}


SparseSet SparseSet::operator | (const SparseSet &other) const
{
  // iterate with two iterators over both sets, comparing the ranges to decide
  // what to do: include a range from the first set, include a range from the
  // second set, or merge (multiple) ranges from both sets.

  SparseSet union_set;
  std::vector<range>::const_iterator it1 = ranges.begin(), it2 = other.ranges.begin();

  while (it1 != ranges.end() && it2 != other.ranges.end()) {
    if (it1->end < it2->begin) {
      union_set.ranges.push_back(*it1 ++); // no overlap; *it1 is the smallest
    } else if (it2->end < it1->begin) {
      union_set.ranges.push_back(*it2 ++); // no overlap; *it2 is the smallest
    } else { // there is overlap, or it1 and it2 are contiguous
      unsigned new_begin = std::min(it1->begin, it2->begin);

      // check if subsequent ranges from set1 and set2 must be joined as well
      while (1) {
	if (it1 + 1 != ranges.end() && it1[1].begin <= it2->end) {
	  ++ it1;
	} else if (it2 + 1 != other.ranges.end() && it2[1].begin <= it1->end) {
	  ++ it2;
	} else {
	  break;
	}
      }

      union_set.ranges.push_back(range(new_begin, std::max(it1->end, it2->end)));
      ++ it1, ++ it2;
    }
  }

  // possibly append the remainder of the set that we have not finished yet
  union_set.ranges.insert(union_set.ranges.end(), it1, ranges.end());
  union_set.ranges.insert(union_set.ranges.end(), it2, other.ranges.end());
  return union_set;
}


SparseSet &SparseSet::operator += (size_t count)
{
  for (std::vector<range>::iterator it = ranges.begin(); it != ranges.end(); it ++)
    it->begin += count, it->end += count;

  return *this;
}


SparseSet &SparseSet::operator -= (size_t count)
{
  assert(ranges.size() == 0 || ranges[0].begin >= count);

  for (std::vector<range>::iterator it = ranges.begin(); it != ranges.end(); it ++)
    it->begin -= count, it->end -= count;

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

  for (std::vector<range>::iterator it = ranges.begin(); it != ranges.end(); it ++) {
    bis >> begin >> end;
    it->begin = begin;
    it->end   = end;
  }
}


ssize_t SparseSet::marshall(void *ptr, size_t maxSize) const
{
  size_t size = sizeof(uint32) + ranges.size() * sizeof(range);

  if (size > maxSize)
    return -1;

  * (uint32 *) ptr = ranges.size();
  memcpy((uint32 *) ptr + 1, &ranges[0], ranges.size() * sizeof(range));

  return size;
}


void SparseSet::unmarshall(const void *ptr)
{
  ranges.resize(* (uint32 *) ptr);
  memcpy(&ranges[0], (uint32 *) ptr + 1, ranges.size() * sizeof(range));
}


std::ostream &operator << (std::ostream &str, const SparseSet &set)
{
  for (std::vector<SparseSet::range>::const_iterator it = set.getRanges().begin(); it != set.getRanges().end(); it ++)
    if (it->end == it->begin + 1)
      str << '[' << it->begin << "] ";
    else
      str << '[' << it->begin << ".." << it->end << "> ";

  return str;
}

} // namespace LOFAR
