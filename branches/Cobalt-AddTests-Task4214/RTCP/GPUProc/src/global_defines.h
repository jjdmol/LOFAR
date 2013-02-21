#ifndef GPUPROC_GLOBAL_DEFINES_H
#define GPUPROC_GLOBAL_DEFINES_H
#define __CL_ENABLE_EXCEPTIONS

#if defined __linux__
#include <sched.h>
#include <sys/time.h>
#endif

#define NR_STATION_FILTER_TAPS	16
#define USE_NEW_CORRELATOR
#define NR_POLARIZATIONS	 2
#define NR_TAPS			16
#define USE_2X2
#define USE_INPUT_SECTION
#undef USE_CUSTOM_FFT
#undef USE_TEST_DATA
#undef USE_B7015

namespace LOFAR 
{
    namespace RTCP 
    {
        extern bool	 profiling;
        extern unsigned nrGPUs;
    }
}
#endif
