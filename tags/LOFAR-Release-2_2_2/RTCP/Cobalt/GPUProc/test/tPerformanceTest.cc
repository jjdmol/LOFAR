#include <lofar_config.h>

#include <mpi.h>
#include <Common/Timer.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/Kernels/CorrelatorKernel.h>
#include <InputProc/Transpose/MPIUtil.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/MultiDimArray.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

const size_t BUFSIZE = 128 * 1024 * 1024;

int main(int argc, char **argv) {
  INIT_LOGGER("tPerformanceTest");
  
  LOG_INFO_STR("Initialising GPU");

  gpu::Platform pf;
  vector<gpu::Device> devices = pf.devices();

  if (devices.empty()) {
    LOG_WARN_STR("No GPUs detected -- skipping test.");
    return 3;
  }

  gpu::Context context(devices[0]);
  gpu::Stream stream(context);

  LOG_INFO_STR("Creating kernel");

  Parset ps;
  ps.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  ps.add("Observation.DataProducts.Output_Correlated.filenames", "[SB0.MS]");
  ps.add("Observation.DataProducts.Output_Correlated.locations", "[:.]");
  ps.add("Observation.VirtualInstrument.stationList", "[CS001..CS064]");
  ps.add("Observation.Beam[0].subbandList", "[0]");
  ps.add("Observation.rspBoardList"       , "[0]");
  ps.add("Observation.rspSlotList"        , "[0]");
  ps.add("Cobalt.Correlator.nrChannelsPerSubband", "256");
  ps.updateSettings();

  CorrelatorKernel::Parameters params(ps);
  KernelFactory<CorrelatorKernel> factory(params);

  gpu::DeviceMemory devInput(context, factory.bufferSize(CorrelatorKernel::INPUT_DATA));
  gpu::DeviceMemory devOutput(context, factory.bufferSize(CorrelatorKernel::OUTPUT_DATA));

  Kernel::Buffers buffers(devInput, devOutput);

  SmartPtr<CorrelatorKernel> kernel(factory.create(stream, buffers));

  // Initialise and query MPI
  int provided_mpi_thread_support;

  LOG_INFO("Initialising MPI");
  if (MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_mpi_thread_support) != MPI_SUCCESS) {
    cerr << "MPI_Init_thread failed" << endl;
    exit(1);
  }

  LOG_INFO_STR("Running kernel");

  {
    MultiDimArray<char,1> a(boost::extents[BUFSIZE], 1, mpiAllocator);
    MultiDimArray<char,1> b(boost::extents[BUFSIZE], 1, mpiAllocator);

    NSTimer copyTimer("memcpy", true, true);
    NSTimer zeroTimer("memset", true, true);
    NSTimer syncTimer("gpu sync", true, true);

    for (size_t i = 0; i < 100; i++) {
      const BlockID blockID;

      kernel->enqueue(blockID);
      kernel->enqueue(blockID);

      copyTimer.start();
      memcpy(&b[0], &a[0], a.size());
      copyTimer.stop();

      zeroTimer.start();
      memset(&a[0], 0, b.size());
      zeroTimer.stop();

      syncTimer.start();
      stream.synchronize();
      syncTimer.stop();
    }
  }

  LOG_INFO_STR("Done");

  MPI_Finalize();
}
