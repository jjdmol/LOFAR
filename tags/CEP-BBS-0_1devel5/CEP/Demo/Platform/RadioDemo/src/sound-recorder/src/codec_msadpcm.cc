// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "codec_msadpcm.h"

int16_t CodecMsAdpcm::MsAdpcmP4[] = {230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512,
	409, 307, 230, 230, 230};

int16_t CodecMsAdpcm::MsAdpcmC7[] = {256, 0, 512, -256, 0, 0, 192, 64, 240, 0, 460, -208, 392, -232};

CodecMsAdpcm::CodecMsAdpcm(void * fmtInfo) : Codec(fmtInfo)
{
}

CodecMsAdpcm::~CodecMsAdpcm()
{
}

CodecMsAdpcm::CodecMsAdpcm(const CodecMsAdpcm & codec) : Codec(0)
{
	cerr << "Codec copy-constructor" << endl;
	exit(1);
}

void CodecMsAdpcm::init(void * fmtInfo)
{
	format = fmtInfo;
}

void CodecMsAdpcm::init(void * fmtInfo, const chnl_t chnl, const bps_t bps, const smpl_t smpl)
{
	// Test if format is valid
	format = fmtInfo;

	// Set infostructure to format-buffer
	struct MsAdpcmFormat & MsAdpcmFormat = *(struct MsAdpcmFormat *) format;

	// Test if BitsPerSample is valid
	if(bps != 4){
		cerr << "Error BitsPerSecond (" << bps << ") setting to 4." << endl;
		MsAdpcmFormat.bitsPerSample = 4;			// bps == 4
	}

	// Set format and channels to valid values
	MsAdpcmFormat.wFormatTag = 2;
	MsAdpcmFormat.wChannels = (chnl == 0 || chnl > 2) ? 2 : chnl;

	// Test samplerate
	MsAdpcmFormat.dwSamplesPerSec = (smpl == 0) ? 44100 : smpl;
	if(smpl != 11025 && smpl != 22050 && smpl != 44100)
		cerr << "Samplerate " << smpl << " not common." << endl;

	// Calculate blockalign
	switch(MsAdpcmFormat.wChannels * MsAdpcmFormat.dwSamplesPerSec){
		case 8000:
		case 11025:
			MsAdpcmFormat.wBlockAlign = 256;
			break;
		case 22050:
			MsAdpcmFormat.wBlockAlign = 512;
			break;
		case 44100:
			MsAdpcmFormat.wBlockAlign = 1024;
			break;
		case 88200:
			MsAdpcmFormat.wBlockAlign = 2048;
			break;
		default:
			cerr << "Chnl * smpl = " << 
				MsAdpcmFormat.wChannels * MsAdpcmFormat.dwSamplesPerSec << endl;
			throw "Illegal value chnl*smpl (msadpcm)";
	}

	// Set extra infosize
	MsAdpcmFormat.wCbSize = 32;
	MsAdpcmFormat.wSamplesPerBlock = (((MsAdpcmFormat.wBlockAlign - (7 * MsAdpcmFormat.wChannels)) * 8) /
		(4 * MsAdpcmFormat.wChannels)) + 2; // bps == 4

	// Calc. samples and avg bps
	MsAdpcmFormat.dwAvgBytesPerSec = (MsAdpcmFormat.dwSamplesPerSec * 4) / 8;	// bps == 4

	// Set coefs. in header
	MsAdpcmFormat.wNumCoef = 7;
	memcpy(MsAdpcmFormat.aCoeff, MsAdpcmC7, sizeof(MsAdpcmC7));

	encoded = decoded = 0;
	samplesEncoded = samplesDecoded = 0;

	si1 = 0;
}

