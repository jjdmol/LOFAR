//# createProgram.cc
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

#include <GPUProc/createProgram.h>

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <Common/SystemUtil.h>
#include <Common/SystemCallException.h>
#include <Stream/FileStream.h>

#include <GPUProc/global_defines.h>

#define BUILD_MAX_LOG_SIZE	4095

namespace LOFAR
{
  namespace Cobalt
  {
    using namespace std;

    gpu::Module createProgram(const Parset &ps, gpu::Context &context, vector<string> &targets, const string &srcFilename)
    {
#if 0
      string cudaCompiler(CUDA_TOOLKIT_ROOT_DIR);
      if (!cudaCompiler.empty()) {
        cudaCompiler += "/bin/nvcc";
      }
      if (::access(cudaCompiler.c_str(), X_OK) == -1)
#endif
      ostringstream cmd;
      cmd << "nvcc"; // TODO: allow cmd-line arg override
      //cmd << " --compiler-bindir /usr/bin/g++-4.6"; // TODO: generalize; also in the tCudaRuntimeCompiler test
      cmd << " --ptx";                         // Request intermediate format (ptx) as output. We may want to view or stir it.
      cmd << " -I" << dirname(__FILE__);       // TODO: read from installed directory if installed, or from this directory if testing (?)
#ifdef LOFAR_DEBUG
      cmd << " -g -G"; // --source-in-ptx";
#endif
      //cmd << " -m32";                          // -m64 (default) takes extra regs for a ptr, but allows >2 GB buffers, which we need in some kernels
      // use opt level 99
      // -Ox is only for host code. Opt backend compilation below. (TODO: still pass -O2 to host compiler?)
      //cmd << " --gpu-architecture compute_30"; // TODO: incorrectly assumes one, single virt arch that we don't have everywhere; check out targets arg instead (and fix 2nd build phase below too)
      cmd << " --use_fast_math"; // we believe we can get away with this for LOFAR online DSP

      cmd << " -DNVIDIA_CUDA"; // left-over from OpenCL for Correlator.cl/.cu
      //if (targets[0].getName() == "GeForce GTX 680") // TODO: move this and many -D opts below into kernel-specific build arg routines
      //cmd << " -DUSE_FLOAT4_IN_CORRELATOR";
      cmd << " -DNR_BITS_PER_SAMPLE=" << ps.nrBitsPerSample();
      cmd << " -DSUBBAND_BANDWIDTH=" << std::setprecision(7) << ps.subbandBandwidth() << 'f';
      cmd << " -DNR_SUBBANDS=" << ps.nrSubbands();
      cmd << " -DNR_CHANNELS=" << ps.nrChannelsPerSubband();
      cmd << " -DNR_STATIONS=" << ps.nrStations();
      cmd << " -DNR_SAMPLES_PER_CHANNEL=" << ps.nrSamplesPerChannel();
      cmd << " -DNR_SAMPLES_PER_SUBBAND=" << ps.nrSamplesPerSubband();
      cmd << " -DNR_BEAMS=" << ps.nrBeams();
      cmd << " -DNR_TABS=" << ps.nrTABs(0); // TODO: this restricts to the 1st TAB; make more flex
      cmd << " -DNR_COHERENT_STOKES=" << ps.nrCoherentStokes();
      cmd << " -DNR_INCOHERENT_STOKES=" << ps.nrIncoherentStokes();
      cmd << " -DCOHERENT_STOKES_TIME_INTEGRATION_FACTOR=" << ps.coherentStokesTimeIntegrationFactor();
      cmd << " -DINCOHERENT_STOKES_TIME_INTEGRATION_FACTOR=" << ps.incoherentStokesTimeIntegrationFactor();
      cmd << " -DNR_POLARIZATIONS=" << NR_POLARIZATIONS;
      cmd << " -DNR_TAPS=" << NR_TAPS;
      cmd << " -DNR_STATION_FILTER_TAPS=" << NR_STATION_FILTER_TAPS;
      if (ps.delayCompensation()) {
        cmd << " -DDELAY_COMPENSATION";
      }
      if (ps.correctBandPass()) {
        cmd << " -DBANDPASS_CORRECTION";
      }
      cmd << " -DDEDISPERSION_FFT_SIZE=" << ps.dedispersionFFTsize();

      // TODO: do this only if compilation failed
      static bool printCudaCompileCommand = false; // TODO: -> LOG_ONCE()/WARN_ONCE()/similar; don't care about races
      if (!printCudaCompileCommand) {
        printCudaCompileCommand = true;
        cout << "CUDA compilation to ptx command: " << cmd << endl;
      }

      // Derive output filename from input src filename by replacing the extension.
      string outputFilename(srcFilename);
      size_t idx = outputFilename.find_last_of('.');
      if (idx != string::npos) {
        outputFilename.resize(idx);
      }
      outputFilename += ".ptx"; // output filename to be overwritten

      cmd << " " << srcFilename;
      int rv;
      if ((rv = std::system(cmd.str().c_str())) != 0) { // blocking. If it takes too long, rewrite building all kernels at once. TODO: output goes to stdout/stderr -> collect it for proper logging
        throw SystemCallException("system", errno, THROW_ARGS); // system() is not really a syscall...
      }

// TODO: separate function!!!
      // Compile ptx further.
      // To pass build flags, we need to pass a buffer, so read in the file.
      FileStream file(outputFilename);
      vector<char> buf(file.size());
      file.read(&buf[0], buf.size());

      /*
       * JIT compilation options.
       * Note: need to pass a void* with option vals. Preferably, do not alloc dyn (mem leaks on exc).
       * Instead, use local vars for small variables and vector<char> xxx; passing &xxx[0] for output c-strings.
       */
      vector<CUjit_option> options;
      vector<void*> optionValues;

#if 0
      unsigned int maxRegs = 63; // TODO: write this up
      options.push_back(CU_JIT_MAX_REGISTERS);
      optionValues.push_back(&maxRegs);

      unsigned int thrPerBlk = 256; // input and output val
      options.push_back(CU_JIT_THREADS_PER_BLOCK);
      optionValues.push_back(&thrPerBlk); // can be read back
#endif

      unsigned int infoLogSize  = BUILD_MAX_LOG_SIZE + 1; // input and output var for JIT compiler
      unsigned int errorLogSize = BUILD_MAX_LOG_SIZE + 1; // idem (hence not the a single var or const)

      vector<char> infoLog(infoLogSize);
      options.push_back(CU_JIT_INFO_LOG_BUFFER);
      optionValues.push_back(&infoLog[0]);

      options.push_back(CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES);
      optionValues.push_back(&infoLogSize);

      vector<char> errorLog(errorLogSize);
      options.push_back(CU_JIT_ERROR_LOG_BUFFER);
      optionValues.push_back(&errorLog[0]);

      options.push_back(CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES);
      optionValues.push_back(&errorLogSize);

      float jitWallTime = 0.0f; // output val (init it anyway), in milliseconds
      options.push_back(CU_JIT_WALL_TIME);
      optionValues.push_back(&jitWallTime);

#if 0
      unsigned int optLvl = 4; // 0-4, default 4
      options.push_back(CU_JIT_OPTIMIZATION_LEVEL);
      optionValues.push_back(&optLvl);

      options.push_back(CU_JIT_TARGET_FROM_CUCONTEXT);
      optionValues.push_back(NULL); // no option value, but I suppose the array needs a placeholder
#endif
/*
      CUjit_target_enum target = CU_TARGET_COMPUTE_30; // TODO: determine val from auto-detect or whatever
      options.push_back(CU_JIT_TARGET);
      optionValues.push_back(&target);
*/
#if 0
      CUjit_fallback_enum fallback = CU_PREFER_PTX;
      options.push_back(CU_JIT_FALLBACK_STRATEGY);
      optionValues.push_back(&fallback);
#endif

      try {
        gpu::Module module(&buf[0], options, optionValues);
        // TODO: check what the ptx compiler prints. Don't print bogus. See if infoLogSize indeed is set to 0 if all cool.
        // TODO: maybe retry if buffer len exhausted, esp for errors
        if (infoLogSize > infoLog.size()) { // zero-term log and guard against bogus JIT opt val output
          infoLogSize = infoLog.size();
        }
        infoLog[infoLogSize - 1] = '\0';
        cout << "Build info for '" << outputFilename << "' (build time: " << jitWallTime << " us):" << endl << &infoLog[0] << endl;
        return module;
      } catch (gpu::CUDAException& exc) {
        if (errorLogSize > errorLog.size()) { // idem
          errorLogSize = errorLog.size();
        }
        errorLog[errorLogSize - 1] = '\0';
        cerr << "Build errors for '" << outputFilename << "' (build time: " << jitWallTime << " us):" << endl << &errorLog[0] << endl;
        throw;
      }
    }

  } // namespace Cobalt
} // namespace LOFAR

