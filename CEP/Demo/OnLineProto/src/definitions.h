#ifndef ONLINEPROTO_DEFINITIONS_H
#define ONLINEPROTO_DEFINITIONS_H

#define FSIZE 1
#define NSTATIONS 100

#define TSIZE 1
#define NBEAMLETS 1
#define BFBW 256
#define NVis (BFBW/FSIZE)
#define TFACTOR (BFBW/FSIZE)
#define NCorr (NBEAMLETS*BFBW/64)  
#define ENABLE_FS 0

#define MIN(X,Y) ( (X) < (Y) ? (X) : (Y) )
#endif
