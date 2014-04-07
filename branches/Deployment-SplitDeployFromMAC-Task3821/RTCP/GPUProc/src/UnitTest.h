#ifndef GPUPROC_UNITTEST_H
#define GPUPROC_UNITTEST_H

#include "CL/cl.hpp"

#include "PerformanceCounter.h"
#include "Interface/Parset.h"

namespace LOFAR
{
    namespace RTCP 
    {
        class UnitTest
        {
        protected:
            UnitTest(const Parset &ps, const char *programName = 0);

            template <typename T> void check(T actual, T expected)
            {
                if (expected != actual) {
                    std::cerr << "Test FAILED: expected " << expected << ", computed " << actual << std::endl;
                    exit(1);
                } else {
                    std::cout << "Test OK" << std::endl;
                }
            }

            cl::Context context;
            std::vector<cl::Device> devices;
            cl::Program program;
            cl::CommandQueue queue;

            PerformanceCounter counter;
        };
    }
}
#endif
