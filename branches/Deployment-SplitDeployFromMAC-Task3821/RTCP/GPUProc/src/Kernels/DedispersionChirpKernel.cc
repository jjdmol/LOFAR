#include "lofar_config.h"    

#include "Kernel.h"
#include "DedispersionChirpKernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

namespace LOFAR
{
    namespace RTCP 
    {      
            DedispersionChirpKernel::DedispersionChirpKernel(const Parset &ps, cl::Program &program, cl::CommandQueue &queue, cl::Buffer &buffer, cl::Buffer &DMs)
                :
            Kernel(ps, program, "applyChirp")
            {
                setArg(0, buffer);
                setArg(1, DMs);

                size_t maxNrThreads;
                getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
                unsigned fftSize = ps.dedispersionFFTsize();

                globalWorkSize = cl::NDRange(fftSize, ps.nrSamplesPerChannel() / fftSize, ps.nrChannelsPerSubband());
                //std::cout << "globalWorkSize = NDRange(" << fftSize << ", " << ps.nrSamplesPerChannel() / fftSize << ", " << ps.nrChannelsPerSubband() << ')' << std::endl;

                if (fftSize <= maxNrThreads) {
                    localWorkSize = cl::NDRange(fftSize, 1, maxNrThreads / fftSize);
                    //std::cout << "localWorkSize = NDRange(" << fftSize << ", 1, " << maxNrThreads / fftSize << ')' << std::endl;
                } else {
                    unsigned divisor;

                    for (divisor = 1; fftSize / divisor > maxNrThreads || fftSize % divisor != 0; divisor ++)
                        ;

                    localWorkSize = cl::NDRange(fftSize / divisor, 1, 1);
                    //std::cout << "localWorkSize = NDRange(" << fftSize / divisor << ", 1, 1))" << std::endl;
                }

                nrOperations = (size_t) NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * (9 * ps.nrTABs(0) + 17),
                    nrBytesRead  = nrBytesWritten = sizeof(std::complex<float>) * ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel();
            }

            void DedispersionChirpKernel::enqueue(cl::CommandQueue &queue, PerformanceCounter &counter, double subbandFrequency)
            {
                setArg(2, (float) subbandFrequency);
                Kernel::enqueue(queue, counter);
            }
    }
}
