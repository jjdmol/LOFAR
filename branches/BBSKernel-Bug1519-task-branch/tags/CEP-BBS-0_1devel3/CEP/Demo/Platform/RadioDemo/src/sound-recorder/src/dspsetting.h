// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _dspsetting_h
#define _dspsetting_h

#include <iostream.h>

class DspSetting
{
	private:
		const char *	deviceName;
		int		sampleRate, rwMode, format, channels;


	public:
				DspSetting(){};
				DspSetting(const char * devicename, int samplerate, int rw_mode, 
						int snd_format, int nr_channels);

				DspSetting(const DspSetting & setting);
		const char *	getDeviceName() const { return deviceName;}
		const int	getSampleRate() const { return sampleRate;}
		const int	getRWMode() const { return rwMode;}
		const int	getFormat() const { return format;}
		const int	getChannels() const { return channels;}
};

#endif
