// example function for reading from a view that returns no records

#ifndef _READDATANOMATCHES_H
#define _READDATANOMATCHES_H

#include "example_core.h"

// should return an empty vector of Example objects as we're feeding the view
// a query that should return no matches
vector<Example> ReadDataNoMatches();

#endif
