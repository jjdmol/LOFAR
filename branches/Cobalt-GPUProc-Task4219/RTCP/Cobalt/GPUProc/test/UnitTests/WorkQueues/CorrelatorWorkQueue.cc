#include "lofar_config.h"

#include <WorkQueues/CorrelatorWorkQueue.h>

#include <UnitTest++.h>
#include <iostream>
#include <CoInterface/Parset.h>
#include "CoInterface/CorrelatedData.h"
#include <CoInterface/SparseSet.h>
#include <CoInterface/MultiDimArray.h>

using namespace LOFAR::RTCP;

TEST(CorrelatorWorkQueue_computeFlags)
{
  // Create a parset with the needed parameters
  Parset parset;
  parset.add("Observation.channelsPerSubband","64"); 
  parset.add("OLAP.IONProc.integrationSteps", "3072");  // both are needed?
  parset.add("OLAP.CNProc.integrationSteps", "3072");   // both are needed? Is there a check in the parset that assures equality?
  
  parset.add("OLAP.storageStationNames", "[RS106HBA]"); // Number of names here sets the number of stations.

  // Input flags: an array of sparseset
  MultiDimArray<LOFAR::SparseSet<unsigned>, 1> flags(boost::extents[parset.nrStations()]);

  CorrelatedData output(parset.nrStations(), 
                     parset.nrChannelsPerSubband(), 
                     parset.integrationSteps());
  //computeFlags
  CHECK(true);
}

int main()
{

  return UnitTest::RunAllTests();
}
