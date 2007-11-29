// example showing the use of an indexed view in the dynamic case

#ifndef _DYNAMICINDEXEDVIEWEXAMPLE_H
#define _DYNAMICINDEXEDVIEWEXAMPLE_H

#include "example_core.h"

// BPA using a variant_row
inline void VariantBPAExample(BoundIOs &boundIOs, variant_row &paramObj)
{
	// make the bindings
	boundIOs[0] == paramObj._int();
	boundIOs[1] == paramObj._int();
	boundIOs[2] == paramObj._string();
	boundIOs[3] == paramObj._timestamp();

	boundIOs.BindVariantRow(paramObj);
}

// now we must have a VariantSetParamsExample
inline void VariantSetParamsExample(variant_row &params)
{
	// set parameter values
	params[0] = 2;
	params[1] = 8;
	params[2] = string("Example");
	
	TIMESTAMP_STRUCT paramDate = {2000, 1, 1, 0, 0, 0, 0};
	params[3] = paramDate;
}

// Dynamic IndexedDBView example
void DynamicIndexedViewExample();

// Dynamic IndexedDBView example using a variant_row ParamObj
void DynamicIndexedViewExampleVariantParamObj();

// IndexedDBView using a variant_row ParamObj
void IndexedViewExampleVariantParamObj();

#endif
