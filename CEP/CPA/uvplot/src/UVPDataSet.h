// Copyright notice should go here

#if !defined(UVPDATASET_H)
#define UVPDATASET_H

// $Id$


#include <vector>
#include <map>
#include <UVPDataAtom.h>

//! Atom = TimeCache[timeslot]
typedef std::vector<UVPDataAtom>          UVPTimeCache;

//! Atom = BaselineCache[baseline][timeslot]
typedef std::vector<UVPTimeCache>  UVPBaselineCache;


//! Atom = CorrelationCache[correlation][baseline][timeslot]
typedef std::vector<UVPBaselineCache>  UVPCorrelationCache;


//! Atom = BaselineCache[field][correlation][baseline][timeslot]
typedef std::vector<UVPCorrelationCache>  UVPFieldCache;


//! Atom = BaselineCache[field][correlation][baseline][timeslot]
//typedef UVPFieldCache     UVPDataSet;

typedef std::map<UVPDataAtomHeader, UVPDataAtom> UVPDataSet;

#endif // UVPDATASET_H
