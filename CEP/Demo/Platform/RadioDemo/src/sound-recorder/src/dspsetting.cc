// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "dspsetting.h"


// --------------------------------------------------------------------

DspSetting::DspSetting(const DspSetting & setting) : deviceName(setting.deviceName)
{
	sampleRate = setting.sampleRate;
	format     = setting.format;
	rwMode     = setting.rwMode;
	channels   = setting.channels;
}

DspSetting::DspSetting(const char * devicename, int samplerate, int rw_mode, 
		int snd_format, int nr_channels) : deviceName(devicename)
{
	rwMode = rw_mode;
	sampleRate = samplerate;
	format = snd_format;
	channels = nr_channels;
}
