#include "lofar_config.h"

#include "Common/LofarLogger.h"
#include "CL/cl.hpp"

#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <algorithm>
#include <iostream>

#include "CorrelatorWorkQueue.h"
#include "BandPass.h"
#include "Pipelines/CorrelatorPipelinePrograms.h"
#include "FilterBank.h"

#include "Common/LofarLogger.h"
#include <Input/BeamletBufferToComputeNode.h>
#include <SubbandMetaData.h>

namespace LOFAR
{
  namespace  RTCP
  {
    CorrelatorWorkQueue::CorrelatorWorkQueue(const Parset       &parset,
                                             cl::Context &context,
                                             cl::Device  &device,
                                             unsigned gpuNumber,
                                             CorrelatorPipelinePrograms & programs,
                                             FilterBank &filterBank)
      :
      WorkQueue( context, device, gpuNumber, parset),     
      devCorrectedData(context, 
                       CL_MEM_READ_WRITE, 
                       ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>)),
      devFilteredData(context,
                      CL_MEM_READ_WRITE,
                      std::max(ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>),
                      ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>))),

      inputData(ps.nrBeams(),
                ps.nrStations(),
                NR_POLARIZATIONS,
                (ps.nrSamplesPerChannel() + NR_TAPS - 1) * ps.nrChannelsPerSubband(),  //n_samples in a received datablock
                ps.nrBytesPerComplexSample(),
                queue,
                devCorrectedData),
      visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, devFilteredData),
      devFIRweights(context, CL_MEM_READ_ONLY, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float)),
      firFilterKernel(ps, queue, programs.firFilterProgram, devFilteredData, inputData.inputSamples, devFIRweights),
      fftKernel(ps, context, devFilteredData),
      bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()],
                                queue,
                                CL_MEM_WRITE_ONLY,
                                CL_MEM_READ_ONLY),
      delayAndBandPassKernel(ps, 
                             programs.delayAndBandPassProgram, 
                             devCorrectedData,
                             devFilteredData, 
                             inputData.delaysAtBegin,
                             inputData.delaysAfterEnd,
                             inputData.phaseOffsets,
                             bandPassCorrectionWeights),
#if defined USE_NEW_CORRELATOR
      correlateTriangleKernel(ps, 
                              queue, 
                              programs.correlatorProgram,
                              visibilities, 
                              devCorrectedData),
      correlateRectangleKernel(ps,
                               queue,
                               programs.correlatorProgram, 
                               visibilities,
                               devCorrectedData)
#else
      correlatorKernel(ps,
                       queue,
                       programs.correlatorProgram,
                       visibilities,
                       devCorrectedData)
