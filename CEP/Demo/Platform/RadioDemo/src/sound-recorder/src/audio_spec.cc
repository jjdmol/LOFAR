// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "audio_spec.h"

AudioSpec::AudioSpec()
{
	channels = 0;
	bitsPerSample = 0;
	samplePerSec = 0;
}

AudioSpec::AudioSpec(const AudioSpec & audioSpec)
{
	channels = audioSpec.channels;
	bitsPerSample = audioSpec.bitsPerSample;
	samplePerSec = audioSpec.samplePerSec;
}

AudioSpec::AudioSpec(const chnl_t chnl, const bps_t bps, const smpl_t smpl)
{
	channels = chnl;
	bitsPerSample = bps;
	samplePerSec = smpl;
}

AudioSpec::~AudioSpec()
{
}

