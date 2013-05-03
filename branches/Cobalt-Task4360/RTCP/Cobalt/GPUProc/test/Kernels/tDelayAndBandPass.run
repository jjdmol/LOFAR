#include <string>
#include <GPUProc/cuda/CudaRuntimeCompiler.h>


using namespace std;
int main()
{

   // Run a kernel with two different defines, should result in two different kernels.
   // Just run the compiler with two magic numbers and test for the existance of the numbers
   string kernelPath = "/home/wklijn/sources/4360/LOFAR/RTCP/Cobalt/GPUProc/src/Kernels/DelayAndBandPass.cu";

  
  // Get an instantiation of the default parameters
  CudaRuntimeCompiler::definitions_type definitions = CudaRuntimeCompiler::defaultDefinitions();

  // override the default with a magic number
  definitions["NVIDIA_CUDA"] = "123456";

  string ptx1 = CudaRuntimeCompiler::compileToPtx(kernelPath, 
                                    CudaRuntimeCompiler::defaultFlags(),
                                    definitions);
  return 0;
}