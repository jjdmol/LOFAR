// example function for reading from a view

#ifndef _READDATA_H
#define _READDATA_H

#include "example_core.h"

// Read the contents of the DB_EXAMPLE table and return a vector of the
// resulting rows

vector<Example> ReadData();

void BulkCopyExample();

void InsLong();

void TableStructExample();

void RandomDBViewExample();
void RandomDynamicDBView();

#endif
