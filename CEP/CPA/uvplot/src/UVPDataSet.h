// Copyright notice should go here

#if !defined(UVPDATASET_H)
#define UVPDATASET_H

// $Id$


#include <vector>
#include <map>
#include <uvplot/UVPDataAtom.h>

//! Atom = TimeCache[timeslot]
typedef std::vector<UVPDataAtom>          UVPTimeCache;

typedef std::map<UVPDataAtomHeader, UVPDataAtom> UVPDataSet;

#endif // UVPDATASET_H
