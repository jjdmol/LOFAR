#include "CS1_Interface/SparseSet.h"

#include <cassert>
#include <bitset>
#include <iostream>


#define BITSET_SIZE	4096


using namespace LOFAR;
using namespace std;


void printSet(SparseSet &set)
{
  const vector<SparseSet::range> ranges = set.getRanges();

  cout << '{';

  for (unsigned i = 0; i < ranges.size(); i ++) {
    if (i > 0)
      cout << ',';

    cout << '[' << ranges[i].begin << ',' << (ranges[i].end - 1) << ']';
  }

  cout << "}\n";
}


bool equal(SparseSet &sset, bitset<BITSET_SIZE> &bset)
{
  if (sset.count() != bset.count())
    return false;

  for (unsigned i = 0; i < BITSET_SIZE; i ++)
    if (sset.test(i) != bset.test(i))
      return false;

  return true;
}


int main(void)
{
  SparseSet sset;
  //sset.include(7, 10).include(12, 14).include(20, 22).include(24, 26);
  //sset.include(10, 30).exclude(15,20);
  //printSet(sset);
  //return 0;

  //sset.exclude(0,26);
  //printSet(sset);
  //sset.include(1832, 1886);
  //printSet(sset);

  for (unsigned i = 0; i < 1000; i ++) {
    SparseSet		sset, sset_union;
    bitset<BITSET_SIZE> bset, bset_union;

    for (unsigned j = 0; j < 100; j ++) {
      unsigned first = (unsigned) (drand48() * (BITSET_SIZE - 100));
      unsigned last  = (unsigned) (drand48() * 100) + first;

      if (drand48() > .4) {
	sset.include(first, last);

	for (unsigned k = first; k <= last; k ++)
	  bset.set(k);
      } else {
	sset.exclude(first, last);

	for (unsigned k = first; k <= last; k ++)
	  bset.reset(k);
      }

      assert(equal(sset, bset));

      if (drand48() < .1) {
	sset_union = sset_union | sset;
	bset_union |= bset;
	assert(equal(sset_union, bset_union));
	sset.reset();
	bset.reset();
      }
    }
  }

  for (int i = 0; i < 23; i ++) {
    SparseSet		sset;
    bitset<BITSET_SIZE> bset(0x00727780 >> i);
    sset.include(7, 10).include(12, 14).include(17).include(20, 22) -= i;
    assert(equal(sset, bset));
  }

  return 0;
}
