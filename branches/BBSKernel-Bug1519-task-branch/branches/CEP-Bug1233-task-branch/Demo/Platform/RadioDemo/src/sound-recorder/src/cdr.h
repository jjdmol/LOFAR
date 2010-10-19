// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _cdr_h
#define _cdr_h

#include "types.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "filefmt.h"

class TCdr : public TFileFormat{

	public:
		TCdr();
		~TCdr();

		const bool	create(const char * filename, wavfmt_t fmt, chnl_t chnl, 
					bps_t bps, smpl_t smpl);
		const bool	open(const char * filename);
		const int	write(char * buffer, int len);
		const int	read(char * buffer, int len);
		void		close();

	private:
		void		changeByteOrder(char * buffer, const int len);

	private:
		FILE *		fh;
		int		mode;
};

#endif
