// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "waveriff.h"


TChunk::TChunk(const u_int8_t Id[4] = 0, const u_int32_t size = 0)
{
	chunkData = 0;
	allocate(Id, size);
}

TChunk::~TChunk()
{
	if(chunkData != 0)
		delete [] chunkData;
}

TChunk::TChunk(const TChunk & chunk)
{
	chunkData = 0;
	allocate(chunk.chunkId, chunk.chunkSize);
	if(chunkData != 0)
		memcpy(chunkData, chunk.chunkData, chunkSize);
}

// Allocate only if id != data
void TChunk::allocate(const u_int8_t id[4], const u_int32_t size)
{
	memcpy(chunkId, id, sizeof(chunkId));
	chunkSize = size;

	if(strncmp((const char *) id, "data", 4) != 0){
		if(chunkData != 0)
			delete [] chunkData;

		chunkData = new char[size];
	} else {
		chunkData = 0;
	}

	idNull[0] = idNull[1] = '\0';
}

char * TChunk::getAllocatedBuffer() const
{
	return chunkData;
}

const u_int8_t * TChunk::getId() const
{
	return chunkId;
}

const u_int32_t TChunk::getSize() const
{
	return chunkSize;
}

/* Init the filehandle to 0 to use as test */
TWave::TWave()
{
	fh = 0;
	codec = 0;
}

/* Close to be sure */
TWave::~TWave()
{
	close();
}

/* Create a wav structure with given arguments */
void TWave::createWave(const wavfmt_t fmt, const chnl_t chnl, const bps_t bps, const smpl_t smpl)
{
	memcpy(riffHeader.riffChunk.chunkId, "RIFF", 4);
	riffHeader.riffChunk.chunkSize = 0;		// known afterwards (record mode)
	memcpy(riffHeader.Signature, "WAVE", 4);

	u_int32_t fmtSize = CodecPcm().defaultFormatSize(fmt);
	TChunk * fmtChunk = new TChunk((const unsigned char *) "fmt ", fmtSize);
	*(u_int16_t *) fmtChunk->getAllocatedBuffer() = fmt;
	chunkList.push_back(* fmtChunk);

	assignCodec();
	codec->init(chunkList.begin()->getAllocatedBuffer(), chnl, bps, smpl);

	TChunk * factChunk = new TChunk((const unsigned char *) "fact", 4);
	chunkList.push_back(* factChunk);

	TChunk * dataChunk = new TChunk((const unsigned char *) "data", 0);
	chunkList.push_back(* dataChunk);
}

/* Create a wav file with the given arguments */
const bool TWave::create(const char * filename, 
		   const wavfmt_t fmt,
		   const chnl_t chnl,
		   const bps_t bps,
		   const smpl_t smpl)
{
	if(fh == 0){
		if((fh = ::fopen(filename, "w+b")) == 0){
			perror("Wave create error");
		} else {
			mode = O_WRONLY;
			createWave(fmt, chnl, bps, smpl);
			::fwrite((char *) & riffHeader, 1, sizeof(riffHeader), fh);

			writeChunks();
		}
	}
	return fh != 0;
}

void TWave::assignCodec()
{
	TChunk & chnk = * chunkList.begin();
	if(strncmp((const char *) chnk.getId(), "fmt ", 4) == 0){
		u_int16_t * fmt = (u_int16_t *) chnk.getAllocatedBuffer();
		switch(*fmt){
			case 0x0001:
				codec = new CodecPcm();
				break;
			case 0x0002:
				codec = new CodecMsAdpcm();
				break;
			case 0x0011:
				codec = new CodecImaAdpcm();
				break;
			default:
				throw "No codec available for this format!";
		}
	} else {
		throw "First chunk is not formatchunk ?!?!?";
	}
}

/* Open an existing wav file */
const bool TWave::open(const char * filename)
{
	if(fh == 0){
		if((fh = ::fopen(filename, "rb")) == 0){
			perror("Wave read error");
		} else {
			mode = O_RDONLY;
			fsize = 0;

			::fread((char *) & riffHeader, 1, sizeof(riffHeader), fh);

			if(strncmp((char *) riffHeader.riffChunk.chunkId, "RIFF", 4) != 0 ||
			   strncmp((char *) riffHeader.Signature, "WAVE", 4) != 0)
			   	throw "File is not an valid RIFF-WAVE";

			readChunks();

			assignCodec();

			TChunk & chnk = * chunkList.begin();
			codec->init(chnk.getAllocatedBuffer());
		}
	}
	return fh != 0;
}

