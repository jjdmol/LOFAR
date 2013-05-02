// license goes here

#include <cstddef>
#include <cstring>
#include <iostream>

#include <GPUProc/gpu_wrapper.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main() {
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    return 3;
  }

  gpu::Device dev(0);
  gpu::Context ctx(dev);

  const size_t bufSize = 64 * 1024 * 1024;
  const char expectedVal = 42;
  gpu::HostMemory hBuf1(bufSize);
  gpu::HostMemory hBuf2(bufSize);
  char *buf1 = hBuf1.get<char>();
  char *buf2 = hBuf2.get<char>();
  memset(buf1, expectedVal, bufSize * sizeof(char));
  memset(buf2,           0, bufSize * sizeof(char));

  gpu::DeviceMemory dBuf(bufSize);

  gpu::Stream strm;

  strm.writeBuffer(dBuf, hBuf1, false);
  strm.synchronize();

  // implicitly synchronous read back
  strm.readBuffer(hBuf2, dBuf, true);

  // check if data is there
  size_t nrUnexpectedVals = 0;
  for (size_t i = bufSize; i > 0; ) {
    i--;
    if (buf2[i] != expectedVal) {
      nrUnexpectedVals += 1;
    }
  }

  if (nrUnexpectedVals > 0) {
    cerr << "Got > 0 unexpected values: " << nrUnexpectedVals << endl;
    return 1;
  }

  return 0;
}

