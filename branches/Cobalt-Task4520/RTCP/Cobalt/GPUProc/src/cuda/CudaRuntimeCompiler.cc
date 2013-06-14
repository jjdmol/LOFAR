//# CudaRuntimeCompiler.cc
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

#include "CudaRuntimeCompiler.h"

#include <cstdio>   // popen, pget
#include <stdexcept>
#include <iostream>  
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <boost/format.hpp>

#include <Common/Exception.h>
#include <Common/SystemCallException.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>

#include <GPUProc/global_defines.h>

using namespace std;

// Collection of functions needed for runtime compilation of a kernel supplied 
// as a path to a ptx string.
namespace LOFAR
{
  namespace Cobalt
  {

  // flags
  typedef std::set<std::string> flags_type;

  // defines
  typedef std::map<std::string, std::string> definitions_type;

  // Return the set of default flags for the nvcc compilation of a cuda kernel in Cobalt
  flags_type defaultFlags()
  {
    flags_type flags;

    using boost::format;

    //flags.insert("device-debug");
    flags.insert("use_fast_math"); // TODO: If this simplifies trig funcs, verify effect on the BPDelayKernel. Ideally, a Kernel specifies when to enable build flags.

    // gpu-architecture and -Ipath are set by createPTX()

    return flags;    
  };

  // // Return empty set of definitions for the nvcc compilation of a cuda kernel
  // definitions_type defaultDefinitions()
  // {
  //   definitions_type defs;

  //   return defs;
  // }

  
  // Performs a 'system' call of nvcc. Return the stdout of the command
  // on error no stdout is created and an exception is thrown
  std::string runNVCC(const std::string &cmd)
  {
    // *******************************************************
    // Call nvcc on the command line, get content as string
    stringstream ptxFileContent;                              // The ptx file, to be read from stdout
    char buffer [1024];       
    FILE * ptxFilePointer = popen(cmd.c_str(), "r" );       

    if (!ptxFilePointer)
      throw SystemCallException("popen", errno, THROW_ARGS);

    // Read the content of the file pointer
    while ( ! feof (ptxFilePointer) )                       //We do not get the cerr
    {
      if (fgets(buffer, sizeof buffer, ptxFilePointer) == NULL)  // FILE * can only be read with cstdio functions
        break;

      ptxFileContent << buffer;
    }

    // Close file stream
    if (fclose(ptxFilePointer) == EOF)
      throw SystemCallException("fclose", errno, THROW_ARGS);

    // *******************************************************

    // Fetch and return PTX, if any
    string ptxStr = ptxFileContent.str();

    if (ptxStr.empty()) 
    {
      LOG_DEBUG(" NVCC Compilation of cuda kernel failed. Command line arguments:");
      LOG_DEBUG(cmd);
      // log that we have a failed compile run
      THROW(Exception, "nvcc compilation failed!");
    }

    return ptxStr;
  }

  // Create a nvcc command line string based on the input path, a set of flags and a map
  // of definitions. Use this command to call nvcc and compile the file at input path to a ptx file
  // which content is returned as a string
  std::string compileToPtx(const std::string& pathToCuFile,
                           const flags_type& flags,
                           const definitions_type& definitions)
  {
    const string cudaCompiler = "nvcc"; 
    stringstream cmd("");
    cmd << cudaCompiler ;
    cmd << " " << pathToCuFile ;
    cmd << " --ptx";    

    // add the set of flags
    for (flags_type::const_iterator it=flags.begin(); it!=flags.end(); ++it)
      cmd << " --" << *it;  // flags should be prepended with a space and a minus

    // add the map of defines
    for (definitions_type::const_iterator it=definitions.begin(); it!=definitions.end(); ++it)
    {
      cmd << " -D" << it->first;
      if (!it->second.empty())
        cmd << "=" << it->second; // e.g. -DTEST=20
    }

    // output to stdout
    cmd << " -o -";
    LOG_INFO_STR("Runtime compilation of kernel, command: " << cmd.str());

    return runNVCC(cmd.str());
  };

  }
}

