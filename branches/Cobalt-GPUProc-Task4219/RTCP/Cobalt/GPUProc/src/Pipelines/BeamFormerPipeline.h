#ifndef GPUPROC_BEAMFORMERPIPELINE_H
#define GPUPROC_BEAMFORMERPIPELINE_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"
#include "PerformanceCounter.h"

#include "Pipeline.h"

namespace LOFAR
{
    namespace RTCP 
    {
                class BeamFormerPipeline : public Pipeline
        {
        public:
            BeamFormerPipeline(const Parset &);

            void		    doWork();

            cl::Program		    intToFloatProgram, delayAndBandPassProgram, beamFormerProgram, transposeProgram, dedispersionChirpProgram;

            PerformanceCounter	    intToFloatCounter, fftCounter, delayAndBandPassCounter, beamFormerCounter, transposeCounter, dedispersionForwardFFTcounter, dedispersionChirpCounter, dedispersionBackwardFFTcounter;
            PerformanceCounter	    samplesCounter;
        };
    }
}
#endif 
