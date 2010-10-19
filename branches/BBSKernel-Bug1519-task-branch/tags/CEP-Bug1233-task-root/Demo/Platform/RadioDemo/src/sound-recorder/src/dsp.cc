// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "dsp.h"


// ----------------------------------------------------------------------

Dsp::Dsp(DspSetting & setting) : dspSetting(setting)
{
	dspHandle = 0;
}


Dsp::~Dsp()
{
	close();
}


bool Dsp::init()
{
	bool success = false;
	if(dspHandle <= 0){
		if((dspHandle = ::open(dspSetting.getDeviceName(), dspSetting.getRWMode(), 0)) != -1){

			if( setSetting(SNDCTL_DSP_SETFMT, dspSetting.getFormat()) ||
			    setSetting(SNDCTL_DSP_STEREO, dspSetting.getChannels() - 1) ||
			    setSetting(SNDCTL_DSP_SPEED,  dspSetting.getSampleRate())){

				cerr << "Error intialising sound settings." << endl;
			} else {
				success = true;
			}
		} else {
			cerr << "Error opening device: " << dspSetting.getDeviceName() << endl;
		}
	} else {
		cerr << "Initializing a valid object." << endl;
	}

	return success;
}

const u_int32_t Dsp::getOptimalBufferSize()
{
	int val = 0;
	audio_buf_info abi;

	if(ioctl(dspHandle, dspSetting.getRWMode() == O_RDONLY ? SNDCTL_DSP_GETISPACE : 
								 SNDCTL_DSP_GETOSPACE, &abi) == -1){
		cerr << "Error in getting fragmentsize" << endl;
	} else {
		val = abi.fragstotal * abi.fragsize;
	}

	return val;
}

bool Dsp::setSetting(int setting, int value)
{
	int  copy = value;
	bool retError = 0;

	if(ioctl(dspHandle, setting, &value) == -1){
		cerr << "Error setting value " << value;
		retError = 1;
	} else {
		if(value != copy){
			cerr << "Value " << copy << " was set to " << value;
			retError = 1;
		}
	}

	if(retError)
		cerr << " on functioncode " << setting << endl;

	return retError;
}

void Dsp::close()
{
	if(dspHandle > 0){
		setSetting(SNDCTL_DSP_SYNC, 0);
		::close(dspHandle);
	}
}

int  Dsp::read(void * buffer, u_int32_t size)
{
	return (dspHandle > 0) ? ::read(dspHandle, buffer, size) : 0;
}

int  Dsp::write(void * buffer, u_int32_t size)
{
	return (dspHandle > 0) ? ::write(dspHandle, buffer, size) : 0;
}
