//# CorrelatorWorkQueue.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include "CorrelatorWorkQueue.h"

#include <cstring>
#include <algorithm>

#include <Common/LofarLogger.h>

#include <GPUProc/OpenMP_Support.h>
#include <GPUProc/BandPass.h>
#include <GPUProc/Pipelines/CorrelatorPipelinePrograms.h>
#include <GPUProc/Input/BeamletBufferToComputeNode.h>

namespace LOFAR
{
  namespace Cobalt
  {
    /* The data travels as follows:
     *
     * [input]  -> devInput.inputSamples
     *             -> firFilterKernel
     *          -> devFilteredData
     *             -> fftKernel
     *          -> devFilteredData
     *             -> delayAndBandPassKernel
     *          -> devInput.inputSamples
     *             -> correlatorKernel
     *          -> devFilteredData = visibilities
     * [output] <-
     */
    CorrelatorWorkQueue::CorrelatorWorkQueue(const Parset       &parset,
      cl::Context &context, 
      cl::Device  &device,
      unsigned gpuNumber,
                                             CorrelatorPipelinePrograms & programs,
                                             FilterBank &filterBank
                                             )
      :
    WorkQueue( context, device, gpuNumber, parset),     
      devInput(ps.nrBeams(),
                ps.nrStations(),
                NR_POLARIZATIONS,
                (ps.nrSamplesPerChannel() + NR_TAPS - 1) * ps.nrChannelsPerSubband(),
                ps.nrBytesPerComplexSample(),
                queue,

                // reserve enough space in inputSamples for the output of
                // the delayAndBandPassKernel.
                ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>)),
      devFilteredData(queue,
                      CL_MEM_READ_WRITE,

                      // reserve enough space for the output of the
                      // firFilterKernel,
                      std::max(ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() * sizeof(std::complex<float>),
                      // and the correlatorKernel.
                               ps.nrBaselines() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * NR_POLARIZATIONS * sizeof(std::complex<float>))),
      visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], CL_MEM_READ_ONLY, devFilteredData),
      devFIRweights(queue,
                    CL_MEM_READ_ONLY,
                    ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float)),
      firFilterKernel(ps,
                      queue,
                      programs.firFilterProgram,
                      devFilteredData,
                      devInput.inputSamples,
                      devFIRweights),
      fftKernel(ps,
                context,
                devFilteredData),
      bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()],
                                queue,
                                CL_MEM_WRITE_ONLY,
                                CL_MEM_READ_ONLY),
      delayAndBandPassKernel(ps,
                             programs.delayAndBandPassProgram,
                             devInput.inputSamples,
                             devFilteredData,
                             devInput.delaysAtBegin,
                             devInput.delaysAfterEnd,
                             devInput.phaseOffsets,
                             bandPassCorrectionWeights),
#if defined USE_NEW_CORRELATOR
      correlateTriangleKernel(ps,
                              queue,
                              programs.correlatorProgram,
                              visibilities,
                              devInput.inputSamples),
      correlateRectangleKernel(ps,
                              queue,
                              programs.correlatorProgram, 
                              visibilities, 
                              devInput.inputSamples)
#else
      correlatorKernel(ps,
                       queue, 
                       programs.correlatorProgram, 
                       visibilities, 
                       devInput.inputSamples)
