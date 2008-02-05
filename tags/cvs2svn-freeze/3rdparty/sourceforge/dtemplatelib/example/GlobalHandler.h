#ifndef _GLOBALHANDLER_H
#define _GLOBALHANDLER_H

#include "example_core.h"

class GlobalHandlerExample
{
public:
	dtl_ios_base::MeansOfRecovery operator()(const RootException *pEx,
											 ErrorSeverity::SeverityCode severity);

};

void UseGlobalHandlerExample();

#endif
