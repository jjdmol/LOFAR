#ifndef ONLINEPROTO_DEFINITIONS_H
#define ONLINEPROTO_DEFINITIONS_H

#define FSIZE 1
#define NSTATIONS 100

#define TSIZE 100
#define NBEAMLETS 8
#define BFBW 256
#define NVis (BFBW/FSIZE)
#define TFACTOR (BFBW/FSIZE)
#define NCorr (NBEAMLETS*BFBW)  


#define MIN(X,Y) ( (X) < (Y) ? (X) : (Y) )
#endif
