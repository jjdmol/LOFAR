#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <iostream>

#include "WorkQueues/BeamFormerWorkQueue.h"
#include "BeamFormerPipeline.h"


namespace LOFAR
{
    namespace RTCP 
    {
        BeamFormerPipeline::BeamFormerPipeline(const Parset &ps)
            :
        Pipeline(ps),
            intToFloatCounter("int-to-float"),
            fftCounter("FFT"),
            delayAndBandPassCounter("delay/bp"),
            beamFormerCounter("beamformer"),
            transposeCounter("transpose"),
            dedispersionForwardFFTcounter("ddisp.fw.FFT"),
            dedispersionChirpCounter("chirp"),
            dedispersionBackwardFFTcounter("ddisp.bw.FFT"),
            samplesCounter("samples")
        {
            double startTime = omp_get_wtime();

#pragma omp parallel sections
            {
#pragma omp section
                intToFloatProgram = createProgram("BeamFormer/IntToFloat.cl");
#pragma omp section
                delayAndBandPassProgram = createProgram("DelayAndBandPass.cl");
#pragma omp section
                beamFormerProgram = createProgram("BeamFormer/BeamFormer.cl");
#pragma omp section
                transposeProgram = createProgram("BeamFormer/Transpose.cl");
#pragma omp section
                dedispersionChirpProgram = createProgram("BeamFormer/Dedispersion.cl");
            }

            std::cout << "compile time = " << omp_get_wtime() - startTime << std::endl;
        }

                void BeamFormerPipeline::doWork()
        {
#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
                BeamFormerWorkQueue(*this, omp_get_thread_num()).doWork();
        }
    }
}