/* Return common used data about wav */
void TWave::getWaveInfo(chnl_t & chnl, bps_t & bps, smpl_t & smpl, int & bs)
{
	if(codec == 0)
		throw "Codec not defined to retreive wave-info.";

	codec->getDspSettings(chnl, bps, smpl, bs);
}

/* Write data to a wav-file */
const int TWave::write(char * buffer, int len)
{
	int size = 0;

	if(fh != 0 && mode == O_WRONLY){
		if(codec == 0)
			throw "No codec to write data!";

		size = codec->encode(buffer, len, len, fh);
		fsize += size;
	}
	return size;
}

/* Read data from a wav-file */
const int TWave::read(char * buffer, int len)
{
	int size = 0;

	if(fh != 0 && mode == O_RDONLY){
		if(codec == 0)
			throw "No codec to read data!";

		size = codec->decode(buffer, len, size, fh);
	}
	return size;
}

/* Write chunks to disk */
void TWave::writeChunks()
{
	tChunkList::iterator it = chunkList.begin();
	while(it != chunkList.end()){
		TChunk & chnk = * it++;
		u_int32_t size = chnk.getSize();

		fwrite(chnk.getId(), 1, 4, fh);
		fwrite(& size, 1, 4, fh);
		if(size > 0) fwrite(chnk.getAllocatedBuffer(), 1, size, fh);
	}
}

/* Close the wav-file (if handle exists) and fill in the blanks in the header */
void TWave::close()
{
	if(fh != 0){
		if(mode == O_WRONLY){
			codec->flushEncodeBuffer(fh);

			fseek(fh, 0L, SEEK_END);
			fsize = ftell(fh);
			fseek(fh, 0, 0);

			riffHeader.riffChunk.chunkSize = fsize - sizeof(struct Chunk);
			::fwrite(& riffHeader, 1, sizeof(riffHeader), fh);

			for(tChunkList::iterator it = chunkList.begin(); it != chunkList.end(); it++){
				if(strncmp((char *) (*it).getId(), "fact", 4) == 0){
					* (u_int32_t *) (*it).getAllocatedBuffer() = codec->getEncoded();
					break;
				}
			}

			writeChunks();

			fseek(fh, -4, SEEK_CUR);
			::fwrite(& fsize, 1, 4, fh);
		}
		::fclose(fh);
		fh = 0;
	}

	if(codec != 0){
		delete codec;
		codec = 0;
	}
}

/* Return the number of samples */
const int TWave::sampleCount()
{
	tChunkList::iterator it = chunkList.begin();
	TChunk & chnk = * it;
	u_int32_t * format = 0;
	if(strncmp((const char *) chnk.getId(), "fmt ", 4) == 0){
		format = (u_int32_t *) chnk.getAllocatedBuffer();
	}

	while(strncmp((const char *) chnk.getId(), "data", 4) != 0)
		chnk = * (++it);
	
	return (fh != 0) ? (chnk.getSize() / ((u_int32_t *) format)[2]) : 0;
}

void TWave::readChunks()
{
	int size = 0;
	struct Chunk chunk;
	bool read_all_chunks = false;

	if(fh != 0){
		long pos = ftell(fh);
		long cur_pos = sizeof(struct RiffHeader);
		fseek(fh, cur_pos, SEEK_SET);

		for(int i = 1; read_all_chunks == false; i++){
			size = ::fread(&chunk, 1, sizeof(struct Chunk), fh);
			if(size != sizeof(struct Chunk)){
				cerr << "Error: reading chunk #" << i << endl;
				throw "Chunk check error";
			}

			TChunk * theChunk =  new TChunk(chunk.chunkId, chunk.chunkSize);
			if(theChunk->getAllocatedBuffer() == 0){
				if(strncmp((char *) theChunk->getId(), "data", 4) == 0)
					pos = ftell(fh);

				if(fseek(fh, chunk.chunkSize, SEEK_CUR) == -1)
					throw "Position error";
			} else {
				::fread(theChunk->getAllocatedBuffer(), 1, theChunk->getSize(), fh);
			}

			chunkList.push_back(* theChunk);
			cur_pos += sizeof(chunk) + chunk.chunkSize;

			if((u_int32_t) cur_pos + cur_pos % 1 >= riffHeader.riffChunk.chunkSize){
				read_all_chunks = true;
			}
		}

		fseek(fh, pos, SEEK_SET);
	}
}

