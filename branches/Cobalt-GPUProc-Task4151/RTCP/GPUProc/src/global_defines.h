#ifndef GPUPROC_GLOBAL_DEFINES_H
#define GPUPROC_GLOBAL_DEFINES_H


namespace LOFAR 
{
    namespace RTCP 
    {
#define NR_STATION_FILTER_TAPS	16
#define USE_NEW_CORRELATOR
#define NR_POLARIZATIONS	 2
#define NR_TAPS			16
#define USE_2X2
#undef USE_INPUT_SECTION
        extern bool	 profiling;

    }
}
#endif
