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



  string parsetFile = "./tMPISendReceive.in_parset";

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

  Parset ps(parsetFile);

  // Allow usage of nested omp calls
  omp_set_nested(true);

  // Allow OpenMP thread registration
  OMPThread::init();

  SubbandDistribution subbandDistribution; // rank -> [subbands]

  for (size_t subband = 0; subband < ps.nrSubbands(); ++subband) {
    int receiverRank = subband % nrHosts;

    subbandDistribution[receiverRank].push_back(subband);
  }

  bool correlatorEnabled = ps.settings.correlator.enabled;
  bool beamFormerEnabled = ps.settings.beamFormer.enabled;

  if (correlatorEnabled && beamFormerEnabled) {
    LOG_ERROR("Commensal observations (correlator+beamformer) not supported yet.");
    exit(1);
  }

  /*
  * Initialise MPI (we are done forking)
  */

  // Initialise and query MPI
  int provided_mpi_thread_support;

  LOG_INFO("----- Initialising MPI");
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
  {
  MPIPoll::instance().start();


  Pool<struct MPIRecvData> MPI_receive_pool("rtcp::MPI_recieve_pool");

  const std::vector<size_t>  subbandIndices(subbandDistribution[rank]);

  MPIInput MPI_input(MPI_receive_pool,
    subbandIndices,
    std::find(subbandIndices.begin(),
    subbandIndices.end(), 0U) != subbandIndices.end(),
    ps.nrSamplesPerSubband(),
    ps.nrStations(),
    ps.nrBitsPerSample());

  cout << "Processing subbands " << subbandDistribution[rank] << endl;

#pragma omp parallel sections num_threads(3)
  {
#pragma omp section
    {
      // Read and forward station data
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

#pragma omp section
    {

    size_t nrBlocks = floor((ps.settings.stopTime - ps.settings.startTime) / ps.settings.blockDuration());
    cout << "N blocks: " << nrBlocks << endl;
    

    MPI_input.receiveInput(nrBlocks);
    cout << "second ended" << endl;
  }


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

  }
  MPIPoll::instance().stop();

  MPI_Finalize();


  return 0;
}
