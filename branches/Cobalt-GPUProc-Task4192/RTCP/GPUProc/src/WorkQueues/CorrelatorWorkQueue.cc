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

#include "Pipeline.h"
#include "CorrelatorWorkQueue.h"
#include "BandPass.h"

namespace LOFAR
{
    namespace  RTCP 
    {     
        CorrelatorWorkQueue::CorrelatorWorkQueue(CorrelatorPipeline &pipeline, unsigned queueNumber)
            :
        WorkQueue(pipeline, queueNumber),
            pipeline(pipeline),

            devFIRweights(pipeline.context, CL_MEM_READ_ONLY, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float)),
            devBufferA(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>)),
            devBufferB(pipeline.context, CL_MEM_READ_WRITE, std::max(ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>), ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>))),
            bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            delaysAtBegin(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            delaysAfterEnd(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            phaseOffsets(boost::extents[ps.nrBeams()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            //inputSamples(boost::extents[ps.nrStations()][(ps.nrSamplesPerChannel() + NR_TAPS - 1) * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            //visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY)
            inputSamples(boost::extents[ps.nrStations()][(ps.nrSamplesPerChannel() + NR_TAPS - 1) * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, devBufferA),
            devFilteredData(devBufferB),
            devCorrectedData(devBufferA),
            visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, devBufferB),
            firFilterKernel(ps, queue, pipeline.firFilterProgram, devFilteredData, inputSamples, devFIRweights),

            fftKernel(ps, pipeline.context, devFilteredData),
            delayAndBandPassKernel(ps, pipeline.delayAndBandPassProgram, devCorrectedData, devFilteredData, delaysAtBegin, delaysAfterEnd, phaseOffsets, bandPassCorrectionWeights),
#if defined USE_NEW_CORRELATOR
            correlateTriangleKernel(ps, queue, pipeline.correlatorProgram, visibilities, devCorrectedData),
            correlateRectangleKernel(ps, queue, pipeline.correlatorProgram, visibilities, devCorrectedData)
#else
            correlatorKernel(ps, queue, pipeline.correlatorProgram, visibilities, devCorrectedData)
#endif
        {
            queue.enqueueWriteBuffer(devFIRweights, CL_TRUE, 0, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float), pipeline.filterBank.getWeights().origin());

#if 0
            size_t filteredDataSize = ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>);
            devFilteredData = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, filteredDataSize);
            devCorrectedData = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, filteredDataSize);
#endif

            if (ps.correctBandPass()) {
                BandPass::computeCorrectionFactors(bandPassCorrectionWeights.origin(), ps.nrChannelsPerSubband());
                bandPassCorrectionWeights.hostToDevice(CL_TRUE);
            }
        }


        void CorrelatorWorkQueue::receiveSubbandSamples(unsigned block, unsigned subband)
        {
            pipeline.inputSynchronization.waitFor(block * ps.nrSubbands() + subband);

#ifdef USE_INPUT_SECTION

#pragma omp parallel for
            for (unsigned stat = 0; stat < ps.nrStations(); stat ++) {
                Stream *stream = pipeline.bufferToGPUstreams[stat];

                // read header
                struct BeamletBufferToComputeNode<i16complex>::header header;
                size_t subbandSize = inputSamples[stat].num_elements() * sizeof *inputSamples.origin();

                stream->read(&header, sizeof header);

                ASSERTSTR(subband == header.subband, "Expected subband " << subband << ", got subband " << header.subband);
                ASSERTSTR(subbandSize == header.nrSamples * header.sampleSize, "Expected " << subbandSize << " bytes, got " << header.nrSamples * header.sampleSize << " bytes (= " << header.nrSamples << " samples * " << header.sampleSize << " bytes/sample)");

                // read subband
                stream->read(inputSamples[stat].origin(), subbandSize);

                unsigned beam = ps.subbandToSAPmapping()[subband];

                // read meta data
                SubbandMetaData metaData(1, header.nrDelays);
                metaData.read(stream);

                // the first set of delays represents the central beam, which is the one we correlate
                struct SubbandMetaData::beamInfo &beamInfo = metaData.beams(0)[0];

                for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
                    delaysAtBegin[beam][stat][pol]  = beamInfo.delayAtBegin;
                    delaysAfterEnd[beam][stat][pol] = beamInfo.delayAfterEnd;

                    phaseOffsets[beam][pol] = 0.0;
                }
            }