const int16_t CodecMsAdpcm::calcDelta(const int16_t coef1, const int16_t coef2,
	const int32_t s1, const int32_t s2, const int32_t s3, 
	const int32_t s4, const int32_t s5)
{
	int32_t ttlPred, ttlTemp;
	int16_t delta;

	ttlTemp = ((s5 * coef1) + (s4 * coef1)) >> 8;
	ttlPred = abs(s3 - ttlTemp);

	ttlTemp = ((s4 * coef1) + (s3 * coef1)) >> 8;
	ttlPred += abs(s2 - ttlTemp);

	ttlTemp = ((s3 * coef1) + (s2 * coef1)) >> 8;
	ttlPred += abs(s1 - ttlTemp);

	delta = ttlPred / 12;
	if(delta < 16) delta = 16;		// Minimal delta value

	return delta;
}

const char CodecMsAdpcm::calcSample(const int channel)
{
	/*
	const struct MsAdpcmFormat & fmt = * (struct MsAdpcmFormat *) format;

	int16_t samp1 = eSmpl1[channel];
	int16_t samp2 = eSmpl2[channel];
	int16_t delta = eDelta[channel];

	int32_t pred = (1L * samp1 * iCoef1 + 1L * iSamp2 * iCoef2) >> 8;

	//
	//  encode it
	//
	int32_t lError = (1L * sample) - pred;
	int16_t iOutput = (lError / delta);

	if (iOutput > 7) iOutput = 7;
	else if (iOutput < -8) iOutput = -8;

	lSamp = pred + (1L * delta * iOutput);

	if (lSamp > 32767)
		lSamp = 32767;
	else if (lSamp < -32768)
		lSamp = -32768;

	//
	//  compute the next iDelta
	//
	iDelta = (short)((gaiP4[iOutput & 15] * (long)iDelta) >> MSADPCM_PSCALE);
	if (iDelta < MSADPCM_DELTA4_MIN)
		iDelta = MSADPCM_DELTA4_MIN;

	//
	//  save updated values for this channel back into the
	//  original arrays...
	//
	aiDelta[m] = iDelta;
	aiSamp2[m] = iSamp1;
	aiSamp1[m] = (short)lSamp;

	//
	//  keep a running status on the error for the current
	//  coefficient pair for this channel
	//
	lError = lSamp - iSample;
	adwTotalError[i][m] += (lError * lError) >> 7;
	*/
	return 0;
}

const int8_t CodecMsAdpcm::calcPredictor(const int16_t * buffer, 
	const int samples, const int channels)
{
	const struct MsAdpcmFormat & fmt = * (struct MsAdpcmFormat *) format;
	int16_t deltaStore[7*2];
	
//	const int bits = 16;			// hardcoded input 16bits per sample

	for(int c = 0; c < channels; c++){
		eSmpl1[c] = buffer[c + channels];
		eSmpl2[c] = buffer[c];
	}

	for(int ci = 0; ci < 7; ci++){
		const int16_t coef1 = fmt.aCoeff[ci].wCoef1;
		const int16_t coef2 = fmt.aCoeff[ci].wCoef2;

		// calc initial delta
		for(int c = 0; c < channels; c++){
			eDelta[c] = calcDelta(coef1, coef2,
				buffer[               c], 
				buffer[    channels + c],
				buffer[2 * channels + c],
				buffer[3 * channels + c],
				buffer[4 * channels + c]);

			// keep values to restore the ones calc. with the best
			// predicition
			deltaStore[2 * ci + c] = eDelta[c];

			cerr << "Delta on channel " << channels << ": " <<
				eDelta[c] << endl;

			calcSample(c);
		}
	}
	
	// find best predictor

	// restore best delta with this predictor

	return 0;
}

