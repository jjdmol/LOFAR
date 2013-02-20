#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <iostream>

#include "CorrelatorPipeline.h"
#include "WorkQueues/CorrelatorWorkQueue.h"

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

        void CorrelatorPipeline::doWork()
        {
#pragma omp parallel sections
            {
#pragma omp section
                {
                    double startTime = ps.startTime(), stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

                    size_t nrStations = ps.nrStations();

#pragma omp parallel for num_threads(nrStations)
                    for (size_t stat = 0; stat < nrStations; stat++) {
                        double currentTime;

                        for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) {
                            //#pragma omp critical (cout)
                            //std::cout << "send station = " << stat << ", block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)) << std::endl;

                            sendNextBlock(stat);
                        }
                    }
                }

#pragma omp section
                {
#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
                        CorrelatorWorkQueue(*this, omp_get_thread_num()).doWork();
                }
            }
        }


    }
}

