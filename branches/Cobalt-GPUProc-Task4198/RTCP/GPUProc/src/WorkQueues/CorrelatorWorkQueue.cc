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
#include "Pipelines/CorrelatorPipelineCounters.h"
#include "FilterBank.h"
#include "Interface/Parset.h"

namespace LOFAR
{
    namespace  RTCP 
    {     
        CorrelatorWorkQueue::CorrelatorWorkQueue(const Parset	&parset,
            cl::Context context, cl::Device	&device, unsigned gpuNumber,
            CorrelatorPipelinePrograms & programs, CorrelatorPipelineCounters & inputcounters,
            FilterBank &filterBank
            )
            :
        WorkQueue( context, device, gpuNumber, parset),
            counters(inputcounters),
            devFIRweights(context, CL_MEM_READ_ONLY, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float)),
            devBufferA(context, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>)),
            devBufferB(context, CL_MEM_READ_WRITE, std::max(ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>), ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>))),
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
          //counters = inputcounters;
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

        //void CorrelatorWorkQueue::sendSubbandVisibilites(unsigned block, unsigned subband)
        //{
        //    
        //    pipeline.GPUtoStorageStreams[subband]->write(visibilities.origin(), visibilities.num_elements() * sizeof(std::complex<float>));
        //    
        //}


        void CorrelatorWorkQueue::doSubband(unsigned block, unsigned subband)
        {
            {
#if defined USE_B7015
                OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
                inputSamples.hostToDevice(CL_TRUE);
                counters.samplesCounter.doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
            }

            // Moved from doWork() The delay data should be available before the kernels start.
            // Queue processed ordered. This could main that the transfer is not nicely overlapped

            delaysAtBegin.hostToDevice(CL_FALSE);
            delaysAfterEnd.hostToDevice(CL_FALSE);
            phaseOffsets.hostToDevice(CL_FALSE);

            if (ps.nrChannelsPerSubband() > 1) {
                firFilterKernel.enqueue(queue, counters.firFilterCounter);
                fftKernel.enqueue(queue, counters.fftCounter);
            }

            delayAndBandPassKernel.enqueue(queue, counters.delayAndBandPassCounter, subband);
#if defined USE_NEW_CORRELATOR
            correlateTriangleKernel.enqueue(queue, counters.correlateTriangleCounter);
            correlateRectangleKernel.enqueue(queue, counters.correlateRectangleCounter);
#else
            correlatorKernel.enqueue(queue, counters.correlatorCounter);
#endif
            queue.finish();

            {
#if defined USE_B7015
                OMP_ScopedLock scopedLock(pipeline.deviceToHostLock[gpu / 2]);
#endif
                visibilities.deviceToHost(CL_TRUE);
                counters.visibilitiesCounter.doOperation(visibilities.event, 0, visibilities.bytesize(), 0);
            }

            
        }
    }
}
