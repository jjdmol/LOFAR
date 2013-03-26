#include "lofar_config.h"

#include <WorkQueues/CorrelatorWorkQueue.h>

#include <UnitTest++.h>
#include <iostream>
#include <CoInterface/Parset.h>
#include "CoInterface/CorrelatedData.h"
#include <CoInterface/SparseSet.h>
#include <CoInterface/MultiDimArray.h>
#include <complex>

using namespace LOFAR::RTCP;

TEST(CorrelatorWorkQueueComputeFlags)
{
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","64"); 
  parset.add("OLAP.IONProc.integrationSteps", "3072");  // both are needed?
  parset.add("OLAP.CNProc.integrationSteps", "3072");   // both are needed? Is there a check in the parset that assures equality?
  
  parset.add("OLAP.storageStationNames", "[RS106HBA]"); // Number of names here sets the number of stations.

  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> inputFlags(boost::extents[parset.nrStations()]);

  CorrelatedData output(parset.nrStations(), 
                     parset.nrChannelsPerSubband(), 
                     parset.integrationSteps());

  //propageFlags
  propagateFlagsToOutput(parset, inputFlags, output);
  //computeFlags
  CHECK(true);
}

TEST(getLogOfNrChannels)
{
  CHECK_EQUAL(6u, get2LogOfNrChannels(64));
  CHECK_EQUAL(7u, get2LogOfNrChannels(128));
  CHECK_EQUAL(0u, get2LogOfNrChannels(1));

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

  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> inputFlags(boost::extents[parset.nrStations()]);

  // Insert some flag ranges
  inputFlags[0].include(62, 63);    // A. should result in channelflag (0,16) due to the filter width
  inputFlags[0].include(128, 129);  // B. Outside of the begin range result: (128 / 4) - 16 + 1 = 17 end 33
  inputFlags[0].include(255, 522);  // C. Flag all large ranges (48, 131)
  inputFlags[0].include(1000, 1050); // D. Outside of range should not be a problem but capped at max (235,257)

  inputFlags[1].include(100, 600); // E. Second station (10, 150)
  // The converted channel flags
  MultiDimArray<LOFAR::SparseSet<unsigned>, 2> flagsPerChanel(
          boost::extents[parset.nrChannelsPerSubband()][parset.nrStations()]);

  // ****** perform the translation
  convertFlagsToChannelFlags(parset, inputFlags, flagsPerChanel);
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
  calculateAndSetNumberOfFlaggedSamples(parset, flagsPerChanel, output);
  
  // Now check that the flags are correctly set in the ouput object

  CHECK_EQUAL(1013u, output.nrValidSamples(0,1)); // 11 flagged in station 1
  CHECK_EQUAL(1004u, output.nrValidSamples(1,1)); // The union is 11+9 == 20 flagged
  CHECK_EQUAL(1015u, output.nrValidSamples(2,1)); // 9 flagged in station 2
 
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
  calculateAndSetNumberOfFlaggedSamples(parset, flagsPerChanel, output);
  
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
  applyFractionOfFlaggedSamplesOnVisibilities(parset, output);

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
  applyWeightingToAllPolarizations(0,0,0.5,output);

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
