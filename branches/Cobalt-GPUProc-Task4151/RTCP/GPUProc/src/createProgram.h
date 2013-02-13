#include "lofar_config.h"

#include "CL/cl.hpp"
#include "Interface/Parset.h"

namespace LOFAR 
{
    namespace RTCP 
    {
        cl::Program createProgram(const Parset &ps, cl::Context &context, std::vector<cl::Device> &devices, const char *sources);
    }
}