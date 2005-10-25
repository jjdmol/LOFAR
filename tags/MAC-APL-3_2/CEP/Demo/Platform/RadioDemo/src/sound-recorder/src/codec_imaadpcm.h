// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _codec_imaadpcm_h
#define _codec_imaadpcm_h

#include "types.h"
#include <iostream.h>
#include <exception>
#include <string.h>
#include "codec.h"

#define  IMA_MAXCHNL 2

struct ImaAdpcmFormat {
	u_int16_t		wFormatTag;		// Format category
	u_int16_t		wChannels;		// Number of channels
	u_int32_t		dwSamplesPerSec;	// Sampling rate
	u_int32_t		dwAvgBytesPerSec;	// For buffer estimation
	u_int16_t		wBlockAlign;		// Data block size
	u_int16_t		bitsPerSample;		// Bits per sample
	u_int16_t		wCbSize;		// Size extended block
	u_int16_t		wSamplesPerBlock;	// Samples per block
};

struct ImaAdpcm_blockheader_tag {
	int16_t			iSamp0;
	int8_t			bStepTableIndex;
	int8_t			bReserved;
};


class CodecImaAdpcm : public Codec {

	public:
				CodecImaAdpcm(void * fmtInfo = 0);
				~CodecImaAdpcm();
				CodecImaAdpcm(const CodecImaAdpcm & codec);

		void		init(void * fmtInfo);
		void		init(void * fmtInfo, const chnl_t chnl, const bps_t bps, const smpl_t smpl);
		const int	encode(char * buffer, int & len, const int slen, FILE * fh);
		const int	decode(char * buffer, int & len, const int slen, FILE * fh);
		const int	guessLen(const int len);
		void		getDspSettings(chnl_t & chnl, bps_t & bps, smpl_t & smpl, int & bs);
		const int	flushEncodeBuffer(FILE * fh);
	
	private:
		const static int16_t	ImaStepTable[];
		const static int16_t	ImaIndex5[];
		const static int16_t	ImaIndex4[];
		const static int16_t	ImaIndex3[];
		const static int16_t	ImaIndex2[];

	private:
		int16_t			samplesEncoded, samplesDecoded;
		int16_t			prevESample[2], prevEIndex[2];

		u_int32_t		encodeBuffer[256];
		u_int32_t		decodeBuffer[1024];
		int32_t *		lastEoBuffer;
		int32_t *		lastDiBuffer;
		int16_t			si1;

		// Storeage for previous samples;
		int32_t			eSample[2], dSample[2];
		int16_t			eShift, dShift;

		struct ImaAdpcm_blockheader_tag	encodeHeader[IMA_MAXCHNL];
		struct ImaAdpcm_blockheader_tag	decodeHeader[IMA_MAXCHNL];
};

#endif
