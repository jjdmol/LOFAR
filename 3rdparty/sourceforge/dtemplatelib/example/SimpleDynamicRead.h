// example of reading from a dynamic view ... when you don't know the
// columns or their types

#ifndef _SIMPLEDYNAMICREAD_H
#define _SIMPLEDYNAMICREAD_H

#include "example_core.h"

// Read the contents of a table and print the resulting rows
void SimpleDynamicRead();

vector<variant_row> ReadDynamicData();
#endif
