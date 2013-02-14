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
#include "FFT_Kernel.h"
#include "FFT_Plan.h"

#if defined __linux__
#include <sched.h>
#include <sys/time.h>
#endif

namespace LOFAR {
    namespace RTCP {

        extern bool profiling;  //moved to global defines
        unsigned nrGPUs;

        //#define NR_BITS_PER_SAMPLE	 8
//#define NR_POLARIZATIONS	 2
//#define NR_TAPS			16
//#define NR_STATION_FILTER_TAPS	16

#undef USE_INPUT_SECTION
        //#define USE_INPUT_SECTION
//#define USE_2X2
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

        //Moved to seperate cc and h file
        //cl::Program createProgram(const Parset &ps, cl::Context &context, std::vector<cl::Device> &devices, const char *sources)

        template <typename SAMPLE_TYPE> class StationInput
        {
        public:
            SmartPtr<InputSection<SAMPLE_TYPE> >               inputSection;
            SmartPtr<BeamletBufferToComputeNode<SAMPLE_TYPE> > beamletBufferToComputeNode;

            void init(const Parset &ps, unsigned psetNumber);
        };

        template<typename SAMPLE_TYPE> void StationInput<SAMPLE_TYPE>::init(const Parset &ps, unsigned psetNumber)
        {
            string stationName = ps.getStationNamesAndRSPboardNumbers(psetNumber)[0].station; // TODO: support more than one station
            std::vector<Parset::StationRSPpair> inputs = ps.getStationNamesAndRSPboardNumbers(psetNumber);

            inputSection = new InputSection<SAMPLE_TYPE>(ps, inputs);
            beamletBufferToComputeNode = new BeamletBufferToComputeNode<SAMPLE_TYPE>(ps, stationName, inputSection->itsBeamletBuffers, 0);
        }

        
        class Filter_FFT_Kernel : public FFT_Kernel
        {
        public:
            Filter_FFT_Kernel(const Parset &ps, cl::Context &context, cl::Buffer &devFilteredData)
                :
            FFT_Kernel(context, ps.nrChannelsPerSubband(), ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel(), true, devFilteredData)
            {
            }
        };


#if 0
        class Dedispersion_FFT_Kernel
        {
        public:
            Dedispersion_FFT_Kernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer)
                :
            ps(ps),
                plan(context, ps.dedispersionFFTsize()),
                buffer(buffer)
            {
                ASSERT(ps.nrSamplesPerChannel() % ps.dedispersionFFTsize() == 0);
            }

            void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter, clFFT_Direction direction)
            {
                size_t nrFFTs = (size_t) ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() / ps.dedispersionFFTsize();

                cl_int error = clFFT_ExecuteInterleaved(queue(), plan.plan, nrFFTs, direction, buffer(), buffer(), 0, 0, &event());

                if (error != CL_SUCCESS)
                    throw cl::Error(error, "clFFT_ExecuteInterleaved");

                counter.doOperation(event,
                    nrFFTs * 5 * ps.dedispersionFFTsize() * log2(ps.dedispersionFFTsize()),
                    nrFFTs * ps.dedispersionFFTsize() * sizeof(std::complex<float>),
                    nrFFTs * ps.dedispersionFFTsize() * sizeof(std::complex<float>));
            }

        private:
            const Parset &ps;
            FFT_Plan	 plan;
            cl::Buffer	 &buffer;
            cl::Event	 event;
        };
#else
        class DedispersionForwardFFTkernel : public FFT_Kernel
        {
        public:
            DedispersionForwardFFTkernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer)
                :
            FFT_Kernel(context, ps.dedispersionFFTsize(), ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() / ps.dedispersionFFTsize(), true, buffer)
            {
                ASSERT(ps.nrSamplesPerChannel() % ps.dedispersionFFTsize() == 0);
            }
        };


        class DedispersionBackwardFFTkernel : public FFT_Kernel
        {
        public:
            DedispersionBackwardFFTkernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer)
                :
            FFT_Kernel(context, ps.dedispersionFFTsize(), ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() / ps.dedispersionFFTsize(), false, buffer)
            {
                ASSERT(ps.nrSamplesPerChannel() % ps.dedispersionFFTsize() == 0);
            }
        };
