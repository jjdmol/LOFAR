// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _filefmt_h
#define _filefmt_h

class TFileFormat{

	public:
		virtual			~TFileFormat(){};
		virtual const bool	create(const char * filename, wavfmt_t fmt, 
						chnl_t chnl, bps_t bps, smpl_t smpl) = 0;
		virtual const bool	open(const char * filename) = 0;
		virtual const int	write(char * buffer, int len) = 0;
		virtual const int	read(char * buffer, int len) = 0;
		virtual void		close() = 0;
};

#endif
