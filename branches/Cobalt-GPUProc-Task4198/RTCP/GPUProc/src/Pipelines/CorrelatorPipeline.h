#ifndef GPUPROC_CORRELATORPIPELINE_H
#define GPUPROC_CORRELATORPIPELINE_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Pipeline.h"
#include "FilterBank.h"
#include "PerformanceCounter.h"

namespace LOFAR
{
    namespace RTCP 
    {
        class CorrelatorWorkQueue;

        class CorrelatorPipeline : public Pipeline
        {
        public:
            CorrelatorPipeline(const Parset &);

            void		    doWork();
            void        doWorkQueue(CorrelatorWorkQueue workQueue);
            void        receiveSubbandSamples(CorrelatorWorkQueue workQueue, unsigned subband);

        private:
            friend class CorrelatorWorkQueue;

            FilterBank		    filterBank;

            cl::Program		    firFilterProgram, delayAndBandPassProgram, correlatorProgram;
#if defined USE_NEW_CORRELATOR
            PerformanceCounter	    firFilterCounter, delayAndBandPassCounter, correlateTriangleCounter, correlateRectangleCounter, fftCounter;
#else
            PerformanceCounter	    firFilterCounter, delayAndBandPassCounter, correlatorCounter, fftCounter;
#endif
            PerformanceCounter	    samplesCounter, visibilitiesCounter;
        };

    }
}
#endif
