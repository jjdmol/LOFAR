// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "codec_imaadpcm.h"

// IMA step table
const int16_t CodecImaAdpcm::ImaStepTable[] = {
    7,     8,     9,    10,    11,    12,    13,    14,    16,    17,    19,    21,   23,    25,   28,   31,
   34,    37,    41,    45,    50,    55,    60,    66,    73,    80,    88,    97,  107,   118,  130,  143,
  157,   173,   190,   209,   230,   253,   279,   307,   337,   371,   408,   449,   494,  544,  598,  658,
  724,   796,   876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,  2272, 2499, 2749, 3024,
 3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,  7132,  7845,  8630,  9493, 10442,
11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 };

// IMA Index table for 5 bits samples
const int16_t CodecImaAdpcm::ImaIndex5[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16,
	-1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16};

// IMA Index table for 4 bits samples
const int16_t CodecImaAdpcm::ImaIndex4[] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8};

// IMA Index table for 3 bits samples
const int16_t CodecImaAdpcm::ImaIndex3[] = {
	-1, -1, 1, 2,
	-1, -1, 1, 2};

// IMA Index table for 2 bits samples
const int16_t CodecImaAdpcm::ImaIndex2[] = {
	-1, 2,
	-1, 2};


CodecImaAdpcm::CodecImaAdpcm(void * fmtInfo) : Codec(fmtInfo)
{
}

CodecImaAdpcm::~CodecImaAdpcm()
{
}

CodecImaAdpcm::CodecImaAdpcm(const CodecImaAdpcm & codec) : Codec(0)
{
	cerr << "Codec copy-constructor" << endl;
	exit(1);
}

void CodecImaAdpcm::init(void * fmtInfo)
{
	format = (void *) fmtInfo;
}

void CodecImaAdpcm::init(void * fmtInfo, const chnl_t chnl, const bps_t bps, const smpl_t smpl)
{
	// Use given format info
	format = fmtInfo;

	// Set infostructure to format-buffer
	struct ImaAdpcmFormat & ImaAdpcmFormat = *(struct ImaAdpcmFormat *) format;

	// Test if 3-5 bits per samples are used
	if(bps < 2 || bps > 5){
		cerr << "Error BitsPerSecond (" << bps << ") setting to 4." << endl;
		ImaAdpcmFormat.bitsPerSample = 4;
	} else {
		ImaAdpcmFormat.bitsPerSample = bps;
	}

	// Set format and channels to valid values
	ImaAdpcmFormat.wFormatTag = 0x11;
	ImaAdpcmFormat.wChannels = (chnl == 0 || chnl > IMA_MAXCHNL) ? 2 : chnl;

	// Test samplerate
	ImaAdpcmFormat.dwSamplesPerSec = (smpl == 0) ? 44100 : smpl;
	if(smpl != 11025 && smpl != 22050 && smpl != 44100)
		cerr << "Samplerate " << smpl << " not common!" << endl;

	// Calculate blockalign
	ImaAdpcmFormat.wBlockAlign = 256 * ImaAdpcmFormat.wChannels *
		(ImaAdpcmFormat.dwSamplesPerSec < 11025 ? 1 : ImaAdpcmFormat.dwSamplesPerSec / 11025);

	// Set extra infosize and fill coefs. in header
	ImaAdpcmFormat.wCbSize = 2;
	ImaAdpcmFormat.wSamplesPerBlock =  (8 * (ImaAdpcmFormat.wBlockAlign /
		chnl - sizeof(struct ImaAdpcm_blockheader_tag))) / bps + 1;

	// Calc. avg bps
	ImaAdpcmFormat.dwAvgBytesPerSec = (ImaAdpcmFormat.dwSamplesPerSec * ImaAdpcmFormat.wBlockAlign) /
		ImaAdpcmFormat.wSamplesPerBlock;
	
	encoded = decoded = 0;
	samplesEncoded = samplesDecoded = 0;

	for(int i = 0; i < 2; i++){
		prevESample[i] = 0;
		prevEIndex[i] = 0;
	}

	si1 = 0;
}

