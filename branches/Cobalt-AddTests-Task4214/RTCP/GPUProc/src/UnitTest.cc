#include "lofar_config.h"
#include "UnitTest.h"

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
        counter(programName != 0 ? programName : "test")
        {
            createContext(context, devices);
            queue = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

            if (programName != 0)
                program = createProgram(ps, context, devices, programName);
        }

    }
}