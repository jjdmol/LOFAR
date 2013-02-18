#include "lofar_config.h"

#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#include <global_defines.h>

#include <omp.h>
#include <string.h>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <boost/multi_array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include "Align.h"
#include "ApplCommon/PosixTime.h"
#include "BandPass.h"
#include "Common/LofarLogger.h"
#include "Common/SystemUtil.h"
#include "Stream/SharedMemoryStream.h"
#include "FilterBank.h"
#include "BeamletBufferToComputeNode.h"
#include "InputSection.h"
#include "Interface/Parset.h"
#include "Interface/SmartPtr.h"
#include "OpenCL_FFT/clFFT.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include "SlidingPointer.h"
#include "Stream/Stream.h"
#include "Stream/NullStream.h"
#include "UHEP/InvertedStationPPFWeights.h"
//#include "clAmdFft/include/clAmdFft.h"

//functionality moved to individual sources
#include "createProgram.h"
#include "PerformanceCounter.h"
#include "UnitTest.h"
#include "Kernel.h"
#include "FFT_Kernel.h"
#include "FFT_Plan.h"

#include "Kernels/FIR_FilterKernel.h"
#include "Kernels/DelayAndBandPassKernel.h"
#include "Kernels/CorrelatorKernel.h"
#include "Kernels/IntToFloatKernel.h"
#include "Kernels/IncoherentStokesKernel.h"
#include "Kernels/BeamFormerKernel.h"
#include "Kernels/BeamFormerTransposeKernel.h"
#include "Kernels/DedispersionChirpKernel.h"
#include "Kernels/CoherentStokesKernel.h"
#include "Kernels/UHEP_BeamFormerKernel.h"
#include "Kernels/UHEP_TransposeKernel.h"
#include "Kernels/UHEP_InvFFT_Kernel.h"
#include "Kernels/UHEP_InvFIR_Kernel.h"
#include "Kernels/UHEP_TriggerKernel.h"
#include "Kernels/Filter_FFT_Kernel.h"
#include "Kernels/DedispersionForwardFFTkernel.h"
#include "Kernels/DedispersionBackwardFFTkernel.h"

#include "Pipeline.h"
#include "Pipelines/CorrelatorPipeline.h"
#include "Pipelines/BeamFormerPipeline.h"
#include "Pipelines/UHEP_Pipeline.h"

#include "WorkQueues/WorkQueue.h"
#include "WorkQueues/CorrelatorWorkQueue.h"

#if defined __linux__
#include <sched.h>
#include <sys/time.h>
#endif

namespace LOFAR {
    namespace RTCP {


#undef USE_CUSTOM_FFT
#undef USE_TEST_DATA
#undef USE_B7015

#if defined __linux__

