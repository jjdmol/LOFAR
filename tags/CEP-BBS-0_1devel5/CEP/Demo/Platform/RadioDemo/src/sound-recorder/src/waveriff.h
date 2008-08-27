// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _waveriff_h
#define _wavefiff_h

#include "types.h"

#include <iostream.h>
#include <exception>
#include <list>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "filefmt.h"
#include "codec.h"
#include "codec_pcm.h"
#include "codec_msadpcm.h"
#include "codec_imaadpcm.h"

class TWave;

class TChunk {

	friend			TWave;

	public:
				TChunk(const u_int8_t Id[4] = 0, const u_int32_t size = 0);
				~TChunk();
				TChunk(const TChunk &);

		const u_int8_t  *getId() const;
		const u_int32_t	getSize() const;

	protected:
		void		allocate(const u_int8_t id[4], const u_int32_t size);
		char *		getAllocatedBuffer() const;
		
	private:
		u_int8_t	chunkId[4];		// Chunk id
		u_int8_t	idNull[2];
		u_int32_t	chunkSize;		// Chunk size
		char		* chunkData;
};

typedef list<TChunk> tChunkList;

class TWave : public TFileFormat {

	public:
		TWave();
		~TWave();

		const bool		create(const char * filename, const wavfmt_t fmt, const chnl_t chnl,
						const bps_t bps, const smpl_t smpl);
		const bool		open(const char * filename);
		const int		write(char * buffer, int len);
		const int		read(char * buffer, int len);
		void			close();
		void			getWaveInfo(chnl_t & chnl, bps_t & bps, smpl_t & smpl, int & bs);
		const int		sampleCount();
		void			setFileSize(const long size);

	private:
		struct Chunk{
			u_int8_t			chunkId[4];		// Chunk id
			u_int32_t			chunkSize;		// Chunk size
		};

		struct RiffHeader{
			struct Chunk			riffChunk;
			u_int8_t			Signature[4];
		};

		FILE *			fh;
		int			mode;
		u_int32_t		fsize;
		tChunkList		chunkList;
		struct RiffHeader	riffHeader;
		Codec			* codec;

	private:
		void			createWave(const wavfmt_t fmt, const chnl_t chnl, 
						const bps_t bps, const smpl_t smpl);
		void			readChunks();
		void			assignCodec();
		void			writeChunks();

};

#endif