#endif
  
        

        class Pipeline
        {
        public:
            Pipeline(const Parset &);

            cl::Program		    createProgram(const char *sources);

            const Parset	    &ps;
            cl::Context		    context;
            std::vector<cl::Device> devices;

            std::vector<StationInput<i16complex> > stationInputs16; // indexed by station
            std::vector<StationInput<i8complex> >  stationInputs8; // indexed by station
            std::vector<StationInput<i4complex> >  stationInputs4; // indexed by station

            std::vector<SmartPtr<Stream> >  bufferToGPUstreams; // indexed by station
            std::vector<SmartPtr<Stream> >  GPUtoStorageStreams; // indexed by subband
            SlidingPointer<uint64_t> inputSynchronization, outputSynchronization;

#if defined USE_B7015
            OMP_Lock hostToDeviceLock[4], deviceToHostLock[4];
#endif

            //private:
            void                    sendNextBlock(unsigned station);
        };


        class CorrelatorWorkQueue;

        class CorrelatorPipeline : public Pipeline
        {
        public:
            CorrelatorPipeline(const Parset &);

            void		    doWork();

        private:
            friend class CorrelatorWorkQueue;

            FilterBank		    filterBank;

            cl::Program		    firFilterProgram, delayAndBandPassProgram, correlatorProgram;
#if defined USE_NEW_CORRELATOR
            PerformanceCounter	    firFilterCounter, delayAndBandPassCounter, correlateTriangleCounter, correlateRectangleCounter, fftCounter;
#else
            PerformanceCounter	    firFilterCounter, delayAndBandPassCounter, correlatorCounter, fftCounter;
#endif
            PerformanceCounter	    samplesCounter, visibilitiesCounter;
        };


        class BeamFormerPipeline : public Pipeline
        {
        public:
            BeamFormerPipeline(const Parset &);

            void		    doWork();

            cl::Program		    intToFloatProgram, delayAndBandPassProgram, beamFormerProgram, transposeProgram, dedispersionChirpProgram;

            PerformanceCounter	    intToFloatCounter, fftCounter, delayAndBandPassCounter, beamFormerCounter, transposeCounter, dedispersionForwardFFTcounter, dedispersionChirpCounter, dedispersionBackwardFFTcounter;
            PerformanceCounter	    samplesCounter;
        };


        class UHEP_Pipeline : public Pipeline
        {
        public:
            UHEP_Pipeline(const Parset &);

            void		    doWork();

            cl::Program		    beamFormerProgram, transposeProgram, invFFTprogram, invFIRfilterProgram, triggerProgram;
            PerformanceCounter	    beamFormerCounter, transposeCounter, invFFTcounter, invFIRfilterCounter, triggerCounter;
            PerformanceCounter	    beamFormerWeightsCounter, samplesCounter;
        };


        Pipeline::Pipeline(const Parset &ps)
            :
        ps(ps),
            stationInputs16(ps.nrStations()),
            stationInputs8(ps.nrStations()),
            stationInputs4(ps.nrStations()),
            bufferToGPUstreams(ps.nrStations()),
            GPUtoStorageStreams(ps.nrSubbands())
        {
            createContext(context, devices);

#ifdef USE_INPUT_SECTION
            for (unsigned stat = 0; stat < ps.nrStations(); stat ++) {
                bufferToGPUstreams[stat] = new SharedMemoryStream;

                switch (ps.nrBitsPerSample()) {
                default:
                case 16:
                    stationInputs16[stat].init(ps, stat);
                    break;

                case 8:
                    stationInputs8[stat].init(ps, stat);
                    break;

                case 4:
                    stationInputs4[stat].init(ps, stat);
                    break;
                }
            }
#else
            for (unsigned stat = 0; stat < ps.nrStations(); stat ++)
                bufferToGPUstreams[stat] = new NullStream;
#endif

            for (unsigned sb = 0; sb < ps.nrSubbands(); sb ++)
                GPUtoStorageStreams[sb] = new NullStream;
        }


        cl::Program Pipeline::createProgram(const char *sources)
        {
            return LOFAR::RTCP::createProgram(ps, context, devices, sources);
        }


        void Pipeline::sendNextBlock(unsigned station)
        {
            (void)station;
#ifdef USE_INPUT_SECTION
            unsigned bitsPerSample = ps.nrBitsPerSample();

            Stream *stream = bufferToGPUstreams[station];

            switch(bitsPerSample) {
            default:
            case 16:
                stationInputs16[station].beamletBufferToComputeNode->process(stream);
                break;

            case 8:
                stationInputs8[station].beamletBufferToComputeNode->process(stream);
                break;

            case 4:
                stationInputs4[station].beamletBufferToComputeNode->process(stream);
                break;
            }
#endif
        }


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


        UHEP_Pipeline::UHEP_Pipeline(const Parset &ps)
            :
        Pipeline(ps),
            beamFormerCounter("beamformer"),
            transposeCounter("transpose"),
            invFFTcounter("inv. FFT"),
            invFIRfilterCounter("inv. FIR"),
            triggerCounter("trigger"),
            beamFormerWeightsCounter("BF weights"),
            samplesCounter("samples")
        {
            double startTime = omp_get_wtime();

#pragma omp parallel sections
            {
#pragma omp section
                beamFormerProgram = createProgram("UHEP/BeamFormer.cl");
#pragma omp section
                transposeProgram = createProgram("UHEP/Transpose.cl");
#pragma omp section
                invFFTprogram = createProgram("UHEP/InvFFT.cl");
#pragma omp section
                invFIRfilterProgram = createProgram("UHEP/InvFIR.cl");
#pragma omp section
                triggerProgram = createProgram("UHEP/Trigger.cl");
            }

            std::cout << "compile time = " << omp_get_wtime() - startTime << std::endl;
        }


        class WorkQueue
        {
        public:
            WorkQueue(Pipeline &, unsigned queueNumber);

            const unsigned	gpu;
            cl::Device		&device;
            cl::CommandQueue	queue;

        protected:
            const Parset	&ps;
        };


        class CorrelatorWorkQueue : public WorkQueue
        {
        public:
            CorrelatorWorkQueue(CorrelatorPipeline &, unsigned queueNumber);

            void doWork();

#if defined USE_TEST_DATA
            void setTestPattern();
            void printTestOutput();
#endif

            //private:
            void doSubband(unsigned block, unsigned subband);
            void receiveSubbandSamples(unsigned block, unsigned subband);
            void sendSubbandVisibilites(unsigned block, unsigned subband);

            CorrelatorPipeline	&pipeline;
            cl::Buffer		devFIRweights;
            cl::Buffer		devBufferA, devBufferB;
            MultiArraySharedBuffer<float, 1> bandPassCorrectionWeights;
            MultiArraySharedBuffer<float, 3> delaysAtBegin, delaysAfterEnd;
            MultiArraySharedBuffer<float, 2> phaseOffsets;
            MultiArraySharedBuffer<char, 4> inputSamples;

            cl::Buffer		devFilteredData;
            cl::Buffer		devCorrectedData;

            MultiArraySharedBuffer<std::complex<float>, 4> visibilities;

            FIR_FilterKernel		firFilterKernel;
            Filter_FFT_Kernel		fftKernel;
            DelayAndBandPassKernel	delayAndBandPassKernel;
#if defined USE_NEW_CORRELATOR
            CorrelateTriangleKernel	correlateTriangleKernel;
            CorrelateRectangleKernel	correlateRectangleKernel;
#else
            CorrelatorKernel		correlatorKernel;
#endif
        };


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


        WorkQueue::WorkQueue(Pipeline &pipeline, unsigned queueNumber)
            :
        gpu(queueNumber % nrGPUs),
            device(pipeline.devices[gpu]),
            ps(pipeline.ps)
        {
#if defined __linux__ && defined USE_B7015
            set_affinity(gpu);
#endif

            queue = cl::CommandQueue(pipeline.context, device, profiling ? CL_QUEUE_PROFILING_ENABLE : 0);
        }


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
                    try {
                        doSubband(block, subband);
                    } catch (cl::Error &error) {
#pragma omp critical (cerr)
                        std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
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


        // complexVoltages()
        // float2 (*ComplexVoltagesType)[NR_CHANNELS][NR_TIMES_PER_BLOCK][NR_TABS][NR_POLARIZATIONS];
        // transpose()
        //
        // float2 (*DedispersedDataType)[nrTABs][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()];
        // FFT()
        //
        // applyChrip()
        //
        // FFT-1()
        // float2 (*DedispersedDataType)[nrTABs][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()];
        // (*ComplexVoltagesType)[NR_CHANNELS][NR_TIMES_PER_BLOCK][NR_TABS];
        // computeStokes()
        // float (*StokesType)[NR_TABS][NR_STOKES][NR_TIMES_PER_BLOCK / STOKES_INTEGRATION_SAMPLES][NR_CHANNELS];


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
                        std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
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
                    std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
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
                        std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
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
                std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
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
                std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
                exit(1);
            }
        }

        struct CorrelatorTest : public UnitTest
        {
            CorrelatorTest(const Parset &ps)
                :
#if defined USE_NEW_CORRELATOR
            UnitTest(ps, "NewCorrelator.cl")
#else
            UnitTest(ps, "Correlator.cl")
#endif
            {
                if (ps.nrStations() >= 5 && ps.nrChannelsPerSubband() >= 6 && ps.nrSamplesPerChannel() >= 100) {
                    MultiArraySharedBuffer<std::complex<float>, 4> visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    CorrelatorKernel correlator(ps, queue, program, visibilities, inputData);

                    //inputData[3][5][99][1] = std::complex<float>(3, 4);
                    //inputData[4][5][99][1] = std::complex<float>(5, 6);
                    inputData[0][5][99][1] = std::complex<float>(3, 4);
                    inputData[2][5][99][1] = std::complex<float>(5, 6);

                    visibilities.hostToDevice(CL_FALSE);
                    inputData.hostToDevice(CL_FALSE);
                    correlator.enqueue(queue, counter);
                    visibilities.deviceToHost(CL_TRUE);

                    //check(visibilities[13][5][1][1], std::complex<float>(39, 2));
                    //check(visibilities[5463][5][1][1], std::complex<float>(39, 2));
                    for (unsigned bl = 0; bl < ps.nrBaselines(); bl ++)
                        if (visibilities[bl][5][1][1] != std::complex<float>(0, 0))
                            std::cout << "bl = " << bl << ", visibility = " << visibilities[bl][5][1][1] << std::endl;
                }
            }
        };


