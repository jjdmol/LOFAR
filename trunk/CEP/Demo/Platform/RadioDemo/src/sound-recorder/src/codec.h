// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _codec_h
#define _codec_h

#include <stdio.h>
#include <iostream.h>
#include <exception>
#include <sys/types.h>

class Codec {

	public:
					Codec(void * fmtInfo = 0) : format(fmtInfo){ encoded = decoded = 0;};
		virtual			~Codec(){};

		virtual void		init(void * fmtInfo) = 0;
		virtual void		init(void * fmtInfo, const chnl_t chnl, const bps_t bps,
						const smpl_t smpl) = 0;
		virtual const int	encode(char * buffer, int & len, const int slen, FILE * fh) = 0;
		virtual const int	decode(char * buffer, int & len, const int slen, FILE * fh) = 0;
		virtual const int	guessLen(const int len) = 0;
		virtual const int	flushEncodeBuffer(FILE * fh) = 0;
		virtual void		getDspSettings(chnl_t & chnl, bps_t & bps, smpl_t & smpl,
						int & bs) = 0;

		const int		defaultFormatSize(const int format) const {
						switch(format){
							case 0x0001: return 16;
							case 0x0002: return 52;
							case 0x0011: return 20;
						}
						return 0;
					}

		const int		getEncoded() const { return encoded; }
		const int		getDecoded() const { return decoded; }

	protected:
		const int16_t		readSample16(const char * buffer, const int bits){
			if(bits == 8)
				return (*buffer - 128) << 8;
			else
				return *((int16_t *) buffer);
		}
		const int8_t		readSample8(const char * buffer, const int bits){
			if(bits == 8)
				return *buffer;
			else
				return (*((int16_t *) buffer) >> 8) + 128;
		}

	protected:
		void			* format;
		int			encoded;
		int			decoded;
};

#endif

