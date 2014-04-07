extern "C" {
  __global__ void FIR_filter(void *filteredDataPtr)
  {
    int *filteredData =(int *)filteredDataPtr;
    unsigned cpr = blockIdx.x*blockDim.x+threadIdx.x;
    filteredData[cpr] = NVIDIA_CUDA;  // define will be set on the command line as a parameter to the nvcc compiler
    int test = 20;
    return;
  }
}