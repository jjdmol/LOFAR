#ifndef BLUEGENE_CORRELATOR_DEFINITIONS_H
#define BLUEGENE_CORRELATOR_DEFINITIONS_H

#define __BLRTS__

// the frontend ip 
#define FRONTEND_IP  "127.0.0.1"
#define LOCALHOST_IP "127.0.0.1"
#define BASEPORT     8900

#define NSTATIONS 10
#define NCHANNELS 50
#define NSAMPLES  100

#define RUNS      500

/* #define SAMPLES  100 */
/* #define TSIZE SAMPLES */
/* #define BFBW 16 */

/* #define CHANNELS 128 */
/* #define NBEAMLETS 1 */
/* #define NVis (BFBW/FSIZE) */
/* #define TFACTOR (BFBW/FSIZE) */
/* #define NCorr (NBEAMLETS*BFBW) */
/* #define ENABLE_FS 1 */

#ifdef __BLRTS__
#include <mpi.h>

#define SEND_TASK_ID      1
#define SEND_TASK_DATA    2
#define SEND_RESULT_ID    3
#define SEND_RESULT_DATA  4
#define STOP_TAG          5

#endif

#define SOMESIZE 100


#endif