const int CodecImaAdpcm::flushEncodeBuffer(FILE * fh)
{
	int written = 0;

	if(samplesEncoded > 0){
		samplesEncoded = 0;
		eShift = 0;

		const struct ImaAdpcmFormat & fmt = * (struct ImaAdpcmFormat *) format;
		written = ::fwrite(encodeBuffer, 1, fmt.wBlockAlign, fh);
	}
	return written;
}

// Input MUST be 16 bits samples aligned in 'chnls'
const int CodecImaAdpcm::encode(char * buffer, int & len, const int slen, FILE * fh)
{
	int samples = slen;
	int32_t * oBuffer;
	int16_t * srcSamples = (int16_t *) buffer;
	const struct ImaAdpcmFormat & fmt = * (struct ImaAdpcmFormat *) format;

	const int smplSize = 2;

	// Number of channels in sourcedata
	chnl_t chnls = fmt.wChannels;
	if(chnls > 2)
		throw "More than 2 channels not implemented.";

	// Calculate number of blocks needed
	int spb = fmt.wSamplesPerBlock;

	// Test if 3-5 bits per samples are used
	const bps_t bps = fmt.bitsPerSample;

	// Header of data
	struct ImaAdpcm_blockheader_tag * headers = 
		(struct ImaAdpcm_blockheader_tag *) encodeBuffer;

	// Recall last position of outputbuffer
	if(samplesEncoded > 0){
		oBuffer = lastEoBuffer;
	} else {
		oBuffer = (int32_t *) ((char *) encodeBuffer + 
			chnls * sizeof(struct ImaAdpcm_blockheader_tag));
	}

	len = 0;
	// For all blocks in inputbuffer do....
	while(samples > 0){

		if(samplesEncoded == spb)
			len += flushEncodeBuffer(fh);

		if(samplesEncoded == 0){
			// Clear buffer
			memset(encodeBuffer, 0, sizeof(encodeBuffer));

			// Create and store headers
			for(int c = 0; c < chnls; c++){
				headers[c].iSamp0 = prevESample[c] = *(srcSamples++);
				headers[c].bStepTableIndex = prevEIndex[c];
				headers[c].bReserved = 0;
				samples -= smplSize;
			}
			samplesEncoded++;
			encoded++;

			oBuffer = (int32_t *) ((char *) encodeBuffer + 
				chnls * sizeof(struct ImaAdpcm_blockheader_tag));

			continue;
		}

		int32_t	index, sample;
		for(int c = 0; c < chnls; c++){
			index = prevEIndex[c];
			sample = prevESample[c];

			u_int32_t delta = 0;
			int32_t Diff = *(srcSamples++);
			Diff -= sample;
			samples -= smplSize;

			const bool sign = Diff < 0;
			if(sign == true) Diff = -Diff;

			int16_t step = ImaStepTable[index];

			int16_t vpDiff = 0;
			switch(bps){
				case 2:
					if(Diff >= (step >> 1)){
						delta |= 1;
						vpDiff += step;
					}

					if(sign == true)
						delta |= 2;

					index += ImaIndex2[delta];

					break;
				case 3:
					vpDiff = step >> 2;
					if(Diff >= step){
						delta |= 2;
						Diff -= step;
						vpDiff += step;
					}

					step >>= 1;
					if(Diff >= step){
						delta |= 1;
						vpDiff += step;
					}

					if(sign == true)
						delta |= 4;


					index += ImaIndex3[delta];
					break;
				case 4:
					vpDiff = step >> 3;
					if(Diff >= step){
						delta |= 4;
						Diff -= step;
						vpDiff += step;
					}

					step >>= 1;
					if(Diff >= step){
						delta |= 2;
						Diff -= step;
						vpDiff += step;
					}

					step >>= 1;
					if(Diff >= step){
						delta |= 1;
						vpDiff += step;
					}

					if(sign == true)
						delta |= 8;

					index += ImaIndex4[delta];

					break;
				case 5:
					vpDiff = step >> 4;
					if(Diff >= step){
						delta |= 8;
						Diff -= step;
						vpDiff += step;
					}

					step >>= 1;
					if(Diff >= step){
						delta |= 4;
						Diff -= step;
						vpDiff += step;
					}

					step >>= 1;
					if(Diff >= step){
						delta |= 2;
						Diff -= step;
						vpDiff += step;
					}

					step >>= 1;
					if(Diff >= step){
						delta |= 1;
						vpDiff += step;
					}

					if(sign == true)
						delta |= 0x10;

					index += ImaIndex5[delta];

					break;
			}

			// Correct sign
			if(sign == true){
				sample -= vpDiff;
			} else{
				sample += vpDiff;
			}

			// Clamp sample to 16 bits
			if(sample > 32767) sample = 32767;
			if(sample < -32768) sample = -32768;

			// Clamp index
			if(index > 88) index = 88;
			if(index < 0) index = 0;

			// Store sample
			prevEIndex[c] = index;
			prevESample[c] = (int16_t) sample;

			// Output delta
			eSample[c] |= (delta << eShift);
			if((32 - bps) <= eShift){
				oBuffer[c] = eSample[c];
				eSample[c] = 0L | (delta >> (32 - eShift));

				if(c == (chnls - 1))
					oBuffer += chnls;
			}
		}
		encoded++;
		samplesEncoded++;
		eShift = (eShift + bps) & 31;
	}

	if(samplesEncoded > 0){
		lastEoBuffer = oBuffer;
	}
	
	// Give a note of written length
	return len;
}

