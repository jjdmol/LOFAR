#ifndef TINYONLINEPROTO_DEFINITIONS_H
#define TINYONLINEPROTO_DEFINITIONS_H

#define FSIZE 1
#define NSTATIONS 1

#define TSIZE 100
#define NBEAMLETS 1
#define BFBW 1
#define NVis (BFBW/FSIZE)
#define TFACTOR (BFBW/FSIZE)
//#define NCorr (NBEAMLETS*BFBW)
#define ENABLE_FS 1

#define LOCALHOST_IP "127.0.0.1"
#define FRONTEND_IP  "10.0.0.22"    // bgfe01.watson.ibm.com

#define IN_BASEPORT  8100
#define OUT_BASEPORT 9100

#define MAX_BG_NODES 64

#define MIN(X,Y) ( (X) < (Y) ? (X) : (Y) )
#endif
