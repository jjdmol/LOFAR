// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "cdr.h"

/* Initialize object with a invalid file-handle (to test later on) */
TCdr::TCdr()
{
	fh = 0;
}

/* Close file (to be sure) */
TCdr::~TCdr()
{
	close();
}

/* Create a new file with specified arguments */
const bool TCdr::create(const char * filename, wavfmt_t, chnl_t, bps_t, smpl_t)
{
	if(fh == 0){
		if(filename[0] == '-' && filename[1] == '\0') {
			fprintf(stderr, "Writing to stdout.\n");
			fh = stdout;
			mode = O_WRONLY;
		} else {
			if((fh = ::fopen(filename, "w+b")) == 0){
				perror("CDR create error");
			} else {
				mode = O_WRONLY;
			}
		}
	}
	return fh != 0;
}

/* Open a cdr file for reading */
const bool TCdr::open(const char * filename)
{
	if(fh == 0){
		if(filename[0] == '-' && filename[1] == '\0') {
			fprintf(stderr, "Reading from stdin.\n");
			fh = stdin;
			mode = O_RDONLY;
		} else {
			if((fh = ::fopen(filename, "rb")) == 0){
				perror("Cdr read error");
			} else {
				mode = O_RDONLY;
			}
		}
	}
	return fh != 0;
}

/* Write data to a cdr-file */
const int TCdr::write(char * buffer, int len)
{
	int size = 0;

	if(fh != 0 && mode == O_WRONLY){
		changeByteOrder(buffer, len);
		size = ::fwrite(buffer, 1, len, fh);
	}
	return size;
}

/* Read data from a cdr file */
const int TCdr::read(char * buffer, int len)
{
	int size = 0;

	if(fh != 0 && mode == O_RDONLY){
		size = ::fread(buffer, 1, len, fh);
		changeByteOrder(buffer, size);
	}
	return size;
}

/* Close a cdr file */
void TCdr::close()
{
	if(fh != 0){
		::fclose(fh);
		fh = 0;
	}
}


/* Change byteoder */
void TCdr::changeByteOrder(char * buffer, const int len)
{
	for(int i = 0; i < len; i += 2){
		const char low = buffer[i];
		buffer[i] = buffer[i+1];
		buffer[i+1] = low;
	}
}
