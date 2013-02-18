#ifndef GPUPROC_BEAMFORMERWORKQUEUE_H
#define GPUPROC_BEAMFORMERWORKQUEUE_H

#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <algorithm>
#include <iostream>
#include "ApplCommon/PosixTime.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Pipelines/BeamFormerPipeline.h"
#include "CorrelatorWorkQueue.h"
#include "BandPass.h"

namespace LOFAR
{
    namespace  RTCP 
    {     
                class BeamFormerWorkQueue : public WorkQueue
        {
        public:
            BeamFormerWorkQueue(BeamFormerPipeline &, unsigned queueNumber);

            void doWork();

            BeamFormerPipeline	&pipeline;

            MultiArraySharedBuffer<char, 4>		   inputSamples;
            cl::Buffer					   devFilteredData;
            MultiArraySharedBuffer<float, 1>		   bandPassCorrectionWeights;
            MultiArraySharedBuffer<float, 3>		   delaysAtBegin, delaysAfterEnd;
            MultiArraySharedBuffer<float, 2>		   phaseOffsets;
            cl::Buffer					   devCorrectedData;
            MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights;
            cl::Buffer					   devComplexVoltages;
            MultiArraySharedBuffer<std::complex<float>, 4> transposedComplexVoltages;
            MultiArraySharedBuffer<float, 1>		   DMs;
        };

    }
}
#endif
