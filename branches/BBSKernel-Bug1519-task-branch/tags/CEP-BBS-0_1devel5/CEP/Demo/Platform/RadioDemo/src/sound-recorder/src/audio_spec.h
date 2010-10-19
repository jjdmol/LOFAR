// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _audio_spec_h
#define _audio_spec_h

#include "types.h"

class AudioSpec
{
	public:
				AudioSpec();
				AudioSpec(const AudioSpec & audioSpec);
				AudioSpec(const chnl_t chnl, const bps_t bps,
						const smpl_t smpl);
				~AudioSpec();
	private:
		chnl_t		channels;
		bps_t		bitsPerSample;
		smpl_t		samplePerSec;
};

#endif