#endif
    {
      // create all the counters
      // Move the FIR filter weight to the GPU
#if defined USE_NEW_CORRELATOR
      addCounter("compute - cor.triangle");
      addCounter("compute - cor.rectangle");
#else
      addCounter("compute - correlator");
#endif

      addCounter("compute - FIR");
      addCounter("compute - delay/bp");
      addCounter("compute - FFT");
      addCounter("input - samples");
      addCounter("output - visibilities");

      // CPU timers are set by CorrelatorPipeline
      addTimer("CPU - total");
      addTimer("CPU - input");
      addTimer("CPU - output");
      addTimer("CPU - compute");

      // GPU timers are set by us
      addTimer("GPU - total");
      addTimer("GPU - input");
      addTimer("GPU - output");
      addTimer("GPU - compute");
      addTimer("GPU - wait");

      queue.enqueueWriteBuffer(devFIRweights, CL_TRUE, 0, ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float), filterBank.getWeights().origin());

      if (ps.correctBandPass())
      {
        BandPass::computeCorrectionFactors(bandPassCorrectionWeights.origin(), ps.nrChannelsPerSubband());
        bandPassCorrectionWeights.hostToDevice(CL_TRUE);
      }
    }
      
      void calculate_the_flags(const Parset &parset)
      {
      }
        ////An array to store the flags converted to channels
        //// This is a temporary scratch container: should be changed to a list??
        //MultiDimArray<SparseSet<unsigned>, parset.nrChannelsPerSubband()> flagsPerChanel(boost::extents[parset.nrStations()][parset.nrChannelsPerSubband()]);

        //// Calculate the log thingy of the number of channels. This is needed to convert the 
        //// ranges to the different channels
        //unsigned logNrChannels;
        //for (logNrChannels = 0; 1U << logNrChannels != logNrChannels; logNrChannels ++)
        //  ;

        //// now loop all stations
        //for (unsigned stat = 0; stat < parset.nrStations(); stat ++) 
        //{
        //  // get the flag ranges
        //  const SparseSet<unsigned>::Ranges &ranges = inputSamplesFlaggedRangesPerStation[stat];
        //  // loop
        //  for (SparseSet<unsigned>::const_iterator it = ranges.begin(); it != ranges.end(); it ++) 
        //  {
        //    // Magic
        //    unsigned begin = ps.nrChannelsPerSubband() == 1 ? it->begin : std::max(0, (signed) (it->begin >> logNrChannels) - NR_TAPS + 1);
        //    unsigned end   = std::min(itsNrSamplesPerIntegration, ((it->end - 1) >> logNrChannels) + 1);

        //    for (unsigned ch = 0; ch < itsNrChannels; ch++) 
        //    {
        //      flagsPerChanel[ch][stat].include(begin, end);
        //    }
        //  }
        //}

    //  // loop the stations
    //  for (unsigned stat2 = 0; stat2 < ps.nrStations(); stat2 ++) 
    //  {
    //    for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) 
    //    {
    //      //TODO:  Calculate the station, should be moved to helper function
    //      unsigned bl  =  stat2 * (stat2 + 1) / 2 + stat1 ; //baseline(stat1, stat2); This function should be moved to a helper class

    //      // TODO: is this correct? jan-David please validate
    //      unsigned nrSamplesPerIntegration = (ps.nrSamplesPerChannel() + NR_TAPS - 1) * ps.nrChannelsPerSubband(); 

    //      // If there is a single channel then the index 0 contains real data
    //      if (ps.nrChannelsPerSubband() == 1) 
    //      {                                            
    //        //The number of invalid (flagged) samples is the union of the flagged samples in the two stations
    //        unsigned nrValidSamples = nrSamplesPerIntegration -
    //          (inputSamplesFlaggedRangesPerStation[stat1] |
    //          inputSamplesFlaggedRangesPerStation[stat2]).count();
    //        fractionFlaggedPerBaseline[bl][0] = double(nrValidSamples) / nrSamplesPerIntegration;        // This is a overloaded bitwise compare.
    //      } 
    //      else //otherwise the index 0 contains garbage
    //      {
    //        // channel 0 does not contain valid data
    //        fractionFlaggedPerBaseline[bl][0] = 1.0; 

    //        // TODO: is this channel needed for at the entry point we dont have channels??
    //        for (unsigned ch = 1; ch < ps.nrChannelsPerSubband(); ch ++) 
    //        {
    //          unsigned nrValidSamples = nrSamplesPerIntegration -
    //            (inputSamplesFlaggedRangesPerStation[stat1] |
    //            inputSamplesFlaggedRangesPerStation[stat2]).count();
    //          fractionFlaggedPerBaseline[bl][ch] = double(nrValidSamples) / nrSamplesPerIntegration;
    //        }
    //      }
    //    }


    //}


    void computeFlags(const Parset& parset,
                      WorkQueueInputData& inputData,
                      CorrelatedData &output)
    {
      size_t nChannels = parset.nrChannelsPerSubband();
      size_t nIntegrationSteps = parset.integrationSteps();
      // TODO: base weights on flags
      // Just set the weights to the total number of samples     
      for (size_t bl = 0; bl < output.itsNrBaselines; ++bl)
        for (size_t ch = 0; ch < nChannels; ++ch)
          output.setNrValidSamples(bl, ch, nIntegrationSteps);
    }


    void CorrelatorWorkQueue::doSubband(unsigned block, unsigned subband, CorrelatedData &output)
    {
      timers["GPU - total"]->start();

      {
        timers["GPU - input"]->start();

#if defined USE_B7015
        OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
        inputData.inputSamples.hostToDevice(CL_TRUE);
        counters["input - samples"]->doOperation(inputData.inputSamples.event, 0, 0, inputData.inputSamples.bytesize());

        timers["GPU - input"]->stop();
      }

      timers["GPU - compute"]->start();

      // Moved from doWork() The delay data should be available before the kernels start.
      // Queue processed ordered. This could main that the transfer is not nicely overlapped

      inputData.delaysAtBegin.hostToDevice(CL_FALSE);
      inputData.delaysAfterEnd.hostToDevice(CL_FALSE);
      inputData.phaseOffsets.hostToDevice(CL_FALSE);

      if (ps.nrChannelsPerSubband() > 1) {
        firFilterKernel.enqueue(queue, *counters["compute - FIR"]);
        fftKernel.enqueue(queue, *counters["compute - FFT"]);
      }

      delayAndBandPassKernel.enqueue(queue, *counters["compute - delay/bp"], subband);
#if defined USE_NEW_CORRELATOR
      correlateTriangleKernel.enqueue(queue, *counters["compute - cor.triangle"]);
      correlateRectangleKernel.enqueue(queue, *counters["compute - cor.rectangle"]);
#else
      correlatorKernel.enqueue(queue, *counters["compute - correlator"]);
#endif

      // ***** The GPU will be occupied for a while, do some calculations in the
      // background.

      // Propagate the flags
      computeFlags(ps, inputData, output);

      // Wait for the GPU to finish.
      timers["GPU - wait"]->start();
      queue.finish();
      timers["GPU - wait"]->stop();

      timers["GPU - compute"]->stop();

      {
        timers["GPU - output"]->start();

#if defined USE_B7015
        OMP_ScopedLock scopedLock(pipeline.deviceToHostLock[gpu / 2]);
#endif
        visibilities.deviceToHost(CL_TRUE);
        counters["output - visibilities"]->doOperation(visibilities.event, 0, visibilities.bytesize(), 0);

        timers["GPU - output"]->stop();
      }

      timers["GPU - total"]->stop();

      // Copy visibilities
      //output.visibilities = visibilities; <<-- TODO: get this working? Rather not: Make it an explicit copy so :
      // output.visibilities.copyFrom(visibilities)
      ASSERT(output.visibilities.num_elements() == visibilities.num_elements());
      memcpy(output.visibilities.origin(), visibilities.origin(), visibilities.bytesize());
      // The flags should be copied here also
    }


    void WorkQueueInputData::read(Stream *inputStream, size_t stationIdx, unsigned subband, unsigned beamIdx)
    {
      // create a header objects
      BeamletBufferToComputeNode<i16complex>::header header_object;
      size_t subbandSize = inputSamples[stationIdx].num_elements() * sizeof *inputSamples.origin();

      // fill it with the header data from the stream
      inputStream->read(&header_object, sizeof header_object);

      // validate that the data to be received is of the correct size for the target buffer
      ASSERTSTR(subband == header_object.subband,
                "Expected subband " << subband << ", got subband "
                                    << header_object.subband);
      ASSERTSTR(subbandSize == header_object.nrSamples * header_object.sampleSize,
                "Expected " << subbandSize << " bytes, got "
                            << header_object.nrSamples * header_object.sampleSize
                            << " bytes (= " << header_object.nrSamples
                            << " samples * " << header_object.sampleSize
                            << " bytes/sample)");

      // read data into the shared buffer: This can be loaded into the gpu with a single command later on
      inputStream->read(inputSamples[stationIdx].origin(), subbandSize);

      // meta data object
      SubbandMetaData metaData(header_object.nrTABs);
      // fill it with from the stream
      metaData.read(inputStream);

      // save the flags to the input_data object, to allow
      // transfer to output
      inputFlags[stationIdx] = metaData.flags;
      
      // flag the input data, contained in the meta data
      flagInputSamples(stationIdx, metaData);

      // extract delays for the stationion beam
      struct SubbandMetaData::beamInfo &beamInfo_object = metaData.stationBeam;
      // assign the delays

      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++)
      {
        delaysAtBegin[beamIdx][stationIdx][pol] = beamInfo_object.delayAtBegin;
        delaysAfterEnd[beamIdx][stationIdx][pol] = beamInfo_object.delayAfterEnd;
        phaseOffsets[stationIdx][pol] = 0.0;
      }
    }

    // flag the input samples.
    void WorkQueueInputData::flagInputSamples(unsigned station,
                                              const SubbandMetaData& metaData)
    {
      // Get the size of a sample in bytes.
      size_t sizeof_sample = sizeof *inputSamples.origin();

      // Calculate the number elements to skip when striding over the second
      // dimension of inputSamples.
      size_t stride = inputSamples[station][0].num_elements();

      // Zero the bytes in the input data for the flagged ranges.
      for(SparseSet<unsigned>::const_iterator it = metaData.flags.getRanges().begin();
          it != metaData.flags.getRanges().end(); ++it)
      {
        void *offset = inputSamples[station][it->begin].origin();
        size_t size = stride * (it->end - it->begin) * sizeof_sample;
        memset(offset, 0, size);
      }
    }

  }
}