// Buffer MUST contain 16 bit samples
const int CodecMsAdpcm::encode(char * buffer, int & len, const int slen, FILE * fh)
{
	int samples = slen;
	int8_t * iBuffer;
	int16_t * srcSamples = (int16_t *) buffer;
	const struct MsAdpcmFormat & fmt = * (struct MsAdpcmFormat *) format;

	const int smplSize = 2;

	// Number of channels in sourcedata
	chnl_t chnls = fmt.wChannels;
	if(chnls > 2)
		throw "More than 2 channels not implemented.";

	// Calculate number of blocks needed
	int spb = fmt.wSamplesPerBlock;

	// Recall last position of outputbuffer
	if(samplesEncoded > 0){
		iBuffer = lastEiBuffer;
	} else {
		iBuffer = (int8_t *) ((char *) encodeBuffer + chnls * 7);
	}

	len = 0;
	// For all blocks in inputbuffer do....
	while(samples > 0){

		if(samplesEncoded == spb)
			len += flushEncodeBuffer(fh);

		if(samplesEncoded == 0){
			// Clear buffer
			memset(encodeBuffer, 0, sizeof(encodeBuffer));

			calcPredictor(srcSamples, samples, chnls);

			// Create and store headers
			for(int c = 0; c < chnls; c++){
				// ePredictors[c] calculated with calcPredictor
				// eDelta[c] result of the calc. predictor
				eSmpl1[c] = srcSamples[c];
				samples -= smplSize;

				if(samples <= 0){
					throw "Buffer undersize while reading header.";
				}
			
				eSmpl2[c] = srcSamples[c + chnls];
				samples -= smplSize;
			}
			samplesEncoded++;
			encoded++;

			iBuffer = (int8_t *) ((char *) encodeBuffer + chnls * 7);

			continue;
		}
		samples -= smplSize;
	}

	if(samplesEncoded > 0){
		lastEiBuffer = iBuffer;
	}
	
	// Give a note of written length
	return len;
}

const int CodecMsAdpcm::decode(char * buffer, int & len, const int slen, FILE * fh)
{
	// Set structures/data to access format info
	const struct MsAdpcmFormat & fmt = * (struct MsAdpcmFormat *) format;
	const int bits = fmt.bitsPerSample * 2 * (fmt.wChannels == 1 ? 1 : 2);
	struct strCoefs {int16_t iCoef1; int16_t iCoef2;};

	// Test if 4 bits per samples are used
	const bps_t bps = fmt.bitsPerSample;
	if(bps != 4)
		throw "Only know 4bits MSADPCM.";

	int samples = 0;
	const int smplSize = bits / 8;
	int8_t * iBuffer;

	chnl_t chnls = fmt.wChannels;
	int spb = fmt.wSamplesPerBlock;

	iBuffer = (int8_t *) ((char *) decodeBuffer + chnls * 7); // 7 bytes in a header

	// Next set of variables and start of block
	int16_t nibble0 = 0;
	int16_t nibble1 = 0;
	bool grabbed = false;

	while(samples < len){
		
		if(samplesDecoded == spb){
			samplesDecoded = 0;
		}

		if(samplesDecoded == 0){
			if((::fread(decodeBuffer, 1, fmt.wBlockAlign, fh)) == 0)
				return samples;

			// Initialize all variables needed in decoding
			// Copying necessary because alignment conflict on
			// 64 bit architectures and mis-align errors
			// (DEC-Alpha) -> compiler doesn't allow casting!
			memcpy(dPredictors, decodeBuffer, chnls);
			memcpy(dDelta, decodeBuffer + chnls, 2 * chnls);
			memcpy(dSmpl1, decodeBuffer + 3 * chnls, 2 * chnls);
			memcpy(dSmpl2, decodeBuffer + 5 * chnls, 2 * chnls);

			// Check predictor values
			for(int c = 0; c < chnls; c++){
				if(dPredictors[c] >= fmt.wNumCoef){
					cerr << "Predictor index out of range" << 
						dPredictors[c] << " " << fmt.wNumCoef << endl;

					throw "Predictor out of range (msadpcm)";
				}
			}

			iBuffer = (int8_t *) ((char *) decodeBuffer + chnls * 7); // 7 bytes in a header

			// Initialize shift and si1 (source-index)
			si1 = 0;
			grabbed = false;

			// Output sample that is stored in header
			for(int c = 0; c < chnls; c++){
				decoded++;
				int16_t smpl = dSmpl1[c];
				if(bits == 8){
					(*buffer++) = ((smpl >> 8) + 128);
				} else {
					*((int16_t *) buffer) = smpl;
					decoded++;
					buffer += 2;
				}
				samples += smplSize;
			}
			samplesDecoded++;

			if(samples >= len){
				throw "Buffer undersize while reading header.";
			}
			
			for(int c = 0; c < chnls; c++){
				decoded++;
				int16_t smpl = dSmpl2[c];
				if(bits == 8){
					(*buffer++) = ((smpl >> 8) + 128);
				} else {
					*((int16_t *) buffer) = smpl;
					decoded++;
					buffer += 2;
				}
				samples += smplSize;
			}
			samplesDecoded++;
			continue;
		}

		for(int c = 0; c < chnls; c++){

			// Read 2 nibbles at once
			if(grabbed == false){
				int16_t input = iBuffer[si1++];
				nibble0 = input >> 4;
				nibble1 = input;
			} else {
				nibble0 = nibble1;
			}
			grabbed ^= true;

			// Sign extend
			nibble0 &= 0xf;
			if((nibble0 & 8) == 8 && nibble0 > 0)
				nibble0 = ((signed char) (nibble0 | 0xf0));

			// Calculate delta
			int16_t d = dDelta[c];
			dDelta[c] = (int16_t) ((MsAdpcmP4[nibble0 & 0xf] * d) >> 8);

			// Allow >= 16
			if(dDelta[c] < 16)
				dDelta[c] = 16;

			// Calc prediction for next sample
			const struct msadpcmCoefs * coef = &fmt.aCoeff[dPredictors[c]];
			int32_t pred = (dSmpl1[c] * coef->wCoef1 + dSmpl2[c] * coef->wCoef2) >> 8;

			int32_t sample = (1L * nibble0 * d) + pred;
			if(sample > 32767) sample = 32767;
			if(sample < -32768) sample = -32768;

			// Output
			decoded++;
			if(bits == 8){
				(*buffer++) = (((int16_t) sample >> 8) + 128);
			} else {
				*((int16_t *) buffer) = (int16_t) sample;
				decoded++;
				buffer += 2;
			}
			samples += smplSize;

			// Shift current and previous sample
			dSmpl2[c] = dSmpl1[c];
			dSmpl1[c] = (int16_t) sample;
		}
		// Adjust bit-shift to new sample
		samplesDecoded++;
	}
	
	return samples;
}

