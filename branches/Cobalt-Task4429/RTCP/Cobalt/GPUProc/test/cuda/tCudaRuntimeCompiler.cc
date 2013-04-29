
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Common/SystemCallException.h>
#include <Common/SystemUtil.h>


using namespace std;
int main()
{

  char *kernelPath = "/home/wklijn/build/4429/gnu_debug/installed/share/gpu/kernels/TestKernel.cu";

  // Run a kernel with two different defines, should result in two results
  std::cout << kernelPath << std:: endl;

  string srcFilename(kernelPath);
#if 0
  string cudaCompiler(CUDA_TOOLKIT_ROOT_DIR);
  if (!cudaCompiler.empty()) 
  {
    cudaCompiler += "/bin/nvcc";
  }
  if (::access(cudaCompiler.c_str(), X_OK) == -1)
#endif
  const string cudaCompiler = "nvcc"; // try through PATH  TODO: allow cmd-line arg override?

  bool debug = false; // TODO: wire up to program arg or 

  stringstream cmd(cudaCompiler);
  cmd << " --ptx";                         // Request intermediate format (ptx) as output. We may want to view or stir it.
  cmd << " -I" << dirname(__FILE__);       // TODO: move kernel sources to their own dir
  if (debug) 
  {
    cmd << " -G --source-in-ptx";
  }
  cmd << " -m32";                          // -m64 (default) takes extra regs for a ptr, but allows >2 GB buffers, which we don't need
  // use opt level 99
  // -Ox is only for host code. Opt backend compilation below.
  cmd << " --gpu-architecture compute_30"; // TODO: incorrectly assumes one, single virt arch; check out targets arg
  //cmd << " --maxrregcount 63;" // probably only effective for backend compilation below
  cmd << " --use_fast_math"; // we believe we can get away with this for LOFAR online DSP

  // From here arguments that should be in 
  cmd << " -DNVIDIA_CUDA"; // left-over from OpenCL for Correlator.cl/.cu
  //cmd << " -DUSE_FLOAT4_IN_CORRELATOR"; // on "GeForce GTX 680" // TODO: move this and many -D opts below into kernel-specific build arg routines
  cmd << " -DNR_BITS_PER_SAMPLE=" << ps.nrBitsPerSample();
  cmd << " -DSUBBAND_BANDWIDTH=" << std::setprecision(7) << ps.subbandBandwidth() << 'f';
  cmd << " -DNR_SUBBANDS=" << ps.nrSubbands();
  cmd << " -DNR_CHANNELS=" << ps.nrChannelsPerSubband();
  cmd << " -DNR_STATIONS=" << ps.nrStations();
  cmd << " -DNR_SAMPLES_PER_CHANNEL=" << ps.nrSamplesPerChannel();
  cmd << " -DNR_SAMPLES_PER_SUBBAND=" << ps.nrSamplesPerSubband();
  cmd << " -DNR_BEAMS=" << ps.nrBeams();
  cmd << " -DNR_TABS=" << ps.nrTABs(0);
  cmd << " -DNR_COHERENT_STOKES=" << ps.nrCoherentStokes();
  cmd << " -DNR_INCOHERENT_STOKES=" << ps.nrIncoherentStokes();
  cmd << " -DCOHERENT_STOKES_TIME_INTEGRATION_FACTOR=" << ps.coherentStokesTimeIntegrationFactor();
  cmd << " -DINCOHERENT_STOKES_TIME_INTEGRATION_FACTOR=" << ps.incoherentStokesTimeIntegrationFactor();
  cmd << " -DNR_POLARIZATIONS=" << NR_POLARIZATIONS;
  cmd << " -DNR_TAPS=" << NR_TAPS;
  cmd << " -DNR_STATION_FILTER_TAPS=" << NR_STATION_FILTER_TAPS;
  if (ps.delayCompensation()) 
  {
    cmd << " -DDELAY_COMPENSATION";
  }
  if (ps.correctBandPass()) 
  {
    cmd << " -DBANDPASS_CORRECTION";
  }
  cmd << " -DDEDISPERSION_FFT_SIZE=" << ps.dedispersionFFTsize();

  // TODO: do this only if compilation failed
  static bool printCudaCompileCommand = false; // TODO: -> LOG_ONCE()/WARN_ONCE()/similar; don't care about races
  if (!printCudaCompileCommand) 
  {
    printCudaCompileCommand = true;
    cout << "CUDA compilation to ptx command: " << cmd << endl;
  }

  // Derive output filename from input src filename by replacing the extension.
  string outputFilename(srcFilename);
  size_t idx = outputFilename.find_last_of('.');
  if (idx != string::npos) 
  {
    outputFilename.resize(idx);
  }
  outputFilename += ".ptx"; // output filename to be overwritten

  cmd << " " << srcFilename;
  int rv;
  if ((rv = std::system(cmd.str().c_str())) != 0) 
  { // blocking. If it takes too long, rewrite building all kernels at once. TODO: output goes to stdout/stderr -> collect it for proper logging
    throw SystemCallException("system", errno, THROW_ARGS); // system() is not really a syscall...
  }

  return 0;
}
