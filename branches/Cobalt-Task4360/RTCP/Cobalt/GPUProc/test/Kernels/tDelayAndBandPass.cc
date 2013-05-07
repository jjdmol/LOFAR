#include <string>
#include <cstdlib>


#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <GPUProc/cuda/Module.h>
#include <GPUProc/cuda/Error.h>

using namespace std;
int main()
{

   // Run a kernel with two different defines, should result in two different kernels.
   // Just run the compiler with two magic numbers and test for the existance of the numbers
  /*const char* lofarroot = getenv("LOFARROOT");  //runtime environment and thus installed
  if (lofarroot)
*/

  string kernelPath = "DelayAndBandPass.cu";
 
  // Get an instantiation of the default parameters
  CudaRuntimeCompiler::definitions_type definitions = CudaRuntimeCompiler::defaultDefinitions();
  CudaRuntimeCompiler::flags_type flags = CudaRuntimeCompiler::defaultFlags();
  flags.insert("gpu-architecture compute_20");


  definitions["NR_CHANNELS"] = "64";
  unsigned NR_CHANNELS = 64;
  definitions["NR_STATIONS"] = "20";
  unsigned NR_STATIONS = 20;
  definitions["NR_SAMPLES_PER_CHANNEL"] = "128";
  unsigned NR_SAMPLES_PER_CHANNEL = 128;
  definitions["NR_SAMPLES_PER_SUBBAND"] = "128";
  unsigned NR_SAMPLES_PER_SUBBAND = 128;
  definitions["NR_BITS_PER_SAMPLE"] = "8";
  unsigned NR_BITS_PER_SAMPLE = 8;
  definitions["NR_POLARIZATIONS"] = "2";
  unsigned NR_POLARIZATIONS = 2;
  definitions["NR_BEAMS"] = "8";
  unsigned NR_BEAMS = 8;
  definitions["USE_CUDA"] = "1";
  definitions["COMPLEX"] = "2";
  unsigned COMPLEX = 2;
  definitions["SUBBAND_BANDWIDTH"] = ".04";
  unsigned SUBBAND_BANDWIDTH = 4;
  definitions["BANDPASS_CORRECTION"] = "1";
  definitions["DELAY_COMPENSATION"] = "1";

  string ptx1 = CudaRuntimeCompiler::compileToPtx(kernelPath, 
                                    flags,
                                    definitions);

  cudaFree(0); // Hack to initialize the primary context. should use a proper api functions

  // The module compiler needs a target 
  vector<CUjit_option> options;
  vector<void*> optionValues;
  options.push_back(CU_JIT_TARGET_FROM_CUCONTEXT);
  optionValues.push_back(NULL); // no option value, but I suppose the array needs a placeholder

  // load the created module and get function entry point
  CUmodule     hModule  = 0;
  Module delayAndBandPassModule(ptx1.c_str(), options, optionValues);
  CUfunction   hKernel  =  delayAndBandPassModule.getKernelEntryPoint("applyDelaysAndCorrectBandPass");

  // Create the data arrays

  unsigned sizeFilteredData = NR_STATIONS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_CHANNELS * COMPLEX;
  float* rawFilteredData = new float[sizeFilteredData];
  for (unsigned idx = 0; idx < sizeFilteredData; ++idx)
    rawFilteredData[idx] = 0;
  // alloc memory on the device
  CUdeviceptr DevFilteredData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevFilteredData, sizeFilteredData * sizeof(float)));
  // copy to device
  checkCudaCall(cuMemcpyHtoD(DevFilteredData, 
                             rawFilteredData,
                             sizeFilteredData * sizeof(float)));

  unsigned sizeCorrectedData = NR_STATIONS * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX;
  float* rawCorrectedData = new float[sizeCorrectedData];
  for (unsigned idx = 0; idx < sizeFilteredData; ++idx)
    rawCorrectedData[idx] = 0;
  // alloc memory on the device
  CUdeviceptr DevCorrectedData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevCorrectedData, sizeCorrectedData * sizeof(float)));
  // copy to device
  checkCudaCall(cuMemcpyHtoD(DevCorrectedData, 
                             rawCorrectedData,
                             sizeCorrectedData * sizeof(float)));

  unsigned sizeDelaysAtBeginData = NR_STATIONS * NR_BEAMS * 2;
  float* rawDelaysAtBeginData = new float[sizeDelaysAtBeginData];
  for (unsigned idx = 0; idx < sizeDelaysAtBeginData; ++idx)
    rawDelaysAtBeginData[idx] = 0;
  // alloc memory on the device
  CUdeviceptr DevDelaysAtBeginData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevDelaysAtBeginData, sizeDelaysAtBeginData * sizeof(float)));
  // copy to device
  checkCudaCall(cuMemcpyHtoD(DevDelaysAtBeginData, 
                             rawDelaysAtBeginData,
                             sizeDelaysAtBeginData * sizeof(float)));

  unsigned sizeDelaysAfterEndData = NR_STATIONS * NR_BEAMS * 2;
  float* rawDelaysAfterEndData = new float[sizeDelaysAfterEndData];
  for (unsigned idx = 0; idx < sizeDelaysAfterEndData; ++idx)
    rawDelaysAfterEndData[idx] = 0;
  // alloc memory on the device
  CUdeviceptr DevDelaysAfterEndData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevDelaysAfterEndData, sizeDelaysAfterEndData * sizeof(float)));
  // copy to device
  checkCudaCall(cuMemcpyHtoD(DevDelaysAfterEndData, 
                             rawDelaysAfterEndData,
                             sizeDelaysAfterEndData * sizeof(float)));

  
   unsigned sizePhaseOffsetData = NR_STATIONS * 2;
  float* rawPhaseOffsetData = new float[sizePhaseOffsetData];
  for (unsigned idx = 0; idx < sizePhaseOffsetData; ++idx)
    rawPhaseOffsetData[idx] = 0;
  // alloc memory on the device
  CUdeviceptr DevPhaseOffsetData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevPhaseOffsetData, sizePhaseOffsetData * sizeof(float)));
  // copy to device
  checkCudaCall(cuMemcpyHtoD(DevPhaseOffsetData, 
                             rawPhaseOffsetData,
                             sizePhaseOffsetData * sizeof(float)));


   unsigned sizebandPassFactorsData = NR_CHANNELS;
  float* rawbandPassFactorsData = new float[sizebandPassFactorsData];
  for (unsigned idx = 0; idx < sizebandPassFactorsData; ++idx)
    rawbandPassFactorsData[idx] = 0;
  // alloc memory on the device
  CUdeviceptr DevbandPassFactorsData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevbandPassFactorsData, sizebandPassFactorsData * sizeof(float)));
  // copy to device
  checkCudaCall(cuMemcpyHtoD(DevbandPassFactorsData, 
                             rawbandPassFactorsData,
                             sizebandPassFactorsData * sizeof(float)));


  float* subbandFrequency = new float[1];
  subbandFrequency[0] = 10.0f;
  // alloc memory on the device
  CUdeviceptr DevSubbandFrequencyData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevSubbandFrequencyData, sizeof(float)));
  // copy to device
  checkCudaCall(cuMemcpyHtoD(DevSubbandFrequencyData, 
                             subbandFrequency,
                             sizeof(float)));

  unsigned* beam = new unsigned[1];
  beam[0] = 0;
  // alloc memory on the device
  CUdeviceptr DevBeamData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevBeamData, sizeof(unsigned)));
  // copy to device
  checkCudaCall(cuMemcpyHtoD(DevBeamData, 
                             beam,
                             sizeof(unsigned)));

  // Assure that all memory moves are complete and check for errors
  checkCudaCall(cudaDeviceSynchronize());

  void* kernel_func_args[8] = { &DevCorrectedData,
                                  &DevFilteredData,
                                  &DevSubbandFrequencyData,
                                  &DevBeamData,
                                  &DevDelaysAtBeginData,
                                  &DevDelaysAfterEndData,
                                  &DevPhaseOffsetData,
                                  &DevbandPassFactorsData};



  // Number of threads?
  int nrChannelsPerSubband = NR_CHANNELS;
  int nrStations = NR_STATIONS; 
  int MAXNRCUDATHREADS = 1024;//doet moet nog opgevraagt worden en niuet als magish getal
  size_t maxNrThreads = MAXNRCUDATHREADS;
  unsigned totalNrThreads = nrChannelsPerSubband * NR_POLARIZATIONS * 2; //ps.nrChannelsPerSubband()
  unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
  
  dim3 globalWorkSize(1, 1); 
  dim3 localWorkSize(1, 1); 

    cudaStream_t cuStream;
  checkCudaCall(cudaStreamCreate(&cuStream));
  
		
		
    checkCudaCall(cuLaunchKernel( hKernel, globalWorkSize.x, globalWorkSize.y, globalWorkSize.z, 
    localWorkSize.x, localWorkSize.y, localWorkSize.z, NULL, cuStream, kernel_func_args,0));

    checkCudaCall(cudaDeviceSynchronize());

  // Copy output vector from GPU buffer to host memory.
  checkCudaCall(cuMemcpyDtoH(rawCorrectedData, DevCorrectedData,
    sizeCorrectedData * sizeof(float)));
  cerr << "Retrieved from output: " << rawCorrectedData[0] << " " 
       << rawCorrectedData[1] << " " << rawCorrectedData[2] << endl;

    return 0;
}
