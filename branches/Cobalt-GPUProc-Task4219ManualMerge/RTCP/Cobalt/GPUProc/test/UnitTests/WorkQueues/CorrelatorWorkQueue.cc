#include "lofar_config.h"

#include <WorkQueues/CorrelatorWorkQueue.h>

#include <UnitTest++.h>
#include <iostream>
#include <CoInterface/Parset.h>
#include "CoInterface/CorrelatedData.h"
#include <CoInterface/SparseSet.h>
#include <CoInterface/MultiDimArray.h>
#include <complex>

using namespace LOFAR::Cobalt;

TEST(propagateFlagsToOutput)
{
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","4"); //not to large a number of subbands, else the integration steps gets big
  parset.add("OLAP.IONProc.integrationSteps", "1");  
  parset.add("OLAP.CNProc.integrationSteps", "1024");   //samples per channel 
  
  parset.add("OLAP.storageStationNames", "[RS102HBA, RS103HBA, RS104HBA, RS105HBA]"); // Number of names here sets the number of stations.
  parset.updateSettings();
  unsigned number_of_baselines = (parset.nrStations() * 1.0 * (parset.nrStations() + 1)) / 2;

  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> inputFlags(boost::extents[parset.nrStations()]);
  // Set some flags
  inputFlags[2].include(100, 101);    // A. a single sample flagged in station 3
  inputFlags[3].include(500, 501);  // B. a single sample flagged is station 4
  
  // The output data to be flagged
  CorrelatedData output(parset.nrStations(), 
                     parset.nrChannelsPerSubband(), 
                     parset.integrationSteps());

  // The content ergo the visibilities should be 1, this allows us to validate the weighting
  //assign all values 1 pol 0
  for(unsigned idx_baseline = 0; idx_baseline < number_of_baselines; ++idx_baseline)
    for(unsigned idx_channel = 0; idx_channel < parset.nrChannelsPerSubband(); ++idx_channel)    
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2)    
           output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2] = std::complex<float>(1,0);



  // *********************************************************************************************
  //propageFlags: exercise the functionality
  CorrelatorWorkQueue::flagFunctions::propagateFlagsToOutput(parset, inputFlags, output);

  // now perform weighting of the data based on the number of valid samples
  CorrelatorWorkQueue::flagFunctions::applyFractionOfFlaggedSamplesOnVisibilities(parset, output);  
  // *********************************************************************************************

  // Now validate the functionality:
  // 1. Channel zero all zero, there are more then 1 channels: fft fills channel 0 with garbage
  for(unsigned idx_baseline = 0; idx_baseline < number_of_baselines; ++idx_baseline)
    for(unsigned idx_channel = 0; idx_channel < 1; ++idx_channel)    //validate channel zero
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_EQUAL(std::complex<float>(0,0), output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2]);

  // 2. station zero and one have no flags, the baselines for these station should be default 
  float weight_of_unflagged_sample = 1e-6f/1024; // default weighting / number of samples
  for(unsigned idx_baseline = 0; idx_baseline < 3; ++idx_baseline)  //bl 0-3 are 0.0 1.0 and 1.1
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
           CHECK_CLOSE(weight_of_unflagged_sample,
                      output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta

  // 3. Now check the weights for bl 4 to 6: flagging should be a single flagged sample on the input
  // only a single point in time is flagged should result in weight_of_single_sample
  float weight_of_single_sample = 1e-6f/(1024 - 1 * NR_TAPS);  // 1 * filter width
  for(unsigned idx_baseline = 3; idx_baseline < 6; ++idx_baseline)
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_CLOSE(weight_of_single_sample,
                      output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta

    // 3. Now check the weights for bl 4 to 6: flagging should be a single flagged sample on the input from station 3
  // only a single point in time is flagged should result in weight_of_single_sample
  for(unsigned idx_baseline = 6; idx_baseline < 8; ++idx_baseline)
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_CLOSE(weight_of_single_sample,
                      output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta

  // station 2 and 3: two samples
  float weight_of_two_sample = 1e-6f/(1024 - 2 * NR_TAPS);  // 1 * filter width
  for(unsigned idx_baseline = 8; idx_baseline < 9; ++idx_baseline)
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_CLOSE(weight_of_two_sample,
                      output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta

    // station 3 and auto 3: 1 sample
  for(unsigned idx_baseline = 9; idx_baseline < 10; ++idx_baseline)
    for(unsigned idx_channel = 1; idx_channel < 4; ++idx_channel)    //validate channel ONE and higher
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2) 
          CHECK_CLOSE(weight_of_single_sample,
                      output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2].real(),
                      1e-18f);  // float compare with this delta
 
}

TEST(getLogOfNrChannels)
{
  CHECK_EQUAL(6u, CorrelatorWorkQueue::flagFunctions::get2LogOfNrChannels(64));
  CHECK_EQUAL(7u, CorrelatorWorkQueue::flagFunctions::get2LogOfNrChannels(128));
  CHECK_EQUAL(0u, CorrelatorWorkQueue::flagFunctions::get2LogOfNrChannels(1));

  //cant take 2log of zero: raise exception
  //CHECK_THROW(get2LogOfNrChannels(0), LOFAR::AssertError);
}

TEST(convertFlagsToChannelFlags)
{
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","4"); //not to large a number of subbands, else the integration steps gets big
  parset.add("OLAP.IONProc.integrationSteps", "1");  
  parset.add("OLAP.CNProc.integrationSteps", "256");   //samples per channel 
  
  parset.add("OLAP.storageStationNames", "[RS106HBA, RS105HBA]"); // Number of names here sets the number of stations.
  parset.updateSettings();
  
  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> inputFlags(boost::extents[parset.nrStations()]);

  //// Insert some flag ranges
  //std::cout << "inputFlags.size(): " << inputFlags.size() << std::endl;
  //std::cout << "parset.nrStations(): " << parset.nrStations() << std::endl;
  inputFlags[0].include(62, 63);    // A. should result in channelflag (0,16) due to the filter width
  inputFlags[0].include(128, 129);  // B. Outside of the begin range result: (128 / 4) - 16 + 1 = 17 end 33
  inputFlags[0].include(255, 522);  // C. Flag all large ranges (48, 131)
  inputFlags[0].include(1000, 1050); // D. Outside of range should not be a problem but capped at max (235,257)

  inputFlags[1].include(100, 600); // E. Second station (10, 150)
  // The converted channel flags
  MultiDimArray<LOFAR::SparseSet<unsigned>, 2> flagsPerChanel(
          boost::extents[parset.nrChannelsPerSubband()][parset.nrStations()]);

  // ****** perform the translation
  CorrelatorWorkQueue::flagFunctions::convertFlagsToChannelFlags(parset, inputFlags, flagsPerChanel);
  // ******

  //validate the corner cases
  CHECK(0 == flagsPerChanel[0][0].getRanges()[0].begin && 
        16 == flagsPerChanel[0][0].getRanges()[0].end);  //A.
  CHECK(17 == flagsPerChanel[0][0].getRanges()[1].begin &&
        33 == flagsPerChanel[0][0].getRanges()[1].end);  //B.
  CHECK(48 == flagsPerChanel[0][0].getRanges()[2].begin &&
        131 == flagsPerChanel[0][0].getRanges()[2].end);  //C.
  CHECK(235 == flagsPerChanel[0][0].getRanges()[3].begin &&
        257 == flagsPerChanel[0][0].getRanges()[3].end);  //D.

  CHECK(10 == flagsPerChanel[0][1].getRanges()[0].begin &&
        150 == flagsPerChanel[0][1].getRanges()[0].end);  //E.
}

TEST(calculateAndSetNumberOfFlaggedSamples4Channels)
{
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","4"); 
  parset.add("OLAP.IONProc.integrationSteps", "1");  // both are needed?
  parset.add("OLAP.CNProc.integrationSteps", "256");   // both are needed? Is there a check in the parset that assures equality?
  
  parset.add("OLAP.storageStationNames", "[RS106HBA, RS107HBA]"); // Number of names here sets the number of stations.
  parset.updateSettings();

  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 2> flagsPerChanel(
          boost::extents[parset.nrChannelsPerSubband()][parset.nrStations()]);

  // Output object
  CorrelatedData output(parset.nrStations(), 
                     parset.nrChannelsPerSubband(), 
                     parset.integrationSteps());

  //insert same cases
  flagsPerChanel[1][0].include(100,111);//A.
  flagsPerChanel[1][1].include(111,120);//E. second station flags
  
  //propageFlags
  CorrelatorWorkQueue::flagFunctions::calculateAndSetNumberOfFlaggedSamples(parset, flagsPerChanel, output);
  
  // Now check that the flags are correctly set in the ouput object

  CHECK_EQUAL(256u - 11u, output.nrValidSamples(0,1)); // 11 flagged in station 1
  CHECK_EQUAL(256u - 20u, output.nrValidSamples(1,1)); // The union is 11+9 == 20 flagged
  CHECK_EQUAL(256u - 9u, output.nrValidSamples(2,1)); // 9 flagged in station 2
 
  // Channel zero should always be all flagged
  CHECK_EQUAL(0u, output.nrValidSamples(0,0)); // all flagged in station 2
  CHECK_EQUAL(0u, output.nrValidSamples(1,0)); // all flagged in station 2
  CHECK_EQUAL(0u, output.nrValidSamples(2,0)); // all flagged in station 2
}

TEST(calculateAndSetNumberOfFlaggedSamples1Channels)
{
  // on channel so the zero channel should be filled with the flags!!
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","1"); 
  parset.add("OLAP.IONProc.integrationSteps", "1");  // both are needed?
  parset.add("OLAP.CNProc.integrationSteps", "256");   // both are needed? Is there a check in the parset that assures equality?
  
  parset.add("OLAP.storageStationNames", "[RS106HBA, RS107HBA]"); // Number of names here sets the number of stations.
  parset.updateSettings();
  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 2> flagsPerChanel(
          boost::extents[parset.nrChannelsPerSubband()][parset.nrStations()]);

  // Output object
  CorrelatedData output(parset.nrStations(), 
                     parset.nrChannelsPerSubband(), 
                     parset.integrationSteps());

  //insert same cases
  flagsPerChanel[0][0].include(100,111);//A.
  flagsPerChanel[0][1].include(111,120);//E. second station flags
  
  //propageFlags
  CorrelatorWorkQueue::flagFunctions::calculateAndSetNumberOfFlaggedSamples(parset, flagsPerChanel, output);
  
  // Now check that the flags are correctly set in the ouput object
  // channel is 1 so no time resolution loss!!
  CHECK_EQUAL(245u, output.nrValidSamples(0,0)); // 11 flagged in station 1
  CHECK_EQUAL(236u, output.nrValidSamples(1,0)); // The union is 11+9 == 20 flagged
  CHECK_EQUAL(247u, output.nrValidSamples(2,0)); // 9 flagged in station 2  
}

TEST(applyFractionOfFlaggedSamplesOnVisibilities)
{
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","4"); 
  parset.add("OLAP.IONProc.integrationSteps", "1");  // both are needed?
  parset.add("OLAP.CNProc.integrationSteps", "256");   // both are needed? Is there a check in the parset that assures equality?
  
  parset.add("OLAP.storageStationNames", "[RS106HBA, RS107HBA]"); // Number of names here sets the number of stations.
  parset.updateSettings();
  // Create correlated data object
  CorrelatedData output(parset.nrStations(), 
                     parset.nrChannelsPerSubband(), 
                     parset.integrationSteps());

 
  //assign all values 1 pol 0
  for(unsigned idx_baseline = 0; idx_baseline < 3; ++idx_baseline)
    for(unsigned idx_channel = 0; idx_channel < parset.nrChannelsPerSubband(); ++idx_channel)    
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2)    
           output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2] = std::complex<float>(1,0);
        
  // set some flagged samples
  unsigned n_valid_samples = 20;
  output.setNrValidSamples(0,1,n_valid_samples); //baseline 0, channel 1
  output.setNrValidSamples(1,1,256); //baseline 1, channel 1
  output.setNrValidSamples(2,1,0); //baseline 0, channel 1
  CorrelatorWorkQueue::flagFunctions::applyFractionOfFlaggedSamplesOnVisibilities(parset, output);

  // 4 channels: therefore the chanel zero should be zero
  CHECK_EQUAL(std::complex<float>(0,0), output.visibilities[0][0][0][0]);
  CHECK_EQUAL(std::complex<float>(0,0), output.visibilities[2][0][1][1]); // check origin and far corner

  // the weighted values should be multiplied with 1e-6 divided
  // by the number of samples
  CHECK_EQUAL(std::complex<float>(1e-6/n_valid_samples,0), output.visibilities[0][1][0][0]);
  CHECK_EQUAL(std::complex<float>(1e-6/n_valid_samples,0), output.visibilities[0][1][1][1]);

  // baselines 1 
  CHECK_EQUAL(std::complex<float>(1e-6/256,0), output.visibilities[1][1][0][0]);
  CHECK_EQUAL(std::complex<float>(1e-6/256,0), output.visibilities[1][1][1][1]);

  //baseline 2 no samples so should be zero
  CHECK_EQUAL(std::complex<float>(0,0), output.visibilities[2][1][0][0]);
  CHECK_EQUAL(std::complex<float>(0,0), output.visibilities[2][1][1][1]);
}

