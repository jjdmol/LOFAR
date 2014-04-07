#ifndef LOFAR_CNPROC_RING_H
#define LOFAR_CNPROC_RING_H

namespace LOFAR {
namespace RTCP {

#include <vector>

/*
 * Ring handles a ring of subbands or beams to be processed by a compute node.
 *
 * pset:        the pset index of this node
 * numperpset:  the number of subbands or beams to be processed per pset
 * core:        the core index of this node
 * numcores:    the number of cores per pset (that will be used)
 */

class Ring {
public:
  Ring( unsigned pset, unsigned numperpset, unsigned core, unsigned numcores ):
    pset(pset),
    core(core),
    numperpset(numperpset),
    numcores(numcores),

    first(pset * numperpset),
    last((pset+1) * numperpset),
    increment(numcores % numperpset),
    current(first + core % numperpset) {}

  // emulate a cast to (unsigned) for ease of use, and add a few shorthands
  operator unsigned() const { return current; }

  void next();

  // returns the relative core number within this pset to process this 'second' of data
  unsigned relative() const { return current - first; }

  // is the current element the last to be processed for this 'second' of data?
  bool isLast() const { return current + increment >= last || numcores >= numperpset; }

  // list the elements to process
  std::vector<unsigned> list() const;

  const unsigned pset;
  const unsigned core;

  const unsigned numperpset;
  const unsigned numcores;

  const unsigned first;
  const unsigned last;
  const unsigned increment;

private:
  unsigned current;
};

inline void Ring::next()
{
  if ((current += increment) >= last) {
    current -= last - first;
  }
}

inline std::vector<unsigned> Ring::list() const
{
  std::vector<unsigned> list;
  Ring copy = *this;

  for (Ring copy = *this; list.empty() || copy.current != current; copy.next()) {
    list.push_back(copy);
  }

  return list;
}


} // namespace RTCP
} // namespace LOFAR

#endif
