//# tCorrelatorSubbandProc.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <GPUProc/cuda/SubbandProcs/CorrelatorStep.h>

#include <UnitTest++.h>
#include <iostream>
#include <CoInterface/Parset.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/SparseSet.h>
#include <CoInterface/MultiDimArray.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <complex>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

// Global variable holding the GPU context to bind objects to
gpu::Context *context = 0;

TEST(propagateFlags)
{
  // Create a parset with the needed parameters
  Parset parset;

  parset.add("Cobalt.Correlator.nrChannelsPerSubband","4");
  parset.add("Cobalt.Correlator.nrBlocksPerIntegration", "1");
  parset.add("Cobalt.blockSize", "4096");
  
  parset.add("Observation.VirtualInstrument.stationList", "[RS102, RS103, RS104, RS105]"); // Number of names here sets the number of stations.
  parset.add("Observation.antennaSet", "HBA_ZERO");
  parset.add("Observation.rspBoardList", "[0]");
  parset.add("Observation.rspSlotList", "[0]");
  parset.add("Observation.nrBeams", "1");
  parset.add("Observation.Beam[0].subbandList", "[0]");

  parset.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  parset.add("Observation.DataProducts.Output_Correlated.filenames","[L24523_B000_S0_P000_bf.ms]");
  parset.add("Observation.DataProducts.Output_Correlated.locations","[lse011:/data3/L2011_24523/]");

  parset.updateSettings();
  unsigned number_of_baselines = parset.nrBaselines();

  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> inputFlags(boost::extents[parset.settings.correlator.stations.size()]);
  // Set some flags
  inputFlags[2].include(100, 101);    // A. a single sample flagged in station 3
  inputFlags[3].include(500, 501);  // B. a single sample flagged is station 4
  
  // The output data to be flagged
  SubbandProcOutputData::CorrelatedData output(1,
                     parset.settings.correlator.stations.size(),
                     parset.settings.correlator.nrChannels, 
                     parset.settings.correlator.nrSamplesPerIntegration(),
                     *context);
  MultiDimArray<fcomplex, 4> &visibilities = output.subblocks[0]->visibilities;

  // The content ergo the visibilities should be 1, this allows us to validate the weighting
  //assign all values 1 pol 0
  for(unsigned idx_baseline = 0; idx_baseline < number_of_baselines; ++idx_baseline)
    for(unsigned idx_channel = 0; idx_channel < parset.settings.correlator.nrChannels; ++idx_channel)    
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2)    
           visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2] = std::complex<float>(1,0);


  // *********************************************************************************************
  //propageFlags: exercise the functionality
  CorrelatorStep::Flagger::propagateFlags(parset, inputFlags, output);

  // now perform weighting of the data based on the number of valid samples
  CorrelatorStep::Flagger::applyNrValidSamples(parset, *output.subblocks[0]);  
  // *********************************************************************************************

  // Now validate the functionality:
  // 1. Channel zero all zero, there are more then 1 channels: fft fills channel 0 with garbage
  for(unsigned idx_baseline = 0; idx_baseline < number_of_baselines; ++idx_baseline)
    for(unsigned idx_channel = 0; idx_channel < 1; ++idx_channel)    //validate channel zero
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_EQUAL(std::complex<float>(0,0), visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2]);

  // 2. station zero and one have no flags, the baselines for these station should be default 
  float weight_of_unflagged_sample = 1.0f/1024; // default weighting / number of samples
  for(unsigned idx_baseline = 0; idx_baseline < 3; ++idx_baseline)  //bl 0-3 are 0.0 1.0 and 1.1
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
           CHECK_CLOSE(weight_of_unflagged_sample,
                      visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta

  // 3. Now check the weights for bl 4 to 6: flagging should be a single flagged sample on the input
  // only a single point in time is flagged should result in weight_of_single_sample
  float weight_of_single_sample = 1.0f/(1024 - 1 * NR_TAPS);  // 1 * filter width
  for(unsigned idx_baseline = 3; idx_baseline < 6; ++idx_baseline)
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_CLOSE(weight_of_single_sample,
                      visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta

    // 3. Now check the weights for bl 4 to 6: flagging should be a single flagged sample on the input from station 3
  // only a single point in time is flagged should result in weight_of_single_sample
  for(unsigned idx_baseline = 6; idx_baseline < 8; ++idx_baseline)
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_CLOSE(weight_of_single_sample,
                      visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta

  // station 2 and 3: two samples
  float weight_of_two_sample = 1.0f/(1024 - 2 * NR_TAPS);  // 1 * filter width
  for(unsigned idx_baseline = 8; idx_baseline < 9; ++idx_baseline)
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_CLOSE(weight_of_two_sample,
                      visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta

    // station 3 and auto 3: 1 sample
  for(unsigned idx_baseline = 9; idx_baseline < 10; ++idx_baseline)
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_CLOSE(weight_of_single_sample,
                      visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta
}


TEST(calcNrValidSamples4Channels)
{
  // Create a parset with the needed parameters
  Parset parset;

  parset.add("Cobalt.Correlator.nrChannelsPerSubband","4");
  parset.add("Cobalt.Correlator.nrBlocksPerIntegration", "1");
  parset.add("Cobalt.blockSize", "1024");
  
  parset.add("Observation.VirtualInstrument.stationList", "[RS106, RS107]"); // Number of names here sets the number of stations.
  parset.add("Observation.antennaSet", "HBA_ZERO");
  parset.add("Observation.rspBoardList", "[0]");
  parset.add("Observation.rspSlotList", "[0]");
  parset.add("Observation.nrBeams", "1");
  parset.add("Observation.Beam[0].subbandList", "[0]");

  parset.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  parset.add("Observation.DataProducts.Output_Correlated.filenames","[L24523_B000_S0_P000_bf.ms]");
  parset.add("Observation.DataProducts.Output_Correlated.locations","[lse011:/data3/L2011_24523/]");

  parset.updateSettings();

  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> flagsPerChannel(
          boost::extents[parset.settings.correlator.stations.size()]);

  // Output object
  SubbandProcOutputData::CorrelatedData output(1,
                     parset.settings.correlator.stations.size(),
                     parset.settings.correlator.nrChannels, 
                     parset.settings.correlator.nrSamplesPerIntegration(),
                     *context);

  //insert same cases
  flagsPerChannel[0].include(100,111);//A.
  flagsPerChannel[1].include(111,120);//E. second station flags
  
  //propageFlags
  CorrelatorStep::Flagger::calcNrValidSamples(parset, flagsPerChannel, output);
  
  // Now check that the flags are correctly set in the ouput object

  CHECK_EQUAL(256u - 11u, output.subblocks[0]->getNrValidSamples(0,1)); // 11 flagged in station 1
  CHECK_EQUAL(256u - 20u, output.subblocks[0]->getNrValidSamples(1,1)); // The union is 11+9 == 20 flagged
  CHECK_EQUAL(256u - 9u, output.subblocks[0]->getNrValidSamples(2,1)); // 9 flagged in station 2
 
  // Channel zero should always be all flagged
  CHECK_EQUAL(0u, output.subblocks[0]->getNrValidSamples(0,0)); // all flagged in station 2
  CHECK_EQUAL(0u, output.subblocks[0]->getNrValidSamples(1,0)); // all flagged in station 2
  CHECK_EQUAL(0u, output.subblocks[0]->getNrValidSamples(2,0)); // all flagged in station 2
}

TEST(calcNrValidSamples1Channels)
{
  // on channel so the zero channel should be filled with the flags!!
  // Create a parset with the needed parameters
  Parset parset;

  parset.add("Cobalt.Correlator.nrChannelsPerSubband","1");
  parset.add("Cobalt.Correlator.nrBlocksPerIntegration", "1");
  parset.add("Cobalt.blockSize", "256");
  
  parset.add("Observation.VirtualInstrument.stationList", "[RS106, RS107]"); // Number of names here sets the number of stations.
  parset.add("Observation.antennaSet", "HBA_ZERO");
  parset.add("Observation.rspBoardList", "[0]");
  parset.add("Observation.rspSlotList", "[0]");
  parset.add("Observation.nrBeams", "1");
  parset.add("Observation.Beam[0].subbandList", "[0]");

  parset.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  parset.add("Observation.DataProducts.Output_Correlated.filenames","[L24523_B000_S0_P000_bf.ms]");
  parset.add("Observation.DataProducts.Output_Correlated.locations","[lse011:/data3/L2011_24523/]");

  parset.updateSettings();
  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> flagsPerChannel(
          boost::extents[parset.settings.correlator.stations.size()]);

  // Output object
  SubbandProcOutputData::CorrelatedData output(1,
                     parset.settings.correlator.stations.size(),
                     parset.settings.correlator.nrChannels, 
                     parset.settings.correlator.nrSamplesPerIntegration(),
                     *context);

  //insert same cases
  flagsPerChannel[0].include(100,111);//A.
  flagsPerChannel[1].include(111,120);//E. second station flags
  
  //propageFlags
  CorrelatorStep::Flagger::calcNrValidSamples(parset, flagsPerChannel, output);
  
  // Now check that the flags are correctly set in the ouput object
  // channel is 1 so no time resolution loss!!
  CHECK_EQUAL(245u, output.subblocks[0]->getNrValidSamples(0,0)); // 11 flagged in station 1
  CHECK_EQUAL(236u, output.subblocks[0]->getNrValidSamples(1,0)); // The union is 11+9 == 20 flagged
  CHECK_EQUAL(247u, output.subblocks[0]->getNrValidSamples(2,0)); // 9 flagged in station 2  
}

TEST(applyNrValidSamples)
{
  // Create a parset with the needed parameters
  Parset parset;

  parset.add("Cobalt.Correlator.nrChannelsPerSubband","4");
  parset.add("Cobalt.Correlator.nrBlocksPerIntegration", "1");
  parset.add("Cobalt.blockSize", "1024");
  
  parset.add("Observation.VirtualInstrument.stationList", "[RS106, RS107]"); // Number of names here sets the number of stations.
  parset.add("Observation.antennaSet", "HBA_ZERO");
  parset.add("Observation.rspBoardList", "[0]");
  parset.add("Observation.rspSlotList", "[0]");
  parset.add("Observation.nrBeams", "1");
  parset.add("Observation.Beam[0].subbandList", "[0]");

  parset.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  parset.add("Observation.DataProducts.Output_Correlated.filenames","[L24523_B000_S0_P000_bf.ms]");
  parset.add("Observation.DataProducts.Output_Correlated.locations","[lse011:/data3/L2011_24523/]");
  parset.updateSettings();
  // Create correlated data object
  SubbandProcOutputData::CorrelatedData output(1,
                     parset.settings.correlator.stations.size(), 
                     parset.settings.correlator.nrChannels, 
                     parset.settings.correlator.nrSamplesPerIntegration(),
                     *context);
  MultiDimArray<fcomplex, 4> &visibilities = output.subblocks[0]->visibilities;
 
  //assign all values 1 pol 0
  for(unsigned idx_baseline = 0; idx_baseline < 3; ++idx_baseline)
    for(unsigned idx_channel = 0; idx_channel < parset.settings.correlator.nrChannels; ++idx_channel)    
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2)    
           visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2] = std::complex<float>(1,0);
        
  // set some flagged samples
  unsigned n_valid_samples = 20;
  output.subblocks[0]->setNrValidSamples(0,1,n_valid_samples); //baseline 0, channel 1
  output.subblocks[0]->setNrValidSamples(1,1,256); //baseline 1, channel 1
  output.subblocks[0]->setNrValidSamples(2,1,0); //baseline 0, channel 1
  CorrelatorStep::Flagger::applyNrValidSamples(parset, *output.subblocks[0]);

  // 4 channels: therefore the chanel zero should be zero
  CHECK_EQUAL(std::complex<float>(0,0), visibilities[0][0][0][0]);
  CHECK_EQUAL(std::complex<float>(0,0), visibilities[2][0][1][1]); // check origin and far corner

  // the weighted values should be divided by the number of samples
  CHECK_EQUAL(std::complex<float>(1.0/n_valid_samples,0), visibilities[0][1][0][0]);
  CHECK_EQUAL(std::complex<float>(1.0/n_valid_samples,0), visibilities[0][1][1][1]);

  // baselines 1 
  CHECK_EQUAL(std::complex<float>(1.0/256,0), visibilities[1][1][0][0]);
  CHECK_EQUAL(std::complex<float>(1.0/256,0), visibilities[1][1][1][1]);

  //baseline 2 no samples so should be zero
  CHECK_EQUAL(std::complex<float>(0,0), visibilities[2][1][0][0]);
  CHECK_EQUAL(std::complex<float>(0,0), visibilities[2][1][1][1]);
}

TEST(applyNrValidSamples2)
{
    // on channel so the zero channel should be filled with the flags!!
  // Create a parset with the needed parameters
  Parset parset;


  parset.add("Cobalt.Correlator.nrChannelsPerSubband","4");
  parset.add("Cobalt.Correlator.nrBlocksPerIntegration", "1");
  parset.add("Cobalt.blockSize", "1024");
  
  parset.add("Observation.VirtualInstrument.stationList", "[RS106, RS107]"); // Number of names here sets the number of stations.
  parset.add("Observation.antennaSet", "HBA_ZERO");
  parset.add("Observation.rspBoardList", "[0]");
  parset.add("Observation.rspSlotList", "[0]");
  parset.add("Observation.nrBeams", "1");
  parset.add("Observation.Beam[0].subbandList", "[0]");

  parset.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  parset.add("Observation.DataProducts.Output_Correlated.filenames","[L24523_B000_S0_P000_bf.ms]");
  parset.add("Observation.DataProducts.Output_Correlated.locations","[lse011:/data3/L2011_24523/]");

  parset.updateSettings();

  // Output object
  SubbandProcOutputData::CorrelatedData output(1,
                        parset.settings.correlator.stations.size(), 
                        parset.settings.correlator.nrChannels, 
                        parset.settings.correlator.nrSamplesPerIntegration(),
                        *context);
  MultiDimArray<fcomplex, 4> &visibilities = output.subblocks[0]->visibilities;
 
  //assign all visibilities values 1 pol 0
  for(unsigned idx_baseline = 0; idx_baseline < 3; ++idx_baseline)
    for(unsigned idx_channel = 0; idx_channel < parset.settings.correlator.nrChannels; ++idx_channel)    
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2)    
           visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2] = std::complex<float>(1,0);
        
  //  multiply all polarization in sb 0 channel 0 with 0,5
  CorrelatorStep::Flagger::applyWeight(0, parset.settings.correlator.nrChannels, 0.5, *output.subblocks[0]);

  //ch 0 should be (0, 0)
  CHECK_EQUAL(std::complex<float>(0,0),   visibilities[0][0][0][0]);
  CHECK_EQUAL(std::complex<float>(0,0),   visibilities[0][0][0][1]);
  CHECK_EQUAL(std::complex<float>(0,0),   visibilities[0][0][1][0]);
  CHECK_EQUAL(std::complex<float>(0,0),   visibilities[0][0][1][1]);
  //ch 1 should be (0.5, 0)
  CHECK_EQUAL(std::complex<float>(0.5,0), visibilities[0][1][0][0]);
  CHECK_EQUAL(std::complex<float>(0.5,0), visibilities[0][1][0][1]);
  CHECK_EQUAL(std::complex<float>(0.5,0), visibilities[0][1][1][0]);
  CHECK_EQUAL(std::complex<float>(0.5,0), visibilities[0][1][1][1]);
  //baseline 1 should be untouched
  CHECK_EQUAL(std::complex<float>(1,0),   visibilities[1][0][0][0]);
  CHECK_EQUAL(std::complex<float>(1,0),   visibilities[1][1][0][0]);
   
}

int main()
{
  INIT_LOGGER("tCorrelatorSubbandProc");

  try {
    gpu::Platform pf;
    LOG_INFO_STR("Detected " << pf.size() << " CUDA devices");
  } catch (gpu::CUDAException& e) {
    LOG_FATAL_STR("Caught exception: " << e.what());
    return 3;
  }

  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  context = &ctx;

  return UnitTest::RunAllTests() > 0;
}

