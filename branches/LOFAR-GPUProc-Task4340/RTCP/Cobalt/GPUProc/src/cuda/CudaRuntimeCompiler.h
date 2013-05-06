#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <cstdio>   // popen, pget
#include <iostream>  
#include <string>
#include <sstream>
#include <iomanip>  // setprecision
#include <map>
#include <set>

// Collection of functions needed for runtime compilation of a kernel supplied 
// as a path to a ptx string.
namespace CudaRuntimeCompiler
{
  // flags
  typedef std::set<std::string> flags_type;

  // defines
  typedef std::map<std::string, std::string> definitions_type;

  // Return the set of default flags for the nvcc compilation of a cuda kernel in Cobalt
  const flags_type& defaultFlags();
  
  // Return the set of default definitions for the nvcc compilation of a cuda kernel in Cobalt
  const definitions_type& defaultDefinitions();
  
  // Performs a 'system' call of nvcc. Return the stdout of the command
  // on error no stdout is created and an exception is thrown
  std::string runNVCC(std::string cmd);
  
  // Create a nvcc command line string based on the input path, a set of flags and a map
  // of definitions. Use this command to call nvcc and compile the file at input path to a ptx file
  // which content is returned as a string
  std::string compileToPtx(const std::string& pathToCuFile, const flags_type& flags, const definitions_type& definitions);
  
  // overloaded function. Use the path and default flags and definitions to call nvcc
  std::string compileToPtx(const std::string& pathToCuFile);
}
