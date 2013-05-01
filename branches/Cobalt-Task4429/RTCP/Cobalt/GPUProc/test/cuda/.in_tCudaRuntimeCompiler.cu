extern "C" {
__global__ void FIR_filter(void *filteredDataPtr)
{
  int *filteredData =(int *)filteredDataPtr;
  unsigned cpr = blockIdx.x*blockDim.x+threadIdx.x;
	filteredData[cpr] = NVIDIA_CUDA;
  int test = 20;
  return;

}
}