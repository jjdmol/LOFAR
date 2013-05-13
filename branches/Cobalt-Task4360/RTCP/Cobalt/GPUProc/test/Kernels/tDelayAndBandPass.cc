#include <string>
#include <cstdlib>
#include <sstream>
#include <cmath> 
#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <GPUProc/cuda/Module.h>
#include <GPUProc/cuda/Error.h>

using namespace std;

float * getInitializedArray(unsigned size, float defaultValue)
{
  float* createdArray =  new float[size];
  for (unsigned idx = 0; idx < size; ++idx)
    createdArray[idx] = defaultValue;
  return createdArray;
}

void createHostDataAndCopy(CUdeviceptr& devicePointer, float* hostPointer, unsigned size)
{
  checkCudaCall(cuMemAlloc(&devicePointer, size*sizeof(float)));
  checkCudaCall(cuMemcpyHtoD(devicePointer, hostPointer, size*sizeof(float)));
}


float * runTest(float bandPassFactor,
                float frequency = 0.0,
                float subbandWidth = 0.0,
                bool delayCompensation = false, 
                float delayBegin = 0.0,
                float delayEnd = 0.0,
                float PhaseOffset = 0.0)
{

  std::stringstream tostrstram("");

   // Run a kernel with two different defines, should result in two different kernels.
   // Just run the compiler with two magic numbers and test for the existance of the numbers
  /*const char* lofarroot = getenv("LOFARROOT");  //runtime environment and thus installed
  if (lofarroot)
  */
  string kernelPath = "DelayAndBandPass.cu";  //The test copies the kernel to the current dir (also the complex header, needed for compilation)
 
  // Get an instantiation of the default parameters
  CudaRuntimeCompiler::definitions_type definitions = CudaRuntimeCompiler::defaultDefinitions();
  CudaRuntimeCompiler::flags_type flags = CudaRuntimeCompiler::defaultFlags();
  flags.insert("gpu-architecture compute_20"); // The real devices will be 3.0

  // ****************************************
  // Compile to ptx
  // Set op string string pairs to be provided to the compiler as defines
  definitions["NR_CHANNELS"] = "16";
  unsigned NR_CHANNELS = 16;
  definitions["NR_STATIONS"] = "2";
  unsigned NR_STATIONS = 2;
  definitions["NR_SAMPLES_PER_CHANNEL"] = "64";
  unsigned NR_SAMPLES_PER_CHANNEL = 64;
  definitions["NR_SAMPLES_PER_SUBBAND"] = "1024";
  unsigned NR_SAMPLES_PER_SUBBAND = 1024;
  definitions["NR_BITS_PER_SAMPLE"] = "8";
  unsigned NR_BITS_PER_SAMPLE = 8;
  definitions["NR_POLARIZATIONS"] = "2";
  unsigned NR_POLARIZATIONS = 2;
  definitions["NR_BEAMS"] = "8";
  unsigned NR_BEAMS = 8;
  definitions["USE_CUDA"] = "1";
  definitions["COMPLEX"] = "2";
  unsigned COMPLEX = 2;
  tostrstram << subbandWidth;
  definitions["SUBBAND_BANDWIDTH"] = tostrstram.str();
  tostrstram.clear();
  float SUBBAND_BANDWIDTH = subbandWidth;
  definitions["BANDPASS_CORRECTION"] = "1";
  if (delayCompensation)
    definitions["DELAY_COMPENSATION"] = "1";
    
  string ptx1 = CudaRuntimeCompiler::compileToPtx(kernelPath, flags, definitions);
  // **************************************************************************

  

  // ****************************************************************
  // Load the module from ptx string 
  // The module compiler needs a target 
  cudaFree(0); // Hack to initialize the primary context. should use a proper api functions ( it does not exist?)

  vector<CUjit_option> options;
  vector<void*> optionValues;
  options.push_back(CU_JIT_TARGET_FROM_CUCONTEXT);
  optionValues.push_back(NULL); // no option value, but I suppose the array needs a placeholder

  // load the created module and get function entry point
  CUmodule     hModule  = 0;
  Module delayAndBandPassModule(ptx1.c_str(), options, optionValues);
  CUfunction   hKernel  =  delayAndBandPassModule.getKernelEntryPoint("applyDelaysAndCorrectBandPass");  // c function this no argument overloading
  // ******************************************************************************************

  // *************************************************************
  // Create the data arrays
  unsigned sizeFilteredData = NR_STATIONS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_CHANNELS * COMPLEX;
  CUdeviceptr DevFilteredData = (CUdeviceptr)NULL;
  float* rawFilteredData = getInitializedArray(sizeFilteredData, 1.0);
  createHostDataAndCopy(DevFilteredData, rawFilteredData, sizeFilteredData);

  unsigned sizeCorrectedData = NR_STATIONS * NR_CHANNELS * NR_SAMPLES_PER_CHANNEL * NR_POLARIZATIONS * COMPLEX;
  CUdeviceptr DevCorrectedData = (CUdeviceptr)NULL;
  float* rawCorrectedData = getInitializedArray(sizeCorrectedData, 42); 
  createHostDataAndCopy(DevCorrectedData, rawCorrectedData, sizeCorrectedData);

  unsigned sizeDelaysAtBeginData = NR_STATIONS * NR_BEAMS * 2;  //not complex: it is two float number
  CUdeviceptr DevDelaysAtBeginData = (CUdeviceptr)NULL;
  float* rawDelaysAtBeginData = getInitializedArray(sizeDelaysAtBeginData, delayBegin);
  createHostDataAndCopy(DevDelaysAtBeginData, rawDelaysAtBeginData, sizeDelaysAtBeginData );

  unsigned sizeDelaysAfterEndData = NR_STATIONS * NR_BEAMS * 2; //not complex: it is two float number
  CUdeviceptr DevDelaysAfterEndData = (CUdeviceptr)NULL;
  float* rawDelaysAfterEndData = getInitializedArray(sizeDelaysAfterEndData, delayEnd);
  createHostDataAndCopy(DevDelaysAfterEndData, rawDelaysAfterEndData, sizeDelaysAfterEndData);

  unsigned sizePhaseOffsetData = NR_STATIONS * 2; //not complex: it is two float number
  CUdeviceptr DevPhaseOffsetData = (CUdeviceptr)NULL;
  float* rawPhaseOffsetData = getInitializedArray(sizePhaseOffsetData, PhaseOffset);
  createHostDataAndCopy(DevPhaseOffsetData, rawPhaseOffsetData, sizePhaseOffsetData);

  unsigned sizebandPassFactorsData = NR_CHANNELS;
  CUdeviceptr DevbandPassFactorsData = (CUdeviceptr)NULL;
  float* rawbandPassFactorsData = getInitializedArray(sizebandPassFactorsData, bandPassFactor);
  createHostDataAndCopy(DevbandPassFactorsData, rawbandPassFactorsData, sizebandPassFactorsData);
  
  unsigned sizeSubbandFrequency = 1;
  CUdeviceptr DevSubbandFrequencyData = (CUdeviceptr)NULL;
  float* subbandFrequency = getInitializedArray(sizeSubbandFrequency, frequency);
  createHostDataAndCopy(DevSubbandFrequencyData, subbandFrequency, sizeSubbandFrequency);

  unsigned* beam = new unsigned[1];
  beam[0] = 0;
  CUdeviceptr DevBeamData = (CUdeviceptr)NULL;;
  checkCudaCall(cuMemAlloc(&DevBeamData, sizeof(unsigned)));
  checkCudaCall(cuMemcpyHtoD(DevBeamData, beam, sizeof(unsigned)));

  // Assure that all memory moves are complete and check for errors
  checkCudaCall(cudaDeviceSynchronize());
  // ****************************************************************************

  // Run the kernel on the created data
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


  //globalWorkSize = cl::NDRange(256, ps.nrChannelsPerSubband() == 1 ? 1 : ps.nrChannelsPerSubband() / 16, ps.nrStations());
  //localWorkSize = cl::NDRange(256, 1, 1);
  // Use static worksize 1 for now
  dim3 globalWorkSize(1, NR_CHANNELS == 1? 1: NR_CHANNELS/16, NR_STATIONS); 
  dim3 localWorkSize(256, 1,1); 

  // Run the kernel
  cudaStream_t cuStream;
  checkCudaCall(cudaStreamCreate(&cuStream));
  
	checkCudaCall(cuLaunchKernel( hKernel, globalWorkSize.x, globalWorkSize.y, globalWorkSize.z, 
    localWorkSize.x, localWorkSize.y, localWorkSize.z, NULL, cuStream, kernel_func_args,0));

  checkCudaCall(cudaDeviceSynchronize());
  // ***********************************************************

  // Copy output vector from GPU buffer to host memory.
  checkCudaCall(cuMemcpyDtoH(rawCorrectedData, DevCorrectedData, sizeCorrectedData * sizeof(float)));

  checkCudaCall(cudaDeviceSynchronize());
  // Create the return values
  float *firstAndLastComplex = new float[4];
  // Return the first complex
  firstAndLastComplex[0] = rawCorrectedData[0];
  firstAndLastComplex[1] = rawCorrectedData[1];
  //return the last complex number
  firstAndLastComplex[2] = rawCorrectedData[sizeCorrectedData-2];
  firstAndLastComplex[3] = rawCorrectedData[sizeCorrectedData-1];
  return firstAndLastComplex;
}