        inline void set_affinity(unsigned device)
        {
#if 0
            static const char mapping[1][12] = {
                0,  1,  2,  3,  8,  9, 10, 11,
            };
#else
            static const char mapping[8][12] = {
                { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
                { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
                { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
                { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
                { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
                { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
                { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
                { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
            };
#endif

            cpu_set_t set;

            CPU_ZERO(&set);

            for (unsigned coreIndex = 0; coreIndex < 12; coreIndex ++)
                CPU_SET(mapping[device][coreIndex], &set);

            if (sched_setaffinity(0, sizeof set, &set) < 0)
                perror("sched_setaffinity");
        }

#endif




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


        class UHEP_WorkQueue : public WorkQueue
        {
        public:
            UHEP_WorkQueue(UHEP_Pipeline &, unsigned queueNumber);

            void doWork(const float *delaysAtBegin, const float *delaysAfterEnd, const float *phaseOffsets);

            UHEP_Pipeline	&pipeline;
            cl::Event		inputSamplesEvent, beamFormerWeightsEvent;

            cl::Buffer		devBuffers[2];
            cl::Buffer		devInputSamples;
            MultiArrayHostBuffer<char, 5> hostInputSamples;

            cl::Buffer		devBeamFormerWeights;
            MultiArrayHostBuffer<std::complex<float>, 3> hostBeamFormerWeights;

            cl::Buffer		devComplexVoltages;
            cl::Buffer		devReverseSubbandMapping;
            cl::Buffer		devFFTedData;
            cl::Buffer		devInvFIRfilteredData;
            cl::Buffer		devInvFIRfilterWeights;

            cl::Buffer		devTriggerInfo;
            VectorHostBuffer<TriggerInfo> hostTriggerInfo;
        };



        BeamFormerWorkQueue::BeamFormerWorkQueue(BeamFormerPipeline &pipeline, unsigned queueNumber)
            :
        WorkQueue(pipeline, queueNumber),
            pipeline(pipeline),
            inputSamples(boost::extents[ps.nrStations()][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            devFilteredData(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>)),
            bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            delaysAtBegin(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            delaysAfterEnd(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            phaseOffsets(boost::extents[ps.nrBeams()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            devCorrectedData(cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>))),
            beamFormerWeights(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
            devComplexVoltages(cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>))),
            //transposedComplexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE)
            transposedComplexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE),
            DMs(boost::extents[ps.nrTABs(0)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY)
        {
            if (ps.correctBandPass()) {
                BandPass::computeCorrectionFactors(bandPassCorrectionWeights.origin(), ps.nrChannelsPerSubband());
                bandPassCorrectionWeights.hostToDevice(CL_TRUE);
            }
        }


        void BeamFormerWorkQueue::doWork()
        {
            //queue.enqueueWriteBuffer(devFIRweights, CL_TRUE, 0, firWeightsSize, firFilterWeights);
            bandPassCorrectionWeights.hostToDevice(CL_TRUE);
            DMs.hostToDevice(CL_TRUE);

            IntToFloatKernel intToFloatKernel(ps, queue, pipeline.intToFloatProgram, devFilteredData, inputSamples);
            Filter_FFT_Kernel fftKernel(ps, pipeline.context, devFilteredData);
            DelayAndBandPassKernel delayAndBandPassKernel(ps, pipeline.delayAndBandPassProgram, devCorrectedData, devFilteredData, delaysAtBegin, delaysAfterEnd, phaseOffsets, bandPassCorrectionWeights);
            BeamFormerKernel beamFormerKernel(ps, pipeline.beamFormerProgram, devComplexVoltages, devCorrectedData, beamFormerWeights);
            BeamFormerTransposeKernel transposeKernel(ps, pipeline.transposeProgram, transposedComplexVoltages, devComplexVoltages);
            DedispersionForwardFFTkernel dedispersionForwardFFTkernel(ps, pipeline.context, transposedComplexVoltages);
            DedispersionBackwardFFTkernel dedispersionBackwardFFTkernel(ps, pipeline.context, transposedComplexVoltages);
            DedispersionChirpKernel dedispersionChirpKernel(ps, pipeline.dedispersionChirpProgram, queue, transposedComplexVoltages, DMs);
            double startTime = ps.startTime(), currentTime, stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

#pragma omp barrier

            double executionStartTime = omp_get_wtime();

            for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block ++) {
#pragma omp single nowait
#pragma omp critical (cout)
                std::cout << "block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)) << std::endl;

                memset(delaysAtBegin.origin(), 0, delaysAtBegin.bytesize());
                memset(delaysAfterEnd.origin(), 0, delaysAfterEnd.bytesize());
                memset(phaseOffsets.origin(), 0, phaseOffsets.bytesize());

                // FIXME!!!
                if (ps.nrStations() >= 3)
                    delaysAtBegin[0][2][0] = 1e-6, delaysAfterEnd[0][2][0] = 1.1e-6;

                delaysAtBegin.hostToDevice(CL_FALSE);
                delaysAfterEnd.hostToDevice(CL_FALSE);
                phaseOffsets.hostToDevice(CL_FALSE);
                beamFormerWeights.hostToDevice(CL_FALSE);

#pragma omp for schedule(dynamic), nowait
                for (unsigned subband = 0; subband < ps.nrSubbands(); subband ++) {
                    try {
#if 1
                        {
#if defined USE_B7015
                            OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
                            inputSamples.hostToDevice(CL_TRUE);
                            pipeline.samplesCounter.doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
                        }
#endif

                        //#pragma omp critical (GPU)
                        {
                            if (ps.nrChannelsPerSubband() > 1) {
                                intToFloatKernel.enqueue(queue, pipeline.intToFloatCounter);
                                fftKernel.enqueue(queue, pipeline.fftCounter);
                            }

                            delayAndBandPassKernel.enqueue(queue, pipeline.delayAndBandPassCounter, subband);
                            beamFormerKernel.enqueue(queue, pipeline.beamFormerCounter);
                            transposeKernel.enqueue(queue, pipeline.transposeCounter);
                            dedispersionForwardFFTkernel.enqueue(queue, pipeline.dedispersionForwardFFTcounter);
                            dedispersionChirpKernel.enqueue(queue, pipeline.dedispersionChirpCounter, ps.subbandToFrequencyMapping()[subband]);
                            dedispersionBackwardFFTkernel.enqueue(queue, pipeline.dedispersionBackwardFFTcounter);

                            queue.finish();
                        }

                        //queue.enqueueReadBuffer(devComplexVoltages, CL_TRUE, 0, hostComplexVoltages.bytesize(), hostComplexVoltages.origin());
                        //dedispersedData.deviceToHost(CL_TRUE);
                    } catch (cl::Error &error) {
#pragma omp critical (cerr)
                        std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl;
                        exit(1);
                    }
                }
            }

#pragma omp barrier

#pragma omp master
            if (!profiling)
#pragma omp critical (cout)
                std::cout << "run time = " << omp_get_wtime() - executionStartTime << std::endl;
        }


        UHEP_WorkQueue::UHEP_WorkQueue(UHEP_Pipeline &pipeline, unsigned queueNumber)
            :
        WorkQueue(pipeline, queueNumber),
            pipeline(pipeline),
            hostInputSamples(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY),
            hostBeamFormerWeights(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY),
            hostTriggerInfo(ps.nrTABs(0), queue, CL_MEM_READ_ONLY)
        {
            size_t inputSamplesSize = ps.nrStations() * ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS * ps.nrBytesPerComplexSample();
            size_t complexVoltagesSize = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>);
            size_t transposedDataSize = ps.nrTABs(0) * NR_POLARIZATIONS * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * 512 * sizeof(std::complex<float>);
            size_t invFIRfilteredDataSize = ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * 512 * sizeof(std::complex<float>);

            size_t buffer0size = std::max(inputSamplesSize, transposedDataSize);
            size_t buffer1size = std::max(complexVoltagesSize, invFIRfilteredDataSize);

            devBuffers[0] = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, buffer0size);
            devBuffers[1] = cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, buffer1size);

            size_t beamFormerWeightsSize = ps.nrStations() * ps.nrSubbands() * ps.nrTABs(0) * sizeof(std::complex<float>);
            devBeamFormerWeights = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, beamFormerWeightsSize);

            devInputSamples = devBuffers[0];
            devComplexVoltages = devBuffers[1];

            devReverseSubbandMapping = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, 512 * sizeof(int));
            devInvFIRfilterWeights = cl::Buffer(pipeline.context, CL_MEM_READ_ONLY, 1024 * NR_STATION_FILTER_TAPS * sizeof(float));
            devFFTedData = devBuffers[0];
            devInvFIRfilteredData = devBuffers[1];

            devTriggerInfo = cl::Buffer(pipeline.context, CL_MEM_WRITE_ONLY, ps.nrTABs(0) * sizeof(TriggerInfo));
        }


        void UHEP_WorkQueue::doWork(const float * /*delaysAtBegin*/, const float * /*delaysAfterEnd*/, const float * /*phaseOffsets*/)
        {
            UHEP_BeamFormerKernel beamFormer(ps, pipeline.beamFormerProgram, devComplexVoltages, devInputSamples, devBeamFormerWeights);
            UHEP_TransposeKernel  transpose(ps, pipeline.transposeProgram, devFFTedData, devComplexVoltages, devReverseSubbandMapping);
            UHEP_InvFFT_Kernel	invFFT(ps, pipeline.invFFTprogram, devFFTedData);
            UHEP_InvFIR_Kernel	invFIR(ps, queue, pipeline.invFIRfilterProgram, devInvFIRfilteredData, devFFTedData, devInvFIRfilterWeights);
            UHEP_TriggerKernel	trigger(ps, pipeline.triggerProgram, devTriggerInfo, devInvFIRfilteredData);
            double startTime = ps.startTime(), stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();
            unsigned nrBlocks = (stopTime - startTime) / blockTime;

            queue.enqueueWriteBuffer(devInvFIRfilterWeights, CL_FALSE, 0, sizeof invertedStationPPFWeights, invertedStationPPFWeights);
            queue.enqueueWriteBuffer(devReverseSubbandMapping, CL_TRUE, 0, 512 * sizeof(int), reverseSubbandMapping);

#pragma omp barrier

            double executionStartTime = omp_get_wtime();

#pragma omp for schedule(dynamic), nowait
            for (unsigned block = 0; block < nrBlocks; block ++) {
                try {
                    double currentTime = startTime + block * blockTime;

                    //#pragma omp single nowait // FIXME: why does the compiler complain here???
#pragma omp critical (cout)
                    std::cout << "block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)) << std::endl;

#if 0
                    {
#if defined USE_B7015
                        OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
                        queue.enqueueWriteBuffer(devInputSamples, CL_TRUE, 0, sampledDataSize, hostInputSamples.origin(), 0, &samplesEvent);
                    }
#endif

                    queue.enqueueWriteBuffer(devBeamFormerWeights, CL_FALSE, 0, hostBeamFormerWeights.bytesize(), hostBeamFormerWeights.origin(), 0, &beamFormerWeightsEvent);
                    pipeline.beamFormerWeightsCounter.doOperation(beamFormerWeightsEvent, 0, 0, hostBeamFormerWeights.bytesize());

                    queue.enqueueWriteBuffer(devInputSamples, CL_FALSE, 0, hostInputSamples.bytesize(), hostInputSamples.origin(), 0, &inputSamplesEvent);
                    pipeline.samplesCounter.doOperation(inputSamplesEvent, 0, 0, hostInputSamples.bytesize());

                    beamFormer.enqueue(queue, pipeline.beamFormerCounter);
                    transpose.enqueue(queue, pipeline.transposeCounter);
                    invFFT.enqueue(queue, pipeline.invFFTcounter);
                    invFIR.enqueue(queue, pipeline.invFIRfilterCounter);
                    trigger.enqueue(queue, pipeline.triggerCounter);
                    queue.finish(); // necessary to overlap I/O & computations ???
                    queue.enqueueReadBuffer(devTriggerInfo, CL_TRUE, 0, hostTriggerInfo.size() * sizeof(TriggerInfo), &hostTriggerInfo[0]);
                } catch (cl::Error &error) {
#pragma omp critical (cerr)
                    std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl;
                    exit(1);
                }
            }

#pragma omp barrier

#pragma omp master
            if (!profiling)
#pragma omp critical (cout)
                std::cout << "run time = " << omp_get_wtime() - executionStartTime << std::endl;
        }


#if defined USE_TEST_DATA

        void CorrelatorWorkQueue::setTestPattern()
        {
            if (ps.nrStations() >= 3) {
                double centerFrequency = 384 * ps.nrSamplesPerSubband();
                double baseFrequency = centerFrequency - .5 * ps.nrSamplesPerSubband();
                unsigned testSignalChannel = ps.nrChannelsPerSubband() >= 231 ? 230 : ps.nrChannelsPerSubband() / 2;
                double signalFrequency = baseFrequency + testSignalChannel * ps.nrSamplesPerSubband() / ps.nrChannelsPerSubband();

                for (unsigned time = 0; time < (NR_TAPS - 1 + ps.nrSamplesPerChannel()) * ps.nrChannelsPerSubband(); time ++) {
                    double phi = 2.0 * M_PI * signalFrequency * time / ps.nrSamplesPerSubband();

                    switch (ps.nrBytesPerComplexSample()) {
                    case 4 : reinterpret_cast<std::complex<short> &>(inputSamples[2][time][1][0]) = std::complex<short>((short) rint(32767 * cos(phi)), (short) rint(32767 * sin(phi)));
                        break;

                    case 2 : reinterpret_cast<std::complex<signed char> &>(inputSamples[2][time][1][0]) = std::complex<signed char>((signed char) rint(127 * cos(phi)), (signed char) rint(127 * sin(phi)));
                        break;
                    }
                }
            }
        }


        void CorrelatorWorkQueue::printTestOutput()
        {
            if (ps.nrBaselines() >= 6)
#pragma omp critical (cout)
            {
                std::cout << "newgraph newcurve linetype solid pts" << std::endl;

                //for (int channel = 0; channel < ps.nrChannelsPerSubband(); channel ++)
                if (ps.nrChannelsPerSubband() == 256)
                    for (int channel = 228; channel <= 232; channel ++)
                        std::cout << channel << ' ' << visibilities[5][channel][1][1] << std::endl;
            }
        }

#endif


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
                    try
                    {
                        CorrelatorWorkQueue(*this, omp_get_thread_num()).doWork();
                    } catch (cl::Error &error) {
#pragma omp critical (cerr)
                        std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl;
                        exit(1);
                    }
                }
            }
        }


        void BeamFormerPipeline::doWork()
        {
#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
            try
            {
                BeamFormerWorkQueue(*this, omp_get_thread_num()).doWork();
            } catch (cl::Error &error) {
#pragma omp critical (cerr)
                std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl;
                exit(1);
            }
        }


        void UHEP_Pipeline::doWork()
        {
            float delaysAtBegin[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));
            float delaysAfterEnd[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));
            float phaseOffsets[ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));