#if defined USE_NEW_CORRELATOR

        struct CorrelateRectangleTest : public UnitTest
        {
            CorrelateRectangleTest(const Parset &ps)
                :
            //UnitTest(ps, "Correlator.cl")
            UnitTest(ps, "NewCorrelator.cl")
            {
                if (ps.nrStations() >= 5 && ps.nrChannelsPerSubband() >= 6 && ps.nrSamplesPerChannel() >= 100) {
                    MultiArraySharedBuffer<std::complex<float>, 4> visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    CorrelateRectangleKernel correlator(ps, queue, program, visibilities, inputData);

                    inputData[27][5][99][1] = std::complex<float>(3, 4);
                    inputData[68][5][99][1] = std::complex<float>(5, 6);

                    visibilities.hostToDevice(CL_FALSE);
                    inputData.hostToDevice(CL_FALSE);
                    correlator.enqueue(queue, counter);
                    visibilities.deviceToHost(CL_TRUE);

                    //check(visibilities[5463][5][1][1], std::complex<float>(39, 2));
                    for (unsigned bl = 0; bl < ps.nrBaselines(); bl ++)
                        if (visibilities[bl][5][1][1] != std::complex<float>(0, 0))
                            std::cout << "bl = " << bl << ", visibility = " << visibilities[bl][5][1][1] << std::endl;
                }
            }
        };


        struct CorrelateTriangleTest : public UnitTest
        {
            CorrelateTriangleTest(const Parset &ps)
                :
            //UnitTest(ps, "Correlator.cl")
            UnitTest(ps, "NewCorrelator.cl")
            {
                if (ps.nrStations() >= 5 && ps.nrChannelsPerSubband() >= 6 && ps.nrSamplesPerChannel() >= 100) {
                    MultiArraySharedBuffer<std::complex<float>, 4> visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    CorrelateTriangleKernel correlator(ps, queue, program, visibilities, inputData);

                    //inputData[3][5][99][1] = std::complex<float>(3, 4);
                    //inputData[4][5][99][1] = std::complex<float>(5, 6);
                    inputData[0][5][99][1] = std::complex<float>(3, 4);
                    inputData[2][5][99][1] = std::complex<float>(5, 6);

                    visibilities.hostToDevice(CL_FALSE);
                    inputData.hostToDevice(CL_FALSE);
                    correlator.enqueue(queue, counter);
                    visibilities.deviceToHost(CL_TRUE);

                    //check(visibilities[13][5][1][1], std::complex<float>(39, 2));
                    for (unsigned bl = 0; bl < ps.nrBaselines(); bl ++)
                        if (visibilities[bl][5][1][1] != std::complex<float>(0, 0))
                            std::cout << "bl = " << bl << ", visibility = " << visibilities[bl][5][1][1] << std::endl;
                }
            }
        };

