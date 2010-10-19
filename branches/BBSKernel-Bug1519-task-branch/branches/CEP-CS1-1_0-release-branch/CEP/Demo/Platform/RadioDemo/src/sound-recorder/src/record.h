// Sound recorder 0.06 (Build on Aug 06 2000), GPL 2 (see COPYRIGHTS)
// 1997, 1998  B. Warmerdam
// $Id$

#ifndef _record_h
#define _record_h

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/soundcard.h>
#include <string.h>
#include <wait.h>

#include "dsp.h"
#include "cd-player.h"
#include "soundrec.h"
#include "waveriff.h"
#include "pcm.h"
#include "cdr.h"
#include "rcfile.h"
#include "mixer.h"

#define NUM_BUFFERS 1024

struct cdRecordData{
	CDplayer * cdrom;
	struct recordSettings * recSettings;
};

struct audio_buffers{
	int first, last;
	unsigned long bufferSize;
	char * buf [NUM_BUFFERS];
	unsigned long len [NUM_BUFFERS];
	int pipe [2];
	unsigned long recordedBytes;
	unsigned long numberOfSeconds;
	Dsp *dsp;
	TFileFormat * oFile;
};

#endif
