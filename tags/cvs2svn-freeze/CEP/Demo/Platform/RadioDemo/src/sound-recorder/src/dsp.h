// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _dsp_h
#define _dsp_h

#include <iostream.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dspsetting.h"

class Dsp
{
	private:
		DspSetting	dspSetting;
		int		dspHandle;

		bool		setSetting(int setting, int value);

	public:
				Dsp(DspSetting & setting);
				~Dsp();

		bool		init();
		void		close();

		const u_int32_t	getOptimalBufferSize();
		int		read(void * buffer, u_int32_t size);
		int		write(void * buffer, u_int32_t size);
};

#endif
