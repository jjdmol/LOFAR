#ifndef GPUPROC_CORRELATORPIPELINE_H
#define GPUPROC_CORRELATORPIPELINE_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Pipeline.h"
#include "FilterBank.h"
#include "PerformanceCounter.h"
#include "SubbandMetaData.h"
#include "Pipelines/CorrelatorPipelinePrograms.h"
#include "Pipelines/CorrelatorPipelineCounters.h"

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
            void        doWorkQueue(CorrelatorWorkQueue &workQueue);
            void        receiveSubbandSamples(CorrelatorWorkQueue &workQueue, unsigned block, unsigned subband);
            void        sendSubbandVisibilities(CorrelatorWorkQueue &workQueue, unsigned block, unsigned subband);

        private:
            friend class CorrelatorWorkQueue;

            FilterBank		    filterBank;            
            CorrelatorPipelineCounters counters;
            CorrelatorPipelinePrograms programs;


      // Flag the input data of the given station using to the flags in the
      // meta-data.
      void flagInputSamples(CorrelatorWorkQueue &workQueue, unsigned station, const SubbandMetaData& metaData);

        };

    }
}
#endif