const int CodecImaAdpcm::decode(char * buffer, int & len, const int slen, FILE * fh)
{
	// Set structures/data to access format info
	const struct ImaAdpcmFormat & fmt = * (struct ImaAdpcmFormat *) format;
	const int bits = (fmt.bitsPerSample * 2 * (fmt.wChannels == 1 ? 1 : 2)) > 8 ? 16 : 8;

	int samples = 0;
	const int smplSize = bits / 8;
	u_int32_t * iBuffer;

	// Test if 3-5 bits per samples are used
	const bps_t bps = fmt.bitsPerSample;
	if(bps < 2 || bps > 5)
		throw "Only know 2, 3, 4 and 5 bits IMAADPCM.";

	// Test if destinationbuffer is big enough
	chnl_t chnls = fmt.wChannels;
	int spb = fmt.wSamplesPerBlock;

	// Set header to new sourcebuffer
	struct ImaAdpcm_blockheader_tag * headers = (struct ImaAdpcm_blockheader_tag *) decodeBuffer;

	iBuffer = (u_int32_t *) ((char *) decodeBuffer + 
		chnls * sizeof(struct ImaAdpcm_blockheader_tag));

	while(samples < len){
		
		if(samplesDecoded == spb){
			samplesDecoded = 0;
		}

		if(samplesDecoded == 0){
			if((::fread(decodeBuffer, 1, fmt.wBlockAlign, fh)) == 0)
				return samples;

			iBuffer = (u_int32_t *) ((char *) decodeBuffer + 
				chnls * sizeof(struct ImaAdpcm_blockheader_tag));

			dShift = 0;

			// Initialize shift and si1 (source-index)
			si1 = 0;

			// Output sample that is stored in header
			for(int c = 0; c < chnls; c++){
				decoded++;
				int16_t smpl = headers[c].iSamp0;
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

		// Next set of variables and start of block
		int16_t smpl_word = 0;

		for(int c = 0; c < chnls; c++){

			// Store sample and index (previous)
			int16_t index = headers[c].bStepTableIndex;
			int32_t sample = headers[c].iSamp0;

			// If overflow occures adjust shift and index
			// to next block
			if(dShift >= 32){
				dShift &= 31;
				si1 += chnls;
			}

			// Store next 32 bits sample
			smpl_word = iBuffer[si1 + c] >> dShift;

			// Initialise Diff and step
			int32_t Diff = 0;
			int step = ImaStepTable[index];

			// Calculate difference according to bitsPerSample
			switch(bps){
				case 2:
					Diff = step >> 1;
					if(smpl_word & 1) Diff += step;
					if(smpl_word & 2) Diff = -Diff;
					index += ImaIndex2[smpl_word & 0x3];
					break;
				case 3:
					if(dShift > 29){
						int e_shift = 32 - dShift;
						u_int8_t e_nibble = iBuffer[si1 + (c + chnls)] << e_shift;
						smpl_word |= e_nibble;
					}

					Diff = step >> 2;
					if(smpl_word & 2) Diff += step;
					if(smpl_word & 1) Diff += step >> 1;
					if(smpl_word & 4) Diff = -Diff;

					index += ImaIndex3[smpl_word & 0x7];
					break;
				case 4:
					Diff = step >> 3;
					if(smpl_word & 4) Diff += step;
					if(smpl_word & 2) Diff += step >> 1;
					if(smpl_word & 1) Diff += step >> 2;
					if(smpl_word & 8) Diff = -Diff;

					index += ImaIndex4[smpl_word & 0xf];
					break;
				case 5:
					if(dShift > 27){
						int e_shift = 32 - dShift;
						u_int8_t e_nibble = iBuffer[si1 + (c + chnls)] << e_shift;
						smpl_word |= e_nibble;
					}

					Diff = step >> 4;
					if(smpl_word & 8) Diff += step;
					if(smpl_word & 4) Diff += step >> 1;
					if(smpl_word & 2) Diff += step >> 2;
					if(smpl_word & 1) Diff += step >> 3;
					if(smpl_word & 0x10) Diff = -Diff;
					index += ImaIndex5[smpl_word & 0x1f];
					break;
			}

			// Calculate new sample
			sample += Diff;

			// Clamp sample to 16 bits
			if(sample > 32767) sample = 32767;
			if(sample < -32768) sample = -32768;

			// Output sample
			decoded++;
			if(bits == 8){
				(*buffer++) = (((int16_t) sample >> 8) + 128);
			} else {
				*((int16_t *) buffer) = (int16_t) sample;
				decoded++;
				buffer += 2;
			}

			// Clamp index
			if(index > 88) index = 88;
			if(index < 0) index = 0;

			// Shift current to previous sample
			headers[c].iSamp0 = (int16_t) sample;
			headers[c].bStepTableIndex = index;
			samples += smplSize;
		}
		// Adjust bit-shift to new sample
		dShift += bps;
		samplesDecoded++;
	}

	return samples;
}

const int CodecImaAdpcm::guessLen(const int len)
{
	return ((struct ImaAdpcmFormat *) format)->wBlockAlign;
}

void CodecImaAdpcm::getDspSettings(chnl_t & chnl, bps_t & bps, smpl_t & smpl, int & bs)
{
	const struct ImaAdpcmFormat & fmt = * (struct ImaAdpcmFormat *) format;
	chnl = fmt.wChannels;
	bps = fmt.bitsPerSample * 2 * (chnl == 1 ? 1 : 2);
	bps = (bps > 8) ? 16 : 8;
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
	bs = 256 * chnl * ((smpl > 11025) ? smpl / 11025 : 1);

/*
	cerr << "Channels: " << chnl << endl <<
		"Bit p/s\t: " << bps << endl <<
		"Samples\t: " << smpl << endl <<
		"Bufsize\t: " << bs << endl <<
		"Align\t: " << fmt.wBlockAlign << endl; */
}