#endif
    {
      // put enough objects in the inputPool to operate
      for(size_t i = 0; i < 2; ++i) {
        inputPool.free.append(new WorkQueueInputData(
                ps.nrBeams(),
                ps.nrStations(),
                NR_POLARIZATIONS,
                (ps.nrSamplesPerChannel() + NR_TAPS - 1) * ps.nrChannelsPerSubband(),
                ps.nrBytesPerComplexSample(),
                devInput));
      }

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

    // Get the log2 of the supplied number
    unsigned CorrelatorWorkQueue::flagFunctions::get2LogOfNrChannels(unsigned nrChannels)
    {
      // Assure that the nrChannels is more then zero: never ending loop 
      ASSERT(powerOfTwo(nrChannels));

      unsigned logNrChannels;
      for (logNrChannels = 0; 1U << logNrChannels != nrChannels;
        logNrChannels ++)
      {;} // do nothing, the creation of the log is a side effect of the for loop

      //Alternative solution snipped:
      //int targetlevel = 0;
      //while (index >>= 1) ++targetlevel; 
      return logNrChannels;
    }

    void CorrelatorWorkQueue::flagFunctions::propagateFlagsToOutput(
      Parset const &parset,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      CorrelatedData &output)
    {   
      unsigned numberOfChannels = parset.nrChannelsPerSubband();

      // Object for storing transformed flags
      MultiDimArray<SparseSet<unsigned>, 2> flagsPerChanel(
        boost::extents[numberOfChannels][parset.nrStations()]);

      // First transform the flags to channel flags: taking in account 
      // reduced resolution in time and the size of the filter
      convertFlagsToChannelFlags(parset, inputFlags, flagsPerChanel);

      // Calculate the number of flafs per baseline and assign to
      // output object.
      calculateAndSetNumberOfFlaggedSamples(parset, flagsPerChanel,
        output);
    }

    void CorrelatorWorkQueue::flagFunctions::convertFlagsToChannelFlags(Parset const &parset,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      MultiDimArray<SparseSet<unsigned>, 2>& flagsPerChanel)
    {
      unsigned numberOfChannels = parset.nrChannelsPerSubband();
      unsigned log2NrChannels = get2LogOfNrChannels(numberOfChannels);
      //Convert the flags per sample to flags per channel
      for (unsigned station = 0; station < parset.nrStations(); station ++) 
      {
        // get the flag ranges
        const SparseSet<unsigned>::Ranges &ranges = inputFlags[station].getRanges();
        for (SparseSet<unsigned>::const_iterator it = ranges.begin();
          it != ranges.end(); it ++) 
        {
          unsigned begin_idx;
          unsigned end_idx;
          if (numberOfChannels == 1)  // if number of channels == 1
          { //do nothing, just take the ranges as supplied
            begin_idx = it->begin; 
            end_idx = std::min(parset.nrSamplesPerChannel(), it->end );
          }
          else
          {
            //Never flag before the start of the time range               
            // use bitshift to divide to the number of channels. 
            //NR_TAPS is the width of the filter: a flagged sample in this 
            //time range corrupts all the data. All flags in this range
            // result in a begin == 0
            begin_idx = std::max(0, 
              (signed) (it->begin >> log2NrChannels) - NR_TAPS + 1);
            // bitshift divide
            // TODO: is the min still needed?
            end_idx = std::min(parset.nrSamplesPerChannel() + 1, 
              ((it->end - 1) >> log2NrChannels) + 1);
          }

          // Now copy the transformed ranges to the channelflags
          for (unsigned ch = 0; ch < numberOfChannels; ch++) 
          {
            flagsPerChanel[ch][station].include(begin_idx, end_idx);
          }
        }
      }
    }

    void CorrelatorWorkQueue::flagFunctions::calculateAndSetNumberOfFlaggedSamples(
      Parset const &parset,
      MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChanel,
      CorrelatedData &output)
    {
      // loop the stations
      for (unsigned stat2 = 0; stat2 < parset.nrStations(); stat2 ++) 
      {
        for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) 
        {
          //TODO:  Calculate the station, should be moved to helper function
          unsigned bl  =  stat2 * (stat2 + 1) / 2 + stat1 ; //baseline(stat1, stat2); This function should be moved to a helper class

          unsigned nrSamplesPerIntegration = parset.nrSamplesPerChannel();
          // If there is a single channel then the index 0 contains real data
          if (parset.nrChannelsPerSubband() == 1) 
          {                                            
            //The number of invalid (flagged) samples is the union of the flagged samples in the two stations
            unsigned nrValidSamples = nrSamplesPerIntegration -
              (flagsPerChanel[0][stat1] | flagsPerChanel[0][stat2]).count();

            // Moet worden toegekend op de correlated dataobject
            output.setNrValidSamples(bl, 0, nrValidSamples);      
          } 
          else 
          {
            // channel 0 does not contain valid data
            output.setNrValidSamples(bl, 0, 0);  //channel zero, has zero valid samples

            for(unsigned ch = 1; ch < parset.nrChannelsPerSubband(); ch ++) 
            {
              // valid samples is total number of samples minus the union of the
              // Two stations.
              unsigned nrValidSamples = nrSamplesPerIntegration -
                (flagsPerChanel[ch][stat1] | flagsPerChanel[ch][stat2]).count();

              output.setNrValidSamples(bl,ch,nrValidSamples);
            }
          }
        }
      }
    }

    void CorrelatorWorkQueue::flagFunctions::applyWeightingToAllPolarizations(unsigned baseline, 
      unsigned channel, float weight, CorrelatedData &output)
    { // TODO: inline???
      for(unsigned idx_polarization_1 = 0; idx_polarization_1 < NR_POLARIZATIONS; ++idx_polarization_1)
        for(unsigned idx_polarization_2 = 0; idx_polarization_2 < NR_POLARIZATIONS; ++idx_polarization_2)
          output.visibilities[baseline][channel][idx_polarization_1][idx_polarization_2] *= weight;
    }

    void CorrelatorWorkQueue::flagFunctions::applyFractionOfFlaggedSamplesOnVisibilities(Parset const &parset,
      CorrelatedData &output)
    {
      unsigned nrSamplesPerIntegration = parset.nrSamplesPerSubband();
      for (unsigned stat2 = 0; stat2 < parset.nrStations(); stat2 ++) 
      {
        for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) 
        {
          unsigned bl  =  stat2 * (stat2 + 1) / 2 + stat1 ; //baseline(stat1, stat2); This function should be moved to a helper class
          unsigned start_channel_idx = 0;
          // If there are more then 1 channels set chanel zero 
          if (parset.nrChannelsPerSubband() > 1) 
          {
            start_channel_idx = 1; // start calculating the weights at 1
            applyWeightingToAllPolarizations(bl,0,0,output);  
          }
          // calculate the weights for the channels
          for(unsigned ch = start_channel_idx; ch < parset.nrChannelsPerSubband(); ch ++) 
          {
            unsigned nrValidSamples = output.nrValidSamples(bl, ch);
            // If all samples flagged weights is zero
            float weight = nrValidSamples ? 1e-6f/nrValidSamples : 0;  

            applyWeightingToAllPolarizations(bl,ch,weight,output);
          }
        }
      }
    }

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

    void CorrelatorWorkQueue::doSubband(unsigned subband, CorrelatedData &output)
    {
      timers["GPU - total"]->start();

      // Get new host input data
      SmartPtr<WorkQueueInputData> inputData(inputPool.filled.remove());

      {
        timers["GPU - input"]->start();

#if defined USE_B7015
        OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
        inputData->inputSamples.hostToDevice(CL_TRUE);
        counters["input - samples"]->doOperation(inputData->inputSamples.deviceBuffer.event, 0, 0, inputData->inputSamples.bytesize());

        timers["GPU - input"]->stop();
      }

      timers["GPU - compute"]->start();

      // Moved from doWork() The delay data should be available before the kernels start.
      // Queue processed ordered. This could main that the transfer is not nicely overlapped

      inputData->delaysAtBegin.hostToDevice(CL_FALSE);
      inputData->delaysAfterEnd.hostToDevice(CL_FALSE);
      inputData->phaseOffsets.hostToDevice(CL_FALSE);

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

      queue.flush();

      // ***** The GPU will be occupied for a while, do some calculations in the
      // background.

      // Propagate the flags.
      flagFunctions::propagateFlagsToOutput(ps, inputData->inputFlags, output);

      // Wait for the GPU to finish.
      timers["GPU - wait"]->start();
      queue.finish();
      timers["GPU - wait"]->stop();

      timers["GPU - compute"]->stop();

      // Finished with host input data
      inputPool.free.append(inputData);

      {
        timers["GPU - output"]->start();

#if defined USE_B7015
        OMP_ScopedLock scopedLock(pipeline.deviceToHostLock[gpu / 2]);
#endif
        visibilities.deviceToHost(CL_TRUE);
        // now perform weighting of the data based on the number of valid samples

        counters["output - visibilities"]->doOperation(visibilities.deviceBuffer.event, 0, visibilities.bytesize(), 0);

        timers["GPU - output"]->stop();
      }

      timers["GPU - total"]->stop();

      // The flags are alrady copied to the correct location
      // now the flagged amount should be applied to the visibilities
      flagFunctions::applyFractionOfFlaggedSamplesOnVisibilities(ps, output);  
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

