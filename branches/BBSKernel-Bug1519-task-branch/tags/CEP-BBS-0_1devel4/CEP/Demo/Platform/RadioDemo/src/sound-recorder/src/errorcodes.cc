// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "errorcodes.h"

ErrorCodes::ErrorCodes()
{
}

ErrorCodes::~ErrorCodes()
{
}

const char * strError(const int errorCode)
{
	char * result = 0;

	switch(errorCode){
		case MIXERDEV_OUT_OF_RANGE:
			result = "Mixer devicenumber out of range.";
			break;
	}

	return result;
}

