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


            /**
             * Fuzzy compare of float val with ref.
             *
             * \param[in] val          value to test
             * \param[in] ref          reference value to test against
             * \param[in] epsilonf     max absolute difference.
             *
             * An epsilonf is actually computation dependent, but for our test cases, this is good (and broad) enough.
             * Return true if val is close enough to ref, false otherwise (including if val or ref is NaN).
             */
            bool equalsRelError(float val, float ref, float epsilonf = 1.0e-5f) const;
        };
    }
}
#endif