TEST(applyWeightingToAllPolarizations)
{
    // on channel so the zero channel should be filled with the flags!!
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","1"); 
  parset.add("OLAP.IONProc.integrationSteps", "1");  // both are needed?
  parset.add("OLAP.CNProc.integrationSteps", "256");   // both are needed? Is there a check in the parset that assures equality?
  
  parset.add("OLAP.storageStationNames", "[RS106HBA, RS107HBA]"); // Number of names here sets the number of stations.
  parset.updateSettings();
   // Output object
  CorrelatedData output(parset.nrStations(), 
                        parset.nrChannelsPerSubband(), 
                        parset.integrationSteps());

 
  //assign all visibilities values 1 pol 0
  for(unsigned idx_baseline = 0; idx_baseline < 3; ++idx_baseline)
    for(unsigned idx_channel = 0; idx_channel < parset.nrChannelsPerSubband(); ++idx_channel)    
      for(unsigned idx_pol1 = 0; idx_pol1 < NR_POLARIZATIONS; ++idx_pol1)    
        for(unsigned idx_pol2 = 0; idx_pol2 < NR_POLARIZATIONS; ++idx_pol2)    
           output.visibilities[idx_baseline][idx_channel][idx_pol1][idx_pol2] = std::complex<float>(1,0);
        
  //  multiply all polarization in sb 0 channel 0 with 0,5
  CorrelatorWorkQueue::flagFunctions::applyWeightingToAllPolarizations(0,0,0.5,output);

  //sb 0 should be (0.5, 0)
  CHECK_EQUAL(std::complex<float>(0.5,0),  
              output.visibilities[0][0][0][0]);
  //still be 1.0
  CHECK_EQUAL(std::complex<float>(1,0),  
              output.visibilities[1][0][0][0]);
   
}

int main()
{
  return UnitTest::RunAllTests();
}

