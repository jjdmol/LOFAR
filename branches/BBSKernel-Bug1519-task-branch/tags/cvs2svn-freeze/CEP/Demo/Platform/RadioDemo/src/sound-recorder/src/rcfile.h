// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _rcfile_h
#define _rcfile_h

#include <sys/types.h>
#include <malloc.h>
#include <iostream.h>
#include <iomanip.h>
#include <fstream.h>
#include <string.h>

#define  LINEBUFFERSIZE 1024

class RCfile
{
	public:
				RCfile(const char * fileName, const bool askNew = false);
				~RCfile();
		const bool	open(const char * fileName, const bool askNew, const bool quiet);
		void		close();
		const bool	getEntry(const char label[], char buffer[], const int bufferSize);
		const bool	putEntry(const char label[], const char buffer[]);
		const bool	isNewlyCreated() const;
		const bool 	isOpen() const;

	private:
		char		* fName;
		fstream		* file;
		bool		isNewCreated;
};

#endif
