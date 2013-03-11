#ifndef GPUPROC_CORRELATORPIPELINE_H
#define GPUPROC_CORRELATORPIPELINE_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "Common/Timer.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Pipeline.h"
#include "FilterBank.h"
#include "PerformanceCounter.h"
#include "SubbandMetaData.h"
#include "Pipelines/CorrelatorPipelinePrograms.h"

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

        private:
            friend class CorrelatorWorkQueue;

            FilterBank		    filterBank;            
            CorrelatorPipelinePrograms programs;

            struct Performance {
              map<string, PerformanceCounter::figures> total_counters;
              map<string, SmartPtr<NSTimer> > total_timers;
              Mutex totalsMutex;

              void addQueue(CorrelatorWorkQueue &queue);
              void log(size_t nrWorkQueues);
            } performance;

        };

    }
}
#endif
