#include <lofar_config.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <mpi.h>
#include <InputProc/Transpose/MPIUtil.h>
#include <omp.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <CoInterface/Parset.h>
#include <CoInterface/OMPThread.h>

#include <GPUProc/Station/StationInput.h>
#include <GPUProc/Station/StationNodeAllocation.h>
#include <GPUProc/MPIReceiver.h>


using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;


int main(int argc, char **argv)
{
  INIT_LOGGER("tMPIReceive");

  LOFAR::Cobalt::MPI mpi;

  // ****************************************************
  cout <<  "MPI rank " << mpi.rank() << " out of " << mpi.size() << " hosts" << endl;
  
  // Allow usage of nested omp calls
  omp_set_nested(true);

  // Allow OpenMP thread registration
  OMPThread::init();

  string parsetFile = "./tMPIReceive.in_parset";
  Parset ps(parsetFile);

  SubbandDistribution subbandDistribution; // rank -> [subbands]
  for (size_t subband = 0; subband < ps.nrSubbands(); ++subband) 
  {
    int receiverRank = subband % mpi.size();
    subbandDistribution[receiverRank].push_back(subband);
  }

  bool correlatorEnabled = ps.settings.correlator.enabled;
  bool beamFormerEnabled = ps.settings.beamFormer.enabled;
  if (correlatorEnabled && beamFormerEnabled) {
    cout << "Commensal observations (correlator+beamformer) not supported yet." << endl;
    exit(1);
  }

  // Initialise MPI
  mpi.init(argc, argv);

  // Create a pool with MPI data objects, this object can now be provided
  // to the receiver to be filled
  // In this test it is simply emptied without doing anything with the data.
  // normally the pool is emptied by transpose input 
  Pool<struct MPIRecvData> MPI_receive_pool("MPI_receive_pool");
  // Who received what subband?
  const std::vector<size_t>  subbandIndices(subbandDistribution[mpi.rank()]);
  bool isThisSubbandZero = std::find(subbandIndices.begin(),
    subbandIndices.end(), 0U) != subbandIndices.end();

  // The receiver object
  MPIReceiver MPI_receiver(MPI_receive_pool,  // pool to insert data into
    subbandIndices,                           // what to process
    isThisSubbandZero,
    ps.nrSamplesPerSubband(),
    ps.nrStations(),
    ps.nrBitsPerSample());

  cout << "Processing subbands " << subbandDistribution[mpi.rank()] << endl;

#pragma omp parallel sections num_threads(3)
  {
#pragma omp section
    {
      // Read and forward station data
      // This is the code that send the data over the MPI line
#pragma omp parallel for num_threads(ps.nrStations())
      for (size_t stat = 0; stat < ps.nrStations(); ++stat) {

        // Determine if this station should start a pipeline for 
        // station..
        const struct StationID stationID(
          StationID::parseFullFieldName(
          ps.settings.antennaFields.at(stat).name));

        const StationNodeAllocation allocation(stationID, ps, mpi.rank(), mpi.size());

        if (!allocation.receivedHere()) {
          // Station is not sending from this node
          continue;
        }

        MACIO::RTmetadata rtmd(ps.observationID(), "", "");
        sendInputToPipeline(ps, stat, subbandDistribution,
                            rtmd, "rtmd key prefix");
        cout << "First ended" << endl;
      }
    }
    // receive the data over MPI and place in pool
#pragma omp section
    {

    size_t nrBlocks = floor((ps.settings.stopTime - ps.settings.startTime) / ps.settings.blockDuration());
    cout << "N blocks: " << nrBlocks << endl;

    MPI_receiver.receiveInput(nrBlocks);
    cout << "second ended" << endl;
  }

   // empty the pool
#pragma omp section
    {
      SmartPtr<struct MPIRecvData> input;
      while ((input = MPI_receive_pool.filled.remove()) != NULL) 
      {
        cout << "Block freed"  << endl;
        MPI_receive_pool.free.append(input);
      }
      cout << "third ended" << endl;
    }

  }

  return 0;
}
