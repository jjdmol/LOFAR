#ifndef RANGES
#define RANGES

#include <Interface/SparseSet.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>

#include <ostream>

namespace LOFAR {
namespace RTCP {

//
// Thread-safe set of int64 [from,to) ranges.
//
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

private:
  struct Range {
    // Write'from' before 'to' to allow the following invariant:
    //
    // from <  to   : a valid range
    // from >= to   : invalid range (being written)
    // from = to = 0: an unused range
    volatile int64 from, to;

    Range(): from(0), to(0) {}
  };

  size_t len;
  Range * ranges;
  Range * begin;
  Range * end;
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

std::ostream& operator<<( std::ostream &str, const Ranges &r )
{
  
  for (struct Ranges::Range *i = r.begin; i != r.end; ++i)
    if (i->to != 0)
      str << "[" << i->from << ", " << i->to << ") ";

  return str;
}

Ranges::Ranges()
:
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

    if (i->from > to) {
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
    // *head is unused
    head->from = from;
    head->to   = to;
    return true;
  }

  if (head->to == from) {
    // *head can be extended
    head->to = to;
    return true;
  }

  // new range is needed
  struct Range * const next = head + 1 == end ? begin : head + 1;

  if (next->to < to - minHistory) {
    // range at 'next' is old enough to toss away
    next->from = from;
    next->to   = to;

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
    int64 to   = i->to;

    if (to == 0) {
      // unused
      continue;
    }

    if (from >= to) {
      // read/write conflict
      continue;
    }

    from = std::max( from, first );
    to   = std::min( to, last );

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
    int64 to   = i->to;

    if (to == 0) {
      // unused
      continue;
    }

    if (from >= to) {
      // read/write conflict
      continue;
    }

    from = std::max( from, first );
    to   = std::min( to, last );

    if (from < to)
      result.include(from, to);
  }

  return result;
}

}
}

#endif

