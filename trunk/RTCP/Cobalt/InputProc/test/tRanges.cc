#include <lofar_config.h>
#include "Buffer/Ranges.h"
#include <Common/LofarLogger.h>
#include <vector>

using namespace LOFAR;
using namespace RTCP;
using namespace std;

int main( int, char **argv ) {
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