const int CodecMsAdpcm::flushEncodeBuffer(FILE * fh)
{
	int written = 0;

	if(samplesEncoded > 0){
		samplesEncoded = 0;

		const struct MsAdpcmFormat & fmt = * (struct MsAdpcmFormat *) format;
		written = ::fwrite(encodeBuffer, 1, fmt.wBlockAlign, fh);
	}
	return written;
}

const int CodecMsAdpcm::guessLen(const int len)
{
	struct MsAdpcmFormat & fmt = * (struct MsAdpcmFormat *) format;

	return fmt.wBlockAlign;
}

void CodecMsAdpcm::getDspSettings(chnl_t & chnl, bps_t & bps, smpl_t & smpl, int & bs)
{
	const struct MsAdpcmFormat & fmt = * (struct MsAdpcmFormat *) format;
	chnl = fmt.wChannels;
	bps  = fmt.bitsPerSample * 2 * (fmt.wChannels == 1 ? 1 : 2);
	switch(fmt.wBlockAlign / chnl){
		case 256:
			smpl = 11025;		// 8k possible (when??)
		case 512:
			smpl = 22050;
			break;
		case 1024:
			smpl = 44100;
			break;
		default:
			throw "Illegal blockalign.";
	}
	bs = ((fmt.dwAvgBytesPerSec / fmt.wSamplesPerBlock) * fmt.wBlockAlign);

/*
	cerr << "Channels: " << chnl << endl <<
		"Bit p/s\t: " << bps << endl <<
		"Samples\t: " << smpl << endl <<
		"Bufsize\t: " << bs << endl <<
		"Align\t: " << fmt.wBlockAlign << endl; */
}

