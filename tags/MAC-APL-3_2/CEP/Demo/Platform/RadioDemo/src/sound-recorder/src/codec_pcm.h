// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _codec_pcm_h
#define _codec_pcm_h

#include "types.h"
#include <iostream.h>
#include <exception>
#include "codec.h"

struct PcmFormat {
	u_int16_t			wFormatTag;		// Format category
	u_int16_t			wChannels;		// Number of channels
	u_int32_t			dwSamplesPerSec;	// Sampling rate
	u_int32_t			dwAvgBytesPerSec;	// For buffer estimation
	u_int16_t			wBlockAlign;		// Data block size
	u_int16_t			bitsPerSample;		// Bits per sample
};


class CodecPcm : public Codec {

	public:
				CodecPcm(void * fmtInfo = 0);
				~CodecPcm();
				CodecPcm(const CodecPcm & codec);

		void		init(void * fmtInfo);
		void		init(void * fmtInfo, const chnl_t chnl, const bps_t bps, const smpl_t smpl);
		const int	encode(char * buffer, int & len, const int slen, FILE * fh);
		const int	decode(char * buffer, int & len, const int slen, FILE * fh);
		const int	guessLen(const int len);
		void		getDspSettings(chnl_t & chnl, bps_t & bps, smpl_t & smpl, int & bs);
		const int	flushEncodeBuffer(FILE * fh) {return 0;}
};

#endif
