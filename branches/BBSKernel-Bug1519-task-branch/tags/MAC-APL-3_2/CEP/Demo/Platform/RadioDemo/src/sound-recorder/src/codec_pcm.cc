// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "codec_pcm.h"

CodecPcm::CodecPcm(void * fmtInfo) : Codec(fmtInfo)
{
}

CodecPcm::~CodecPcm()
{
}

CodecPcm::CodecPcm(const CodecPcm & codec) : Codec(0)
{
	cerr << "Codec copy-constructor" << endl;
	exit(1);
}

void CodecPcm::init(void * fmtInfo)
{
	format = (void *) fmtInfo;
}

void CodecPcm::init(void * fmtInfo, const chnl_t chnl, const bps_t bps, const smpl_t smpl)
{
	format = fmtInfo;
	
	struct PcmFormat & pcmFormat = *(struct PcmFormat *) format;
	pcmFormat.wFormatTag = 1;
	pcmFormat.wChannels = (chnl == 0) ? 2 : chnl;
	pcmFormat.dwSamplesPerSec  = (smpl == 0) ? 44100 : smpl;
	pcmFormat.bitsPerSample = (bps == 0) ? 16 : bps;
	pcmFormat.dwAvgBytesPerSec = (pcmFormat.wChannels * pcmFormat.bitsPerSample * pcmFormat.dwSamplesPerSec) / 8;
	pcmFormat.wBlockAlign = (pcmFormat.wChannels * pcmFormat.bitsPerSample) / 8;
}


const int CodecPcm::encode(char * buffer, int & len, const int slen, FILE * fh)
{
	const struct PcmFormat & fmt = * (struct PcmFormat *) format;
	encoded += len / fmt.wChannels;
	return ::fwrite(buffer, 1, len, fh);
}

const int CodecPcm::decode(char * buffer, int & len, const int slen, FILE * fh)
{
	const struct PcmFormat & fmt = * (struct PcmFormat *) format;
	decoded += len / fmt.wChannels;
	return ::fread(buffer, 1, len, fh);
}

const int CodecPcm::guessLen(const int len)
{
	return len;
}

void CodecPcm::getDspSettings(chnl_t & chnl, bps_t & bps, smpl_t & smpl, int & bs)
{
	const struct PcmFormat & fmt = * (struct PcmFormat *) format;
	chnl = fmt.wChannels;
	bps  = fmt.bitsPerSample;
	smpl = fmt.dwSamplesPerSec;
	bs   = fmt.dwAvgBytesPerSec;
}

