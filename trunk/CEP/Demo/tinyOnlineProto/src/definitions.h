#ifndef TINYONLINEPROTO_DEFINITIONS_H
#define TINYONLINEPROTO_DEFINITIONS_H

#define FSIZE 1
#define NSTATIONS 20

#define TSIZE 100
#define NBEAMLETS 1
#define BFBW 256
#define NVis (BFBW/FSIZE)
#define TFACTOR (BFBW/FSIZE)
//#define NCorr (NBEAMLETS*BFBW)
#define ENABLE_FS 1

#define MIN(X,Y) ( (X) < (Y) ? (X) : (Y) )
#endif
