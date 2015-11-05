#ifndef RANGES
#define RANGES

#include <Interface/SparseSet.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>

#include <ostream>

namespace LOFAR {
namespace RTCP {

//
// Thread-safe, lock-free set of int64 [from,to) ranges.
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

class Ranges {
public:
  Ranges();
  Ranges( void *data, size_t numBytes, int64 minHistory, bool create );
  ~Ranges();

  // remove [0,to)
  void excludeBefore( int64 to );

  // add a range [from,to), and return whether the addition
  // was succesful.
  bool include( int64 from, int64 to );

  // returns whether there is anything set in [first, last)
  bool anythingBetween( int64 first, int64 last ) const;

  SparseSet<int64> sparseSet( int64 first, int64 last ) const;

  static size_t elementSize() { return sizeof(struct Range); }

private:
  struct Range {
    // Write 'from' before 'to' to allow the following invariant:
    //
    // from <  to   : a valid range
    // from >= to   : invalid range (being written)
    // from = to = 0: an unused range
    volatile int64 from, to;

    Range(): from(0), to(0) {}
  };

  bool create;
  size_t len;
  Range *ranges;
  Range *begin;
  Range *end;
  Range *head;

  // minimal history to maintain (samples newer than this
  // will be maintained in favour of newly added ranges)
  int64 minHistory;

public:
  static size_t size(size_t numElements) {
    return numElements * sizeof(struct Range);
  }

  friend std::ostream& operator<<( std::ostream &str, const Ranges &r );
};

std::ostream& operator<<( std::ostream &str, const Ranges &r );

}
}

#endif

