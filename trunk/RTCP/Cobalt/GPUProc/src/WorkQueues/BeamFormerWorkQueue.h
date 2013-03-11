#ifndef GPUPROC_BEAMFORMERWORKQUEUE_H
#define GPUPROC_BEAMFORMERWORKQUEUE_H

#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <algorithm>
#include <iostream>
#include "ApplCommon/PosixTime.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Pipelines/BeamFormerPipeline.h"
#include "BandPass.h"
#include "WorkQueue.h"

#include "Kernels/IntToFloatKernel.h"
#include "Kernels/Filter_FFT_Kernel.h"
#include "Kernels/DelayAndBandPassKernel.h"
#include "Kernels/BeamFormerKernel.h"
#include "Kernels/BeamFormerTransposeKernel.h"
#include "Kernels/DedispersionForwardFFTkernel.h"
#include "Kernels/DedispersionBackwardFFTkernel.h"
#include "Kernels/DedispersionChirpKernel.h"


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

        private:
            IntToFloatKernel intToFloatKernel;
            Filter_FFT_Kernel fftKernel;
            DelayAndBandPassKernel delayAndBandPassKernel;
            BeamFormerKernel beamFormerKernel;
            BeamFormerTransposeKernel transposeKernel;
            DedispersionForwardFFTkernel dedispersionForwardFFTkernel;
            DedispersionBackwardFFTkernel dedispersionBackwardFFTkernel;
            DedispersionChirpKernel dedispersionChirpKernel;
        };

    }
}
#endif
