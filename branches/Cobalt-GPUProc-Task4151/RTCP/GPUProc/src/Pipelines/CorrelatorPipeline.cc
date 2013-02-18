#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <iostream>

#include "CorrelatorPipeline.h"

namespace LOFAR
{
    namespace RTCP 
    {
        CorrelatorPipeline::CorrelatorPipeline(const Parset &ps)
            :
        Pipeline(ps),
            filterBank(true, NR_TAPS, ps.nrChannelsPerSubband(), KAISER),
            firFilterCounter("FIR filter"),
            delayAndBandPassCounter("delay/bp"),
#if defined USE_NEW_CORRELATOR
            correlateTriangleCounter("cor.triangle"),
            correlateRectangleCounter("cor.rectangle"),
#else
            correlatorCounter("correlator"),
#endif
            fftCounter("FFT"),
            samplesCounter("samples"),
            visibilitiesCounter("visibilities")
        {
            filterBank.negateWeights();

            double startTime = omp_get_wtime();

            //#pragma omp parallel sections
            {
                //#pragma omp section
                firFilterProgram = createProgram("FIR.cl");
                //#pragma omp section
                delayAndBandPassProgram = createProgram("DelayAndBandPass.cl");
                //#pragma omp section
#if defined USE_NEW_CORRELATOR
                correlatorProgram = createProgram("NewCorrelator.cl");
#else
                correlatorProgram = createProgram("Correlator.cl");
#endif
            }

            std::cout << "compile time = " << omp_get_wtime() - startTime << std::endl;
        }

    }
}

