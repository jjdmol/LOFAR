#ifndef DTL_VARIANT_EXCEPTION_H
#define DTL_VARIANT_EXCEPTION_H

#include "RootException.h"

BEGIN_DTL_NAMESPACE

// exception class when something goes wrong in Variants
class VariantException : public RootException
{
public:
	VariantException(const tstring &meth, const tstring &msg);
	virtual ~VariantException() throw(){};

	// superclass behavior ok for what()
};

END_DTL_NAMESPACE

#endif