            memset(delaysAtBegin, 0, sizeof delaysAtBegin);
            memset(delaysAfterEnd, 0, sizeof delaysAfterEnd);
            memset(phaseOffsets, 0, sizeof phaseOffsets);
            delaysAtBegin[0][2][0] = 1e-6, delaysAfterEnd[0][2][0] = 1.1e-6;

#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
            try
            {
                UHEP_WorkQueue(*this, omp_get_thread_num()).doWork(&delaysAtBegin[0][0][0], &delaysAfterEnd[0][0][0], &phaseOffsets[0][0]);
            } catch (cl::Error &error) {
#pragma omp critical (cerr)
                std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl;
                exit(1);
            }
        }

        




    } // namespace RTCP
} // namespace LOFAR


void usage(char **argv)
{
    std::cerr << "usage: " << argv[0] << " parset" <<  " [correlator|beam|UHEP]" << std::endl;
}

enum SELECTPIPELINE { correlator, beam, UHEP,unittest};

// Coverts the input argument from string to a valid 'function' name
SELECTPIPELINE to_select_pipeline(char *argument)
{
    if (!strcmp(argument,"correlator")) 
        return correlator;

    if (!strcmp(argument,"beam")) 
        return beam;

    if (!strcmp(argument,"UHEP")) 
        return UHEP;

    std::cout << "incorrect third argument supplied." << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    //Allow usage of nested omp calls
    omp_set_nested(true);

    using namespace LOFAR::RTCP;

    INIT_LOGGER("RTCP");
    std::cout << "running ..." << std::endl;

    // Set parts of the environment
    if (setenv("DISPLAY", ":0", 1) < 0)
    {
        perror("error setting DISPLAY");
        exit(1);
    }

#if 0 && defined __linux__
    set_affinity(0); //something with processor affinity, define at start of rtcp
#endif


    // Display usage on incorrect number parameters
    if (argc < 2)
    {
        usage(argv);
        exit(1);
    }

    // Run the actual code in a try block
    try 
    {       
        // Parse the type of computation to perform
        // TODO: place holder enum for types of pipelines and unittest: Should we use propper argument parsing?
        SELECTPIPELINE option;
        if (argc == 3)
            option = to_select_pipeline(argv[2]);
        else
            option = correlator;

        // Create a parameters set object based on the inputs
        Parset ps(argv[1]);

        // Set the number of stations: Code is currently non functional
        //bool set_num_stations = false;
        //if (set_num_stations)
        //{
        //    const char *str = getenv("NR_STATIONS");
        //    ps.nrStations() = str ? atoi(str) : 77;
        //}
        std::cout << "nr stations = " << ps.nrStations() << std::endl;        

        // Select number of GPUs to run on



        // use a switch to select between modes
        switch (option)
        {
        case correlator:
            std::cout << "We are in the correlator part of the code." << std::endl;
            profiling = false; 
            CorrelatorPipeline(ps).doWork();

            profiling = true; 
            CorrelatorPipeline(ps).doWork();
            break;

        case beam:
            std::cout << "We are in the beam part of the code." << std::endl;
            profiling = false; 
            BeamFormerPipeline(ps).doWork();

            profiling = true; 
            BeamFormerPipeline(ps).doWork();
            break;

        case UHEP:
            std::cout << "We are in the UHEP part of the code." << std::endl;
            profiling = false;
            UHEP_Pipeline(ps).doWork();
            profiling = true;
            UHEP_Pipeline(ps).doWork();
            break;

        default:
            std::cout << "None of the types matched, do nothing" << std::endl;
        }
    } 
    catch (cl::Error &error)
    {
#pragma omp critical (cerr)
        std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl;
        exit(1);
    }

    return 0;
}

