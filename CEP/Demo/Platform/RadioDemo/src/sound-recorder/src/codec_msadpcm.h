// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _codec_msadpcm_h
#define _codec_msadpcm_h

#include "types.h"
#include <iostream.h>
#include <exception>
#include <string.h>
#include "codec.h"

struct msadpcmCoefs {
	int16_t			wCoef1;
	int16_t			wCoef2;     
};

struct MsAdpcmFormat {
	u_int16_t		wFormatTag;		// Format category
	u_int16_t		wChannels;		// Number of channels
	u_int32_t		dwSamplesPerSec;	// Sampling rate
	u_int32_t		dwAvgBytesPerSec;	// For buffer estimation
	u_int16_t		wBlockAlign;		// Data block size
	u_int16_t		bitsPerSample;		// Bits per sample
	u_int16_t		wCbSize;		// Size extended block
	u_int16_t		wSamplesPerBlock;	// Samples per block
	u_int16_t		wNumCoef;		// Number of coefficients
	struct msadpcmCoefs	aCoeff[7];		// Coefs (default 7 more (wNumCoef) if extra)
};

class CodecMsAdpcm : public Codec {

	public:
				CodecMsAdpcm(void * fmtInfo = 0);
				~CodecMsAdpcm();
				CodecMsAdpcm(const CodecMsAdpcm & codec);

		void		init(void * fmtInfo);
		void		init(void * fmtInfo, const chnl_t chnl, const bps_t bps, const smpl_t smpl);
		const int	encode(char * buffer, int & len, const int slen, FILE * fh);
		const int	decode(char * buffer, int & len, const int slen, FILE * fh);
		const int	guessLen(const int len);
		void		getDspSettings(chnl_t & chnl, bps_t & bps, smpl_t & smpl, int & bs);
		const int	flushEncodeBuffer(FILE * fh);
	
	private:
		const int8_t	calcPredictor(const int16_t * buffer, const int samples,
					const int channels);
		const int16_t	calcDelta(const int16_t coef1, const int16_t coef2,
					const int32_t s1, const int32_t s2, const int32_t s3, 
					const int32_t s4, const int32_t s5);
		const char	calcSample(const int channel);

	private:
		static int16_t	MsAdpcmP4[];
		static int16_t	MsAdpcmC7[];

		int16_t		samplesEncoded, samplesDecoded;
		u_int8_t	dPredictors[2], ePredictors[2];
		int16_t		dDelta[2], dSmpl1[2], dSmpl2[2];
		int16_t		eDelta[2], eSmpl1[2], eSmpl2[2];

		int8_t		encodeBuffer[1024];
		int8_t		decodeBuffer[4096];
		int8_t *	lastEiBuffer;
		int8_t *	lastDoBuffer;
		int16_t		si1;
};

#endif
