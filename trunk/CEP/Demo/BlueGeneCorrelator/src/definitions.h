#ifndef BLUEGENE_CORRELATOR_DEFINITIONS_H

// the frontend ip 
#define FRONTEND_IP  "10.0.0.22"
#define LOCALHOST_IP "127.0.0.1"
#define BASEPORT     8900

#define FSIZE 1
#define NSTATIONS 1

#define SAMPLES  100
#define TSIZE SAMPLES
#define CHANNELS 128
#define NBEAMLETS 1
#define BFBW 1
#define NVis (BFBW/FSIZE)
#define TFACTOR (BFBW/FSIZE)
#define NCorr (NBEAMLETS*BFBW)
#define ENABLE_FS 1

#endif
