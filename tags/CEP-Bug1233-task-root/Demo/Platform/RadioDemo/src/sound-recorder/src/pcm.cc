// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#include "pcm.h"

/* Initialize object with a invalid file-handle (to test later on) */
TPcm::TPcm()
{
	fh = 0;
}

/* Close file (to be sure) */
TPcm::~TPcm()
{
	close();
}

/* Create a new file with specified arguments */
const bool TPcm::create(const char * filename, wavfmt_t, chnl_t, bps_t, smpl_t)
{
	if(fh == 0){
		if(filename[0] == '-' && filename[1] == '\0') {
			fprintf(stderr, "Writing to stdout.\n");
			fh = stdout;
			mode = O_WRONLY;
		} else {
			if((fh = ::fopen(filename, "w+b")) == 0){
				perror("PCM create error");
			} else {
				mode = O_WRONLY;
			}
		}
	}
	return fh != 0;
}

/* Open a pcm file for reading */
const bool TPcm::open(const char * filename)
{
	if(fh == 0){
		if(filename[0] == '-' && filename[1] == '\0') {
			fprintf(stderr, "Reading from stdin.\n");
			fh = stdin;
			mode = O_RDONLY;
		} else {
			if((fh = ::fopen(filename, "rb")) == 0){
				perror("Pcm read error");
			} else {
				mode = O_RDONLY;
			}
		}
	}
	return fh != 0;
}

/* Write data to a pcm-file */
const int TPcm::write(char * buffer, int len)
{
	int size = 0;

	if(fh != 0 && mode == O_WRONLY){
		size = ::fwrite(buffer, 1, len, fh);
	}
	return size;
}

/* Read data from a pcm file */
const int TPcm::read(char * buffer, int len)
{
	int size = 0;

	if(fh != 0 && mode == O_RDONLY){
		size = ::fread(buffer, 1, len, fh);
	}
	return size;
}

/* Close a pcm file */
void TPcm::close()
{
	if(fh != 0){
		::fclose(fh);
		fh = 0;
	}
}

