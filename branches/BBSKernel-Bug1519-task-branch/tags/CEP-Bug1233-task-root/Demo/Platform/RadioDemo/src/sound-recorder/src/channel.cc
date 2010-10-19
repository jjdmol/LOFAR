// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "channel.h"

static char * labels[] = SOUND_DEVICE_LABELS;

Channel::Channel()
{
	mixerHandle = 0;
	mixerChannel = 0;
	validChannel = false;
	canRecord = false;
	isStereo  = false;
}

Channel::~Channel()
{
}

Channel::Channel(const Channel & channel)
{
	volume = channel.volume;
	mixerHandle = channel.mixerHandle;
	mixerChannel = channel.mixerChannel;
	validChannel = channel.validChannel;
	canRecord = channel.canRecord;
	isStereo  = channel.isStereo;
} 

const Volume & Channel::getChannelVolume()
{
	volume_t vol;

	if(validChannel == true){
		if(ioctl(mixerHandle, MIXER_READ(mixerChannel), & vol) == -1)
			throw errno;
	} else {
		vol = 0;
	}

	volume.setVolume(vol);

	return volume;
}

void Channel::setChannelVolume(const Volume & vol)
{
	if(validChannel == true){
		volume_t volume = vol.getVolume();
		if(ioctl(mixerHandle, MIXER_WRITE(mixerChannel), & volume) == -1)
			throw errno;
	}
}

void Channel::bindChannelToMixerDevice(const int mixDevice, const int channel)
{
	if(mixDevice < 0)
		throw CHANNEL_ILLEGAL_MIXER_HANDLE;

	if(channel > SOUND_MIXER_NRDEVICES)
		throw MIXERDEV_OUT_OF_RANGE;				// Mixer fault

	int mixer_device_mask, mixer_record_mask, mixer_stereo_mask;
	if((ioctl(mixDevice, SOUND_MIXER_READ_DEVMASK, & mixer_device_mask) == -1) || 
	   (ioctl(mixDevice, SOUND_MIXER_READ_RECMASK, & mixer_record_mask) == -1) ||
	   (ioctl(mixDevice, SOUND_MIXER_READ_STEREODEVS, & mixer_stereo_mask) == -1))
		throw errno;

	validChannel = ((1 << channel) & mixer_device_mask) != 0;
	canRecord = ((1 << channel) & mixer_record_mask) != 0;
	isStereo = ((1 << channel) & mixer_stereo_mask) != 0;

	mixerHandle = mixDevice;
	mixerChannel = channel;
}

const char * Channel::getChannelName()
{
	return labels[mixerChannel];
}

const bool Channel::canChannelRecord() const
{
	return canRecord;
}

const bool Channel::isChannelValid() const
{
	return validChannel;
}

const bool Channel::isChannelStereo() const
{
	return isStereo;
}

const bool Channel::isChannelInRecordState()
{
	int mixer_record_src;
	if(ioctl(mixerHandle, SOUND_MIXER_READ_RECSRC, & mixer_record_src) == -1)
		throw errno;

	return ((1 << mixerChannel) & mixer_record_src) != 0;
}

void Channel::setChannelRecordState(const bool state)
{
	int mixer_record_src = state ? (1 << mixerChannel) : 0;
	if(ioctl(mixerHandle, SOUND_MIXER_WRITE_RECSRC, & mixer_record_src) == -1)
		throw errno;
}
