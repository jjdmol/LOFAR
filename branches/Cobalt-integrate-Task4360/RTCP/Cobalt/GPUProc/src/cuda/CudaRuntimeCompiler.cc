#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <cstdio>   // popen, pget
#include <iostream>  
#include <string>
#include <sstream>
#include <iomanip>  // setprecision
#include <map>
#include <set>

#include "CudaRuntimeCompiler.h"

// Collection of functions needed for runtime compilation of a kernel supplied 
// as a path to a ptx string.
namespace CudaRuntimeCompiler
{
  // flags
  typedef std::set<std::string> flags_type;

  // defines
  typedef std::map<std::string, std::string> definitions_type;

  // Return the set of default flags for the nvcc compilation of a cuda kernel in Cobalt
  const flags_type& defaultFlags()
  {
    static flags_type flags;
    if (flags.empty())  //fill with default values
    {
      //flags.insert("device-debug ");
      flags.insert("use_fast_math");
      //flags.insert("gpu-architecture compute_30");  force that the computate architecture needs to be specified
      // opencl specific: --source-in-ptx  -m64
    }
    return flags;    
  };

  // Return the set of default definitions for the nvcc compilation of a cuda kernel in Cobalt
  const definitions_type& defaultDefinitions()
  {
    static definitions_type definitions;
    if (definitions.empty())  //fill with default values
    {
      definitions["NVIDIA_CUDA"] = "1";       // left-over from OpenCL for Correlator.cl/.cu 
      definitions["NR_BITS_PER_SAMPLE"] = "20";  //ps.nrBitsPerSample();
    }
    return definitions;  
  }
  
  // Performs a 'system' call of nvcc. Return the stdout of the command
  // on error no stdout is created and an exception is thrown
  std::string runNVCC(std::string cmd)
  {
    // *******************************************************
    // Call nvcc on the command line, get content as string
    std::string ptxFileContent;                              // The ptx file, to be red from stdout
    char buffer [1024];       
    FILE * ptxFilePointer = popen(cmd.c_str(), "r" );       

    // First test if we have content. nvcc does not provide output on stdout when compilation failes
    if (fgets (buffer , 100 , ptxFilePointer)  == NULL )    // if we cannot read nothing from the stream
    {
      // log that we have a failed compile run

      throw "nvcc compilation failed!";                     // TODO: Need to be a  lofar::gpu::exeption.
    }
    //Now read the content of the file pointer
    while ( ! feof (ptxFilePointer) )                       //We do not get the cerr
    {
      if ( fgets (buffer , 100 , ptxFilePointer) == NULL )  // FILE * can only be red with cstdio functions
        break;
      ptxFileContent += buffer;
    }

    fclose (ptxFilePointer);                                // Remember to close the FILE *. Otherwise linux pipe error
    // *******************************************************

    return ptxFileContent;
  }

  // Create a nvcc command line string based on the input path, a set of flags and a map
  // of definitions. Use this command to call nvcc and compile the file at input path to a ptx file
  // which content is returned as a string
  std::string compileToPtx(const std::string& pathToCuFile, const flags_type& flags, const definitions_type& definitions)
  {
    const std::string cudaCompiler = "nvcc"; 
    std::stringstream cmd("");
    cmd << cudaCompiler ;
    cmd << " " << pathToCuFile ;
    cmd << " --ptx";                       

    // add the set of flags
    for (flags_type::const_iterator it=flags.begin(); it!=flags.end(); ++it)
      cmd << " --" << *it;  // flags should be prepended with a space and a minus

    // add the map of defines
    for (definitions_type::const_iterator it=definitions.begin(); it!=definitions.end(); ++it)
      cmd << " -D" << it->first << "=" << it->second;  //eg:  -DTEST=20

    // output to stdout
    cmd << " -o -";
    std::cout << "Runtime compilation of kernel, command: " << std::endl << cmd.str()  << std::endl;

    return runNVCC(cmd.str());
  };

  // overloaded function. Use the path and default flags and definitions to call nvcc
  std::string compileToPtx(const std::string& pathToCuFile)
  {
    // compile with the default flags and definitions
    return CudaRuntimeCompiler::compileToPtx(pathToCuFile, defaultFlags(), defaultDefinitions());
  };
}
