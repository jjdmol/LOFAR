#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <iostream>
#include "ApplCommon/PosixTime.h"
#include <boost/date_time/posix_time/posix_time.hpp>

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
#           pragma omp parallel sections
            {
#               pragma omp section
                {
                    double startTime = ps.startTime(), stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

                    size_t nrStations = ps.nrStations();

#                   pragma omp parallel for num_threads(nrStations)
                    for (size_t stat = 0; stat < nrStations; stat++) 
                    {
                        double currentTime;

                        for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) 
                        {
                           sendNextBlock(stat);
                        }
                    }
                }

#               pragma omp section
                {
#                 pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)

                  doWorkQueue(CorrelatorWorkQueue(*this, omp_get_thread_num()));
                }
            }
        }

        //This whole block should be parallel: this allows the thread to pick up a subband from the next block
        void CorrelatorPipeline::doWorkQueue(CorrelatorWorkQueue workQueue)
        {
            double startTime = ps.startTime(), currentTime, stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

#pragma omp barrier

            double executionStartTime = omp_get_wtime();
            double lastTime = omp_get_wtime();

            for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) 
            {
#pragma omp single nowait
#pragma omp critical (cout)
                std::cout << "block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)) << ", exec = " << omp_get_wtime() - lastTime << std::endl;
                
                lastTime = omp_get_wtime();

#pragma omp for schedule(dynamic), nowait, ordered
                for (unsigned subband = 0; subband < ps.nrSubbands(); subband ++) 
                {
                        workQueue.doSubband(block, subband);
                }
            }

#pragma omp barrier //Wait till all the queues are done working on the current block and subband

#pragma omp master
            if (!profiling)
#pragma omp critical (cout)
                std::cout << "run time = " << omp_get_wtime() - executionStartTime << std::endl;
        }
    }
}

