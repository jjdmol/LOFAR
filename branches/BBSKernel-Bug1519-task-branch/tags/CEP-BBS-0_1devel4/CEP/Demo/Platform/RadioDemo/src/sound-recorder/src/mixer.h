// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _mixer_h
#define _mixer_h

#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/soundcard.h>
#include "channel.h"
#include "errorcodes.h"

class Mixer {

	public:
				Mixer();
				~Mixer();

		void		bindMixerToDevice(const char * deviceName);
		void		bindMixerToDevice(const int deviceNumber);

		const string &	readCurrentSetting();
		void		storeCurrentSetting(const string & setting);

		Channel &	getChannel(const char * deviceName);
		Channel &	getChannel(const int deviceNumber);

	private:
		const int	findDeviceId(const char * deviceName);
		void		close();

	private:
		Channel		channels[SOUND_MIXER_NRDEVICES];
		string		mixerDeviceName;
		int		mixer_fd;
};

#endif
