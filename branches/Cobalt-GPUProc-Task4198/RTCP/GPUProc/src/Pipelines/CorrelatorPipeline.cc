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
            counters(
#if defined USE_NEW_CORRELATOR
             "cor.triangle",
             "cor.rectangle",
#else
            "correlator",
#endif
            "FIR filter", "delay/bp", "FFT", "samples", "visibilities")
        {

//          counters.firFilterCounter = firFilterCounter;
//          counters.delayAndBandPassCounter = delayAndBandPassCounter;
//          counters.fftCounter = fftCounter;
//          counters.samplesCounter = samplesCounter;
//          counters.visibilitiesCounter = visibilitiesCounter;
//#if defined USE_NEW_CORRELATOR
//          counters.correlateTriangleCounter("cor.triangle");
//            counters.correlateRectangleCounter("cor.rectangle");
//#else
//            counters.correlatorCounter("correlator");
//#endif


          filterBank.negateWeights();

            double startTime = omp_get_wtime();

            //#pragma omp parallel sections
            {
                programs.firFilterProgram = createProgram("FIR.cl");
                programs.delayAndBandPassProgram = createProgram("DelayAndBandPass.cl");
#if defined USE_NEW_CORRELATOR
                programs.correlatorProgram = createProgram("NewCorrelator.cl");
#else
                programs.correlatorProgram = createProgram("Correlator.cl");
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

                  doWorkQueue(CorrelatorWorkQueue(*this, context, 
                       devices[omp_get_thread_num() % nrGPUs], omp_get_thread_num() % nrGPUs,
                       programs, counters));
                }
            }
        }

        //        void CorrelatorPipeline::receiveSubbandSamples(unsigned block, unsigned subband)
//        {
//            
//
//#ifdef USE_INPUT_SECTION
//
//#pragma omp parallel for
//            for (unsigned stat = 0; stat < ps.nrStations(); stat ++) {
//                Stream *stream = pipeline.bufferToGPUstreams[stat];
//
//                // read header
//                struct BeamletBufferToComputeNode<i16complex>::header header;
//                size_t subbandSize = inputSamples[stat].num_elements() * sizeof *inputSamples.origin();
//
//                stream->read(&header, sizeof header);
//
//                ASSERTSTR(subband == header.subband, "Expected subband " << subband << ", got subband " << header.subband);
//                ASSERTSTR(subbandSize == header.nrSamples * header.sampleSize, "Expected " << subbandSize << " bytes, got " << header.nrSamples * header.sampleSize << " bytes (= " << header.nrSamples << " samples * " << header.sampleSize << " bytes/sample)");
//
//                // read subband
//                stream->read(inputSamples[stat].origin(), subbandSize);
//
//                unsigned beam = ps.subbandToSAPmapping()[subband];
//
//                // read meta data
//                SubbandMetaData metaData(1, header.nrDelays);
//                metaData.read(stream);
//
//                // the first set of delays represents the central beam, which is the one we correlate
//                struct SubbandMetaData::beamInfo &beamInfo = metaData.beams(0)[0];
//
//                for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
//                    delaysAtBegin[beam][stat][pol]  = beamInfo.delayAtBegin;
//                    delaysAfterEnd[beam][stat][pol] = beamInfo.delayAfterEnd;
//
//                    phaseOffsets[beam][pol] = 0.0;
//                }
//            }
//
//#endif
//
//            
//        }

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
                        inputSynchronization.waitFor(block * ps.nrSubbands() + subband);
                        //receiveSubbandSamples
                        inputSynchronization.advanceTo(block * ps.nrSubbands() + subband + 1);
                        workQueue.doSubband(block, subband);
                        //target for send subband samples
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


