#include <lofar_config.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <mpi.h>
#include <InputProc/Transpose/MPIUtil2.h>
#include <omp.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <CoInterface/Parset.h>
#include <CoInterface/OMPThread.h>

#include <GPUProc/Station/StationInput.h>
#include <GPUProc/MPI_utils.h>


using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;


main(int argc, char **argv)
{
  string testname("tMPISendReceive");
  cout << "testname" << endl;
  // ****************************************************
  // Set up the mpi environment
  // Rank in MPI set of hosts, or 0 if no MPI is used
  int rank = 0;

  // Number of MPI hosts, or 1 if no MPI is used
  int nrHosts = 1;


  const char *rankstr, *sizestr;
  // klijn@cbt009:~/build/5607/gnu_debug/RTCP/Cobalt/GPUProc/test$ mpirun.sh -np 4 ./tMPISendReceive

  // OpenMPI rank
  if ((rankstr = getenv("OMPI_COMM_WORLD_RANK")) != NULL)
    rank = boost::lexical_cast<int>(rankstr);

  // OpenMPI size
  if ((sizestr = getenv("OMPI_COMM_WORLD_SIZE")) != NULL)
    nrHosts = boost::lexical_cast<int>(sizestr);

  // MVAPICH2 rank
  if ((rankstr = getenv("MV2_COMM_WORLD_RANK")) != NULL)
    rank = boost::lexical_cast<int>(rankstr);

  // MVAPICH2 size
  if ((sizestr = getenv("MV2_COMM_WORLD_SIZE")) != NULL)
    nrHosts = boost::lexical_cast<int>(sizestr);
  // ****************************************************
  cout <<  "MPI rank " << rank << " out of " << nrHosts << " hosts" << endl;
  
  // Allow usage of nested omp calls
  omp_set_nested(true);

  // Allow OpenMP thread registration
  OMPThread::init();

  string parsetFile = "./tMPISendReceive.in_parset";
  Parset ps(parsetFile);

  SubbandDistribution subbandDistribution; // rank -> [subbands]
  for (size_t subband = 0; subband < ps.nrSubbands(); ++subband) 
  {
    int receiverRank = subband % nrHosts;
    subbandDistribution[receiverRank].push_back(subband);
  }

  bool correlatorEnabled = ps.settings.correlator.enabled;
  bool beamFormerEnabled = ps.settings.beamFormer.enabled;
  if (correlatorEnabled && beamFormerEnabled) {
    cout << "Commensal observations (correlator+beamformer) not supported yet." << endl;
    exit(1);
  }
  // Initialise and query MPI
  int provided_mpi_thread_support;

  if (MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE,
      &provided_mpi_thread_support) != MPI_SUCCESS) {
    cerr << "MPI_Init_thread failed" << endl;
    exit(1);
  }

  // Verify the rank/size settings we assumed earlier
  int real_rank;
  int real_size;

  MPI_Comm_rank(MPI_COMM_WORLD, &real_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &real_size);

  ASSERT(rank == real_rank);
  ASSERT(nrHosts == real_size);
  { // We need to delete all MPI data objects before calling mpi_finalize
    // start a code block therefore
  MPIPoll::instance().start();

  // Create a pool with MPI data objects, this object can now be provided
  // to the receiver to be filled
  // In this test it is simply emptied without doing anything with the data.
  // normally the pool is emptied by transpose input 
  Pool<struct MPIRecvData> MPI_receive_pool("rtcp::MPI_recieve_pool");
  // Who received what subband?
  const std::vector<size_t>  subbandIndices(subbandDistribution[rank]);
  bool isThisSubbandZero = std::find(subbandIndices.begin(),
    subbandIndices.end(), 0U) != subbandIndices.end();

  // The receiver object
  MPIReceiver MPI_receiver(MPI_receive_pool,  // pool to insert data into
    subbandIndices,                           // what to process
    isThisSubbandZero,
    ps.nrSamplesPerSubband(),
    ps.nrStations(),
    ps.nrBitsPerSample());

  cout << "Processing subbands " << subbandDistribution[rank] << endl;

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

        const StationNodeAllocation allocation(stationID, ps);

        if (!allocation.receivedHere()) {
          // Station is not sending from this node
          continue;
        }

        sendInputToPipeline(ps, stat, subbandDistribution);
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

  }// Assure destruction of mpi object
  MPIPoll::instance().stop();

  MPI_Finalize();


  return 0;
}
