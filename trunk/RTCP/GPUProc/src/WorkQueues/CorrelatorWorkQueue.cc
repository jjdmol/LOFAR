#include "lofar_config.h"    

#include "CL/cl.hpp"
#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <algorithm>
#include <iostream>

#include "CorrelatorWorkQueue.h"
#include "BandPass.h"
#include "Pipelines/CorrelatorPipelinePrograms.h"
#include "FilterBank.h"
#include "Interface/Parset.h"

namespace LOFAR
{
    namespace  RTCP 
    {     
        CorrelatorWorkQueue::CorrelatorWorkQueue(const Parset	&parset,
            cl::Context &context, cl::Device	&device, unsigned gpuNumber,
            CorrelatorPipelinePrograms & programs,
            FilterBank &filterBank
            )
            :
        WorkQueue( context, device, gpuNumber, parset),
            devFIRweights(context, CL_MEM_READ_ONLY, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float)),
            devBufferA(context, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>)),
            devBufferB(context, CL_MEM_READ_WRITE,
                       std::max(ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>),
                       ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>))),
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
            firFilterKernel(ps, queue, programs.firFilterProgram, devFilteredData, inputSamples, devFIRweights),

            fftKernel(ps, context, devFilteredData),
            delayAndBandPassKernel(ps, programs.delayAndBandPassProgram, devCorrectedData, devFilteredData, delaysAtBegin, delaysAfterEnd, phaseOffsets, bandPassCorrectionWeights),
#if defined USE_NEW_CORRELATOR
            correlateTriangleKernel(ps, queue, programs.correlatorProgram, visibilities, devCorrectedData),
            correlateRectangleKernel(ps, queue, programs.correlatorProgram, visibilities, devCorrectedData)
#else
            correlatorKernel(ps, queue, programs.correlatorProgram, visibilities, devCorrectedData)
#endif
        {
            // create all the counters
#if defined USE_NEW_CORRELATOR
            addCounter("cor.triangle");
            addCounter("cor.rectangle");
#else
            addCounter("correlator");
#endif

            addCounter("FIR");
            addCounter("delay/bp");
            addCounter("FFT");
            addCounter("samples");
            addCounter("visibilities");

            queue.enqueueWriteBuffer(devFIRweights, CL_TRUE, 0, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float), filterBank.getWeights().origin());

#if 0
            size_t filteredDataSize = ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>);
            devFilteredData = cl::Buffer(context, CL_MEM_READ_WRITE, filteredDataSize);
            devCorrectedData = cl::Buffer(context, CL_MEM_READ_WRITE, filteredDataSize);
#endif

            if (ps.correctBandPass()) {
                BandPass::computeCorrectionFactors(bandPassCorrectionWeights.origin(), ps.nrChannelsPerSubband());
                bandPassCorrectionWeights.hostToDevice(CL_TRUE);
            }
        }


        void CorrelatorWorkQueue::doSubband(unsigned block, unsigned subband)
        {
            {
#if defined USE_B7015
                OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
                inputSamples.hostToDevice(CL_TRUE);
                counters["samples"]->doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
            }

            // Moved from doWork() The delay data should be available before the kernels start.
            // Queue processed ordered. This could main that the transfer is not nicely overlapped

            delaysAtBegin.hostToDevice(CL_FALSE);
            delaysAfterEnd.hostToDevice(CL_FALSE);
            phaseOffsets.hostToDevice(CL_FALSE);

            if (ps.nrChannelsPerSubband() > 1) {
                firFilterKernel.enqueue(queue, *counters["FIR"]);
                fftKernel.enqueue(queue, *counters["FFT"]);
            }

            delayAndBandPassKernel.enqueue(queue, *counters["delay/bp"], subband);
#if defined USE_NEW_CORRELATOR
            correlateTriangleKernel.enqueue(queue, *counters["cor.triangle"]);
            correlateRectangleKernel.enqueue(queue, *counters["cor.rectangle"]);
#else
            correlatorKernel.enqueue(queue, *counters["correlator"]);
#endif
            queue.finish();

            {
#if defined USE_B7015
                OMP_ScopedLock scopedLock(pipeline.deviceToHostLock[gpu / 2]);
#endif
                visibilities.deviceToHost(CL_TRUE);
                counters["visibilities"]->doOperation(visibilities.event, 0, visibilities.bytesize(), 0);
            }
        }
    }
}
