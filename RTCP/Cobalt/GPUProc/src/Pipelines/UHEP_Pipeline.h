#ifndef GPUPROC_UHEP_PIPELINE_H
#define GPUPROC_UHEP_PIPELINE_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Pipeline.h"
#include "FilterBank.h"
#include "PerformanceCounter.h"

namespace LOFAR
{
    namespace RTCP 
    {

        class UHEP_Pipeline : public Pipeline
        {
        public:
            UHEP_Pipeline(const Parset &);

            void		    doWork();

            cl::Program		    beamFormerProgram, transposeProgram, invFFTprogram, invFIRfilterProgram, triggerProgram;
            PerformanceCounter	    beamFormerCounter, transposeCounter, invFFTcounter, invFIRfilterCounter, triggerCounter;
            PerformanceCounter	    beamFormerWeightsCounter, samplesCounter;
        };

    }
}
#endif
