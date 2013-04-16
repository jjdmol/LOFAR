
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>
#include <complex>
#include "complex.h"

using namespace std;

cudaError_t addWithCuda(int *c, std::complex<float> * output_complex, const int *a, const int *b, const std::complex<float> * input_complex, size_t size);

__global__ void addKernel(void *c_ptr, void *in_ptr, const void *a_ptr, const void *b_ptr, const void *out_ptr)
{
    int i = threadIdx.x;
    int *c = (int *) c_ptr;
    int *a = (int *) a_ptr;
    int *b = (int *) b_ptr;

    LOFAR::Cobalt::gpu::complex<float>*in = (LOFAR::Cobalt::gpu::complex<float>*) in_ptr;
    LOFAR::Cobalt::gpu::complex<float>*out = (LOFAR::Cobalt::gpu::complex<float>*) out_ptr;

    c[i] = a[i] + b[i];
    out[i] = in[i] + in[i];
    out[i] -= in[i];
    out[i] /=- in[i];
    out[i] = out[i];
    out[i] *= 10.0;

}

int main()
{
    const int arraySize = 5;
    const int a[arraySize] = { 1, 2, 3, 4, 5 };
    const int b[arraySize] = { 10, 20, 30, 40, 50 };
    const complex<float> complex_in[5] = { complex<float>(1.0,1.0),
                                                complex<float>(1,-1),
                                                complex<float>(-1,1),
                                                complex<float>(-1,-1),
                                                complex<float>(4,-4)};
    complex<float> complex_out[5] = { 0 };
    int c[arraySize] = { 0 };

    // Add vectors in parallel.
    cudaError_t cudaStatus = addWithCuda(c,complex_out, a, b, complex_in,arraySize);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addWithCuda failed!");
        return 1;
    }

    printf("{1,2,3,4,5} + {10,20,30,40,50} = {%d,%d,%d,%d,%d}\n",
        c[0], c[1], c[2], c[3], c[4]);
    
    cout << "complex numbers: {";
    for (int idx =0; idx < 5 ;++idx)
      cout << complex_out[idx] << ", ";
    cout << " }" << endl;

    // cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
        return 1;
    }

    return 0;
}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t addWithCuda(int *c,
                        std::complex<float>* output_complex,
                        const int *a, 
                        const int *b,
                        const std::complex<float>* input_complex,
                        size_t size)
{
    int *dev_a = 0;
    int *dev_b = 0;
    int *dev_c = 0;
    std::complex<float> *dev_in = 0;
    std::complex<float> *dev_out = 0;
    cudaError_t cudaStatus;


    cout << input_complex[0] << endl;
    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_a, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_b, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // allocate the complex buffers
    cudaStatus = cudaMalloc((void**)&dev_in, size * sizeof(std::complex<float>));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_out, size * sizeof(std::complex<float>));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_in, input_complex, size * sizeof(std::complex<float>), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    // Launch a kernel on the GPU with one thread for each element.
    addKernel<<<1, size>>>((void *)dev_c,(void *) dev_in, (const void *)dev_a, (const void *)dev_b, (const void *) dev_out);

    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(output_complex, dev_out, size * sizeof(std::complex<float>), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }


Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);
    cudaFree(dev_in);
    cudaFree(dev_out);
    
    return cudaStatus;
}
