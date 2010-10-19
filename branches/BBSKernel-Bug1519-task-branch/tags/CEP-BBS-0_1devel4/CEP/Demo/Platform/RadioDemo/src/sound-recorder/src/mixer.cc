// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "mixer.h"

Mixer::Mixer()
{
	mixer_fd = 0;
	mixerDeviceName = "";
}

Mixer::~Mixer()
{
	close();
}

const string & Mixer::readCurrentSetting()
{
	static string setting;

	return setting;
}

void Mixer::bindMixerToDevice(const char * deviceName)
{
	mixerDeviceName = deviceName;
	if((mixer_fd = open(deviceName, O_RDWR)) == -1)
		throw errno;

	for(int i = 0; i < SOUND_MIXER_NRDEVICES; i++){
		channels[i].bindChannelToMixerDevice(mixer_fd, i);
	}
}

void Mixer::bindMixerToDevice(const int deviceNumber)
{
	string mixer;

	if(deviceNumber == 0)
		mixer = "/dev/mixer";

	if(deviceNumber == 1)
		mixer = "/dev/mixer1";

	bindMixerToDevice(mixer.c_str());
}

void Mixer::storeCurrentSetting(const string & setting)
{
}

const int Mixer::findDeviceId(const char * deviceName)
{
	int id = -1;
	for(int i = 0; i < SOUND_MIXER_NRDEVICES; i++){
		if(!strncmp(deviceName, channels[i].getChannelName(), strlen(deviceName))){
			id = i;
			break;
		}
	}

	return id;
}

Channel & Mixer::getChannel(const char * deviceName)
{
	int id = findDeviceId(deviceName);
	return channels[id];
}

Channel & Mixer::getChannel(const int deviceNumber)
{
	if(deviceNumber >= SOUND_MIXER_NRDEVICES)
		throw CHANNEL_ILLEGAL_MIXER_HANDLE;

	return channels[deviceNumber];
}

void Mixer::close()
{
	if(mixer_fd > 0)
		mixer_fd = ::close(mixer_fd);
}
