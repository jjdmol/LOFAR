//# tRanges.cc
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

#include <lofar_config.h>

#include <vector>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <CoInterface/SparseSet.h>

#include <Buffer/Ranges.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

int main( int, char **argv )
{
  INIT_LOGGER( argv[0] );

  size_t clock = 200 * 1000 * 1000;
  bool result;

  {
    LOG_INFO("Basic tests");

    vector<char> buf(10 * Ranges::elementSize());
    Ranges r(&buf[0], buf.size(), clock / 1024, true);

    result = r.include(10, 20);
    ASSERT(result);

    /* r == [10,20) */

    ASSERT(r.anythingBetween(10, 20));
    ASSERT(r.anythingBetween(0, 30));
    ASSERT(r.anythingBetween(0, 15));
    ASSERT(r.anythingBetween(15, 30));
    ASSERT(!r.anythingBetween(0, 10));
    ASSERT(!r.anythingBetween(20, 30));

    result = r.include(30, 40);
    ASSERT(result);

    /* r == [10,20) + [30,40) */

    SparseSet<int64> s(r.sparseSet(0,100));
    ASSERT(!s.test(9));
    ASSERT(s.test(10));
    ASSERT(s.test(11));
    ASSERT(s.test(19));
    ASSERT(!s.test(20));
    ASSERT(!s.test(29));
    ASSERT(s.test(30));
    ASSERT(s.test(31));
    ASSERT(s.test(39));
    ASSERT(!s.test(40));

    SparseSet<int64> s2(r.sparseSet(15,35));
    ASSERT(!s2.test(14));
    ASSERT(s2.test(15));
    ASSERT(s2.test(19));
    ASSERT(!s2.test(20));
    ASSERT(!s2.test(29));
    ASSERT(s2.test(30));
    ASSERT(s2.test(31));
    ASSERT(!s2.test(35));

    r.excludeBefore(35);

    /* r == [35,40) */

    ASSERT(!r.anythingBetween(0,35));
    ASSERT(r.anythingBetween(35,40));
    ASSERT(!r.anythingBetween(40,100));
  }


  return 0;
}

