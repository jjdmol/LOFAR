// this example shows range insert transactions in action

#ifndef _RANGE_H
#define _RANGE_H

#include "example_core.h"

// this example shows range insert transactions in action
void RangeInsertExample();

// same, but for an indexed view
void RangeIndexInsertExample();

// range update (fail)
void RangeIndexUpdateExampleFailure();

// range update (successful)
void RangeIndexUpdateExampleSuccess();

#endif
