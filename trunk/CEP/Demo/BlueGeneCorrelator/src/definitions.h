#ifndef BLUEGENE_CORRELATOR_DEFINITIONS_H
#define BLUEGENE_CORRELATOR_DEFINITIONS_H

#define __BLRTS__
#define __MPE_LOGGING__


// the frontend ip 
#define FRONTEND_IP  "127.0.0.1"
#define LOCALHOST_IP "127.0.0.1"
#define BASEPORT     8900

#define NSTATIONS 100
#define NCHANNELS 35
#define NSAMPLES  10

#define RUNS      100

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