#endif


        struct IncoherentStokesTest : public UnitTest
        {
            IncoherentStokesTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/IncoherentStokes.cl")
            {
                if (ps.nrStations() >= 5 && ps.nrChannelsPerSubband() >= 14 && ps.nrSamplesPerChannel() >= 108) {
                    MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<float, 3> stokesData(boost::extents[ps.nrIncoherentStokes()][ps.nrSamplesPerChannel() / ps.incoherentStokesTimeIntegrationFactor()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    IncoherentStokesKernel kernel(ps, queue, program, stokesData, inputData);

                    inputData[4][13][107][0] = std::complex<float>(2, 3);
                    inputData[4][13][107][1] = std::complex<float>(4, 5);

                    inputData.hostToDevice(CL_FALSE);
                    kernel.enqueue(queue, counter);
                    stokesData.deviceToHost(CL_TRUE);

                    const static float expected[] = { 54, -28, 46, 4 };

                    for (unsigned stokes = 0; stokes < ps.nrIncoherentStokes(); stokes ++)
                        check(stokesData[stokes][107 / ps.incoherentStokesTimeIntegrationFactor()][13], expected[stokes]);
                }
            }
        };


        struct IntToFloatTest : public UnitTest
        {
            IntToFloatTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/IntToFloat.cl")
            {
                if (ps.nrStations() >= 3 && ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() >= 10077) {
                    MultiArraySharedBuffer<char, 4> inputData(boost::extents[ps.nrStations()][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 3> outputData(boost::extents[ps.nrStations()][NR_POLARIZATIONS][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    IntToFloatKernel kernel(ps, queue, program, outputData, inputData);

                    switch (ps.nrBytesPerComplexSample()) {
                    case 4 : reinterpret_cast<std::complex<short> &>(inputData[2][10076][1][0]) = 7;
                        break;

                    case 2 : reinterpret_cast<std::complex<signed char> &>(inputData[2][10076][1][0]) = 7;
                        break;

                    case 1 : reinterpret_cast<i4complex &>(inputData[2][10076][1][0]) = i4complex(7, 0);
                        break;
                    }

                    inputData.hostToDevice(CL_FALSE);
                    kernel.enqueue(queue, counter);
                    outputData.deviceToHost(CL_TRUE);
                    check(outputData[2][1][10076], std::complex<float>(7.0f, 0));
                }
            }
        };


        struct BeamFormerTest : public UnitTest
        {
            BeamFormerTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/BeamFormer.cl")
            {
                if (ps.nrStations() >= 5 && ps.nrSamplesPerChannel() >= 13 && ps.nrChannelsPerSubband() >= 7 && ps.nrTABs(0) >= 6) {
                    MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE);
                    BeamFormerKernel beamFormer(ps, program, complexVoltages, inputData, beamFormerWeights);

                    inputData[4][6][12][1] = std::complex<float>(2.2, 3);
                    beamFormerWeights[4][6][5] = std::complex<float>(4, 5);

                    inputData.hostToDevice(CL_FALSE);
                    beamFormerWeights.hostToDevice(CL_FALSE);
                    beamFormer.enqueue(queue, counter);
                    complexVoltages.deviceToHost(CL_TRUE);

                    check(complexVoltages[6][12][5][1], std::complex<float>(-6.2, 23));

#if 0
                    for (unsigned tab = 0; tab < ps.nrTABs(0); tab ++)
                        for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++)
                            for (unsigned ch = 0; ch < ps.nrChannelsPerSubband(); ch ++)
                                for (unsigned t = 0; t < ps.nrSamplesPerChannel(); t ++)
                                    if (complexVoltages[tab][pol][ch][t] != std::complex<float>(0, 0))
                                        std::cout << "complexVoltages[" << tab << "][" << pol << "][" << ch << "][" << t << "] = " << complexVoltages[tab][pol][ch][t] << std::endl;
#endif
                }
            }
        };


        struct BeamFormerTransposeTest : public UnitTest
        {
            BeamFormerTransposeTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/Transpose.cl")
            {
                if (ps.nrChannelsPerSubband() >= 19 && ps.nrSamplesPerChannel() >= 175 && ps.nrTABs(0) >= 5) {
                    MultiArraySharedBuffer<std::complex<float>, 4> transposedData(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
                    BeamFormerTransposeKernel transpose(ps, program, transposedData, complexVoltages);

                    complexVoltages[18][174][4][1] = std::complex<float>(24, 42);

                    complexVoltages.hostToDevice(CL_FALSE);
                    transpose.enqueue(queue, counter);
                    transposedData.deviceToHost(CL_TRUE);

                    check(transposedData[4][1][174][18], std::complex<float>(24, 42));
                }
            }
        };


        struct DedispersionChirpTest : public UnitTest
        {
            DedispersionChirpTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/Dedispersion.cl")
            {
                if (ps.nrTABs(0) > 3 && ps.nrChannelsPerSubband() > 13 && ps.nrSamplesPerChannel() / ps.dedispersionFFTsize() > 1 && ps.dedispersionFFTsize() > 77) {
                    MultiArraySharedBuffer<std::complex<float>, 5> data(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel() / ps.dedispersionFFTsize()][ps.dedispersionFFTsize()], queue, CL_MEM_READ_WRITE, CL_MEM_READ_WRITE);
                    MultiArraySharedBuffer<float, 1> DMs(boost::extents[ps.nrTABs(0)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    DedispersionChirpKernel dedispersionChirpKernel(ps, program, queue, data, DMs);

                    data[3][1][13][1][77] = std::complex<float>(2, 3);
                    DMs[3] = 2;

                    DMs.hostToDevice(CL_FALSE);
                    data.hostToDevice(CL_FALSE);
                    dedispersionChirpKernel.enqueue(queue, counter, 60e6);
                    data.deviceToHost(CL_TRUE);

                    std::cout << data[3][1][13][1][77] << std::endl;
                }
            }
        };


        struct CoherentStokesTest : public UnitTest
        {
            CoherentStokesTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/CoherentStokes.cl")
            {
                if (ps.nrChannelsPerSubband() >= 19 && ps.nrSamplesPerChannel() >= 175 && ps.nrTABs(0) >= 5) {
                    MultiArraySharedBuffer<float, 4> stokesData(boost::extents[ps.nrTABs(0)][ps.nrCoherentStokes()][ps.nrSamplesPerChannel() / ps.coherentStokesTimeIntegrationFactor()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
#if 1
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
                    CoherentStokesKernel stokesKernel(ps, program, stokesData, complexVoltages);

                    complexVoltages[18][174][4][0] = std::complex<float>(2, 3);
                    complexVoltages[18][174][4][1] = std::complex<float>(4, 5);
#else
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
                    CoherentStokesKernel stokesKernel(ps, program, stokesData, complexVoltages);

                    complexVoltages[18][174][4][0] = std::complex<float>(2, 3);
                    complexVoltages[18][174][4][1] = std::complex<float>(4, 5);
#endif

                    complexVoltages.hostToDevice(CL_FALSE);
                    stokesKernel.enqueue(queue, counter);
                    stokesData.deviceToHost(CL_TRUE);

                    for (unsigned stokes = 0; stokes < ps.nrCoherentStokes(); stokes ++)
                        std::cout << stokesData[4][stokes][174 / ps.coherentStokesTimeIntegrationFactor()][18] << std::endl;
                }
            }
        };


        struct UHEP_BeamFormerTest : public UnitTest
        {
            UHEP_BeamFormerTest(const Parset &ps)
                :
            UnitTest(ps, "UHEP/BeamFormer.cl")
            {
                if (ps.nrStations() >= 5 && (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) >= 13 && ps.nrSubbands() >= 7 && ps.nrTABs(0) >= 6) {
                    MultiArraySharedBuffer<char, 5> inputSamples(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights(boost::extents[ps.nrStations()][ps.nrSubbands()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE);
                    UHEP_BeamFormerKernel beamFormer(ps, program, complexVoltages, inputSamples, beamFormerWeights);

                    switch (ps.nrBytesPerComplexSample()) {
                    case 4 : reinterpret_cast<std::complex<short> &>(inputSamples[4][6][12][1][0]) = std::complex<short>(2, 3);
                        break;

                    case 2 : reinterpret_cast<std::complex<signed char> &>(inputSamples[4][6][12][1][0]) = std::complex<signed char>(2, 3);
                        break;

                    case 1 : reinterpret_cast<i4complex &>(inputSamples[4][6][12][1][0]) = i4complex(2, 3);
                        break;
                    }

                    beamFormerWeights[4][6][5] = std::complex<float>(4, 5);

                    inputSamples.hostToDevice(CL_FALSE);
                    beamFormerWeights.hostToDevice(CL_FALSE);
                    beamFormer.enqueue(queue, counter);
                    complexVoltages.deviceToHost(CL_TRUE);

                    check(complexVoltages[6][12][5][1], std::complex<float>(-7, 22));
                }
            }
        };


        struct UHEP_TransposeTest : public UnitTest
        {
            UHEP_TransposeTest(const Parset &ps)
                :
            UnitTest(ps, "UHEP/Transpose.cl")
            {
                if (ps.nrSubbands() >= 19 && ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1 >= 175 && ps.nrTABs(0) >= 5) {
                    MultiArraySharedBuffer<std::complex<float>, 4> transposedData(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][512], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
                    cl::Buffer devReverseSubbandMapping(context, CL_MEM_READ_ONLY, 512 * sizeof(int));
                    UHEP_TransposeKernel transpose(ps, program, transposedData, complexVoltages, devReverseSubbandMapping);

                    complexVoltages[18][174][4][1] = std::complex<float>(24, 42);

                    queue.enqueueWriteBuffer(devReverseSubbandMapping, CL_FALSE, 0, 512 * sizeof(int), reverseSubbandMapping);
                    complexVoltages.hostToDevice(CL_FALSE);
                    transpose.enqueue(queue, counter);
                    transposedData.deviceToHost(CL_TRUE);

                    check(transposedData[4][1][174][38], std::complex<float>(24, 42));
                }
            }
        };


        struct UHEP_TriggerTest : public UnitTest
        {
            UHEP_TriggerTest(const Parset &ps)
                :
            UnitTest(ps, "UHEP/Trigger.cl")
            {
                if (ps.nrTABs(0) >= 4 && 1024 * ps.nrSamplesPerChannel() > 100015) {
                    MultiArraySharedBuffer<float, 3> inputData(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel() * 1024], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<TriggerInfo, 1> triggerInfo(boost::extents[ps.nrTABs(0)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    UHEP_TriggerKernel trigger(ps, program, triggerInfo, inputData);

                    inputData[3][1][100015] = 1000;

                    inputData.hostToDevice(CL_FALSE);
                    trigger.enqueue(queue, counter);
                    triggerInfo.deviceToHost(CL_TRUE);

                    std::cout << "trigger info: mean = " << triggerInfo[3].mean << ", variance = " << triggerInfo[3].variance << ", bestValue = " << triggerInfo[3].bestValue << ", bestApproxIndex = " << triggerInfo[3].bestApproxIndex << std::endl;
                    //check(triggerInfo[3].mean, (float) (1000.0f * 1000.0f) / (float) (ps.nrSamplesPerChannel() * 1024));
                    check(triggerInfo[3].bestValue, 1000.0f * 1000.0f);
                    check(triggerInfo[3].bestApproxIndex, 100016U);
                }
            }
        };


#if 0
        struct FFT_Test : public UnitTest
        {
            FFT_Test(const Parset &ps)
                : UnitTest(ps, "fft.cl")
            {
                MultiArraySharedBuffer<std::complex<float>, 1> in(boost::extents[8], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                MultiArraySharedBuffer<std::complex<float>, 1> out(boost::extents[8], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);

                for (unsigned i = 0; i < 8; i ++)
                    in[i] = std::complex<float>(2 * i + 1, 2 * i + 2);

                clAmdFftSetupData setupData;
                cl::detail::errHandler(clAmdFftInitSetupData(&setupData), "clAmdFftInitSetupData");
                setupData.debugFlags = CLFFT_DUMP_PROGRAMS;
                cl::detail::errHandler(clAmdFftSetup(&setupData), "clAmdFftSetup");

                clAmdFftPlanHandle plan;
                size_t dim[1] = { 8 };

                cl::detail::errHandler(clAmdFftCreateDefaultPlan(&plan, context(), CLFFT_1D, dim), "clAmdFftCreateDefaultPlan");
                cl::detail::errHandler(clAmdFftSetResultLocation(plan, CLFFT_OUTOFPLACE), "clAmdFftSetResultLocation");
                cl::detail::errHandler(clAmdFftSetPlanBatchSize(plan, 1), "clAmdFftSetPlanBatchSize");
                cl::detail::errHandler(clAmdFftBakePlan(plan, 1, &queue(), 0, 0), "clAmdFftBakePlan");

                in.hostToDevice(CL_FALSE);
                cl_mem ins[1] = { ((cl::Buffer) in)() };
                cl_mem outs[1] = { ((cl::Buffer) out)() };
#if 1
                cl::detail::errHandler(clAmdFftEnqueueTransform(plan, CLFFT_FORWARD, 1, &queue(), 0, 0, 0, ins, outs, 0), "clAmdFftEnqueueTransform");
#else
                cl::Kernel kernel(program, "fft_fwd");
                kernel.setArg(0, (cl::Buffer) in);
                kernel.setArg(1, (cl::Buffer) out);
                queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(64, 1, 1), cl::NDRange(64, 1, 1));
#endif
                out.deviceToHost(CL_TRUE);

                for (unsigned i = 0; i < 8; i ++)
                    std::cout << out[i] << std::endl;

                cl::detail::errHandler(clAmdFftDestroyPlan(&plan), "clAmdFftDestroyPlan");
                cl::detail::errHandler(clAmdFftTeardown(), "clAmdFftTeardown");
            }
        };
#endif


    } // namespace RTCP
} // namespace LOFAR


void usage(char **argv)
{
    std::cerr << "usage: " << argv[0] << " parset" <<  " [correlator|beam|UHEP|unittest]" << std::endl;
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

    if (!strcmp(argument,"unittest")) 
        return unittest;

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
        const char *str = getenv("NR_GPUS");
        nrGPUs = str ? atoi(str) : 1;


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

        case unittest:
            std::cout << "We are in the unittest part of the code." << std::endl;
            //(CorrelatorTest)(ps);       //needs parset AARTFAAC!!
            //(CorrelateRectangleTest)(ps); //needs parset AARTFAAC!!

            //works with all parsets
            //Correlate unittest 
            (CorrelateTriangleTest)(ps);

            //UHEP unittest
            (UHEP_BeamFormerTest)(ps);
            (UHEP_TransposeTest)(ps);
            (UHEP_TriggerTest)(ps);

            // beamformed unittest 
            (IncoherentStokesTest)(ps);
            (IntToFloatTest)(ps);
            (BeamFormerTest)(ps);
            (BeamFormerTransposeTest)(ps);
            (DedispersionChirpTest)(ps);
            (CoherentStokesTest)(ps);

            // dunno what test
            //(FFT_Test)(ps);  unknown test
            break;

        default:
            std::cout << "None of the types matched, do nothing" << std::endl;
        }
    } 
    catch (cl::Error &error)
    {
#pragma omp critical (cerr)
        std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl << error;
        exit(1);
    }

    return 0;
}
