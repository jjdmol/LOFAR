// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _errorcodes_h
#define _errorcodes_h

#include <string>

enum errCodes { 
	MIXERDEV_OUT_OF_RANGE = 2000,
	CHANNEL_ILLEGAL_MIXER_HANDLE = 2050
};

class ErrorCodes {

	public:
				ErrorCodes();
				~ErrorCodes();

		const char *	strError(const int errorCode);
};

#endif