int main()
{
  // ***********************************************************
  // Test if the bandpass correction factor is applied correctly in isolation
  float bandPassFactor = 2.0;
  bool delayCompensation = false;
  float * results;

  // ********************************************************
  // The input samples are all ones
  // After correction, multiply with 2.
  // The first and the last complex values are retrieved. They should be scaled with the bandPassFactor == 2
  results = runTest(bandPassFactor, delayCompensation);
  for (unsigned idx = 0; idx < 4; ++idx)
  {
    if (results[idx] != 2.0)
    {
      cerr << "Bandpass correction returned an incorrect value at index" << idx << endl;
      cerr << " expected: 2, 2, 2, 2" << endl;    
      cerr << " received: " << results[0] << ", " << results[1] << ", "<< results[2] << ", "<< results[3] << endl;
      return -1;
    }
  }


  //**********************************************************************
  // Delaycompensation but only for the phase ofsets:
  // All computations the drop except the phase ofset of 1,0 which is fed into a an cexp
  // cexp(1) = e = 2.71828
  results = runTest(1.0,   // bandpass factor
                    1.0,   // frequency
                    1.0,   // SUBBAND_BANDWIDTH set to zero forces
                    true,  // delayCompensation
                    0.0,   // delays begin  
                    0.0,   // delays end
                    1.0);  // phase offsets

  for (unsigned idx = 0; idx < 4; ++idx)
  {
    if ((results[idx] -  2.71828) > 0.00001 )
    {
      cerr << " phase offsets correction returned an incorrect value at index" << idx << endl;
      cerr << " expected: 2.71828, 2.71828, 2.71828, 2.71828" << endl;    
      cerr << " received: " << results[0] << ", " << results[1] << ", "<< results[2] << ", "<< results[3] << endl;
      return -1;
    }
  }


  //****************************************************************************
  // delays  begin and end both 1 no phase offset frequency 1 width 1
  // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
  //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
  // exp(-3.14159+0 i) == 0.04312
  results = runTest(1.0,   // bandpass factor
                    1.0,   // frequency
                    1.0,   // SUBBAND_BANDWIDTH set to zero forces
                    true,  // delayCompensation
                    1.0,   // delays begin  
                    1.0,   // delays end
                    0.0);  // phase offsets

  for (unsigned idx = 0; idx < 4; ++idx)
  {
    if (fabs(results[idx] -  0.04321) > 0.00001 )
    {
      cerr << " delays  begin and end both 1 no phase offset frequency 1 width 1" << idx << endl;
      cerr << " expected:  0.04321,  0.04321,  0.04321,  0.04321" << endl;    
      cerr << " received: " << results[0] << ", " << results[1] << ", "<< results[2] << ", "<< results[3] << endl;
      return -1;
    }
  }

    //****************************************************************************
  // delays  begin 1 and end 0 no phase offset frequency 1 width 1
  // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
  //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
  // exp(-3.14159+0 i) == 0.04312
  // The later sets of samples are calculate as:
  // vX = vX * dvX;  The delays are multiplied because we are calculating with exponents
  // Ask john Romein for more details
  results = runTest(1.0,   // bandpass factor
                    1.0,   // frequency
                    1.0,   // SUBBAND_BANDWIDTH set to zero forces
                    true,  // delayCompensation
                    1.0,   // delays begin  
                    0.0,   // delays end
                    0.0);  // phase offsets

  for (unsigned idx = 0; idx < 4; ++idx)
  {  // Magic number ask John Romein why they are correct
    if(!((fabs(results[idx] -  0.04321) < 0.00001) || (fabs(results[idx] -  0.952098) < 0.00001)))
    {
      cerr << " delays  begin and end both 1 no phase offset frequency 1 width 1" << idx << endl;
      cerr << " expected:  0.04321,  0.04321,  0.952098,  0.952098" << endl;    
      cerr << " received: " << results[0] << ", " << results[1] << ", "<< results[2] << ", "<< results[3] << endl;
      return -1;
    }
  }

    //****************************************************************************
  // delays  begin 1 and end 0 no phase offset frequency 1 width 1
  // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
  //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
  // exp(-3.14159+0 i) == 0.04312
  // The later sets of samples are calculate as:
  // vX = vX * dvX;  The delays are multiplied because we are calculating with exponents
  // Ask john Romein for more details
  // In this test the phase offsetss are also compensated
  results = runTest(2.0,   // bandpass factor (weights == 2)
                    1.0,   // frequency
                    1.0,   // SUBBAND_BANDWIDTH set to zero forces
                    true,  // delayCompensation
                    1.0,   // delays begin  
                    0.0,   // delays end
                    1.0);  // phase offsets (correct with e = 2.71828)

  for (unsigned idx = 0; idx < 4; ++idx)
  {  // Magic number ask John Romein why they are correct
    if(!((fabs(results[idx] -  0.04321 * 2.71828 * 2) < 0.0001)  ||
        (fabs(results[idx] -  2.58807* 2) < 0.0001)))
    {
      cerr << " delays  begin and end both 1 no phase offset frequency 1 width " << idx << endl;
      cerr << " expected:  0.04321,  0.04321,  0.952098,  0.952098" << endl;    
      cerr << " received: " << results[0] << ", " << results[1] << ", "<< results[2] << ", "<< results[3] << endl;
      return -1;
    }
  }

  return 0;
}

