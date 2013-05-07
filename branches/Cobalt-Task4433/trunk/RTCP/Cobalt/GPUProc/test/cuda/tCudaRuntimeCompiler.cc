#include <string>
#include <GPUProc/cuda/CudaRuntimeCompiler.h>


using namespace std;
int main()
{
   // Run a kernel with two different defines, should result in two different kernels.
   // Just run the compiler with two magic numbers and test for the existance of the numbers
   string kernelPath = "tCudaRuntimeCompiler.in_.cu";

  
  // Get an instantiation of the default parameters
  CudaRuntimeCompiler::definitions_type definitions = CudaRuntimeCompiler::defaultDefinitions();

  // override the default with a magic number
  definitions["NVIDIA_CUDA"] = "123456";

  string ptx1 = CudaRuntimeCompiler::compileToPtx(kernelPath, 
                                    CudaRuntimeCompiler::defaultFlags(),
                                    definitions);
  definitions["NVIDIA_CUDA"] = "654321";

  string ptx2 = CudaRuntimeCompiler::compileToPtx(kernelPath, 
                                    CudaRuntimeCompiler::defaultFlags(),
                                    definitions);

  // tests if the magic numbers are inserted into the ptx files
  if ((std::string::npos != ptx1.find("123456")) ||
      (std::string::npos != ptx2.find("654321")))
  {
    return 0;   
  }
  cerr << "did not find the magic numbers in the compiled ptx: 123456 and 654321" << endl;
  cout << ptx1 << ptx2 ;
  return -1;
}