#endif

            pipeline.inputSynchronization.advanceTo(block * ps.nrSubbands() + subband + 1);
        }


        void CorrelatorWorkQueue::sendSubbandVisibilites(unsigned block, unsigned subband)
        {
            pipeline.outputSynchronization.waitFor(block * ps.nrSubbands() + subband);
            pipeline.GPUtoStorageStreams[subband]->write(visibilities.origin(), visibilities.num_elements() * sizeof(std::complex<float>));
            pipeline.outputSynchronization.advanceTo(block * ps.nrSubbands() + subband + 1);
        }


        void CorrelatorWorkQueue::doSubband(unsigned block, unsigned subband)
        {
            receiveSubbandSamples(block, subband);

            {
#if defined USE_B7015
                OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
                inputSamples.hostToDevice(CL_TRUE);
                pipeline.samplesCounter.doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
            }

            if (ps.nrChannelsPerSubband() > 1) {
                firFilterKernel.enqueue(queue, pipeline.firFilterCounter);
                fftKernel.enqueue(queue, pipeline.fftCounter);
            }

            delayAndBandPassKernel.enqueue(queue, pipeline.delayAndBandPassCounter, subband);
#if defined USE_NEW_CORRELATOR
            correlateTriangleKernel.enqueue(queue, pipeline.correlateTriangleCounter);
            correlateRectangleKernel.enqueue(queue, pipeline.correlateRectangleCounter);
#else
            correlatorKernel.enqueue(queue, pipeline.correlatorCounter);
#endif
            queue.finish();

            {
#if defined USE_B7015
                OMP_ScopedLock scopedLock(pipeline.deviceToHostLock[gpu / 2]);
#endif
                visibilities.deviceToHost(CL_TRUE);
                pipeline.visibilitiesCounter.doOperation(visibilities.event, 0, visibilities.bytesize(), 0);
            }

            sendSubbandVisibilites(block, subband);
        }


        void CorrelatorWorkQueue::doWork()
        {
            double startTime = ps.startTime(), currentTime, stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

#pragma omp barrier

            double executionStartTime = omp_get_wtime();
            double lastTime = omp_get_wtime();

            for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) {
#pragma omp single nowait
#pragma omp critical (cout)
                std::cout << "block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)) << ", exec = " << omp_get_wtime() - lastTime << std::endl;
                lastTime = omp_get_wtime();

                memset(delaysAtBegin.origin(), 0, delaysAtBegin.bytesize());
                memset(delaysAfterEnd.origin(), 0, delaysAfterEnd.bytesize());
                memset(phaseOffsets.origin(), 0, phaseOffsets.bytesize());

                // FIXME!!!
                //if (ps.nrStations() >= 3)
                //delaysAtBegin[0][2][0] = 1e-6, delaysAfterEnd[0][2][0] = 1.1e-6;

                delaysAtBegin.hostToDevice(CL_FALSE);
                delaysAfterEnd.hostToDevice(CL_FALSE);
                phaseOffsets.hostToDevice(CL_FALSE);

#pragma omp for schedule(dynamic), nowait, ordered
                for (unsigned subband = 0; subband < ps.nrSubbands(); subband ++) {
                        doSubband(block, subband);
                }
            }

#pragma omp barrier

#pragma omp master
            if (!profiling)
#pragma omp critical (cout)
                std::cout << "run time = " << omp_get_wtime() - executionStartTime << std::endl;
        }


    }
}
