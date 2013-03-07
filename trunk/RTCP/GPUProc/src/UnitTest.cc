#include "lofar_config.h"
#include <complex>
#include "UnitTest.h"

#include "global_defines.h"
#include "Interface/Parset.h"
#include "Interface/SmartPtr.h"
#include "OpenCL_Support.h"
#include "createProgram.h"

namespace LOFAR
{
    namespace RTCP 
    {

        UnitTest::UnitTest(const Parset &ps, const char *programName)
            :
        counter(programName != 0 ? programName : "test", profiling)
        {
            createContext(context, devices);
            queue = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

            if (programName != 0)
                program = createProgram(ps, context, devices, programName);
        }


        bool UnitTest::fpEquals(double val, double ref, double epsilon) const {
            double err = std::abs(val - ref);
            if (ref >= 1.0e-1) {
                err /= ref; // prefer relative error cmp iff away from 0.0
            }
            return err < epsilon;
        }

        bool UnitTest::cfpEquals(std::complex<double> val, std::complex<double> ref, double epsilon) const {
            return fpEquals(val.real(), ref.real(), epsilon) &&
                   fpEquals(val.imag(), ref.imag(), epsilon);
        }
    }
}
