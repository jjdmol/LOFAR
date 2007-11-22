// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _channel_h
#define _channel_h

#include <errno.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include "errorcodes.h"
#include "volume.h"

class Channel {

	public:
				Channel();
				~Channel();
				Channel(const Channel & channel);

		const Volume &	getChannelVolume();
		void		setChannelVolume(const Volume & vol);
		void		bindChannelToMixerDevice(const int mixDevice, const int channel);
		const char *	getChannelName();
		const bool	canChannelRecord() const;
		const bool	isChannelValid() const;
		const bool	isChannelStereo() const;
		const bool	isChannelInRecordState();
		void		setChannelRecordState(const bool state);

	private:
		Volume		volume;
		int		mixerHandle;
		int		mixerChannel;
		bool		validChannel;
		bool		canRecord;
		bool		isStereo;
};

#endif
