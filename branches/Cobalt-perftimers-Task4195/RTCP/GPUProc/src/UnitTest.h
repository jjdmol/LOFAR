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
             * Fuzzy compare of floating point value val with ref.
             * Use absolute error around 0.0 (ref within epsilon), otherwise use relative error.
             *
             * \param[in] val          value to test
             * \param[in] ref          reference value to test against
             * \param[in] epsilon      max absolute difference. Must be positive.
             *
             * A good epsilon depends on the computation, but pick something reasonable for our single precision tests and allow override.
             *
             * Return true if val is close enough to ref, false otherwise (including if val or ref is NaN).
             */
            bool fpEquals(double val, double ref, double epsilon = 1.0e-5) const;

            /**
             * See fpEquals(), but for complex values.
             */
            bool cfpEquals(std::complex<double> val, std::complex<double> ref, double epsilon = 1.0e-5) const;
        };
    }
}
#endif
