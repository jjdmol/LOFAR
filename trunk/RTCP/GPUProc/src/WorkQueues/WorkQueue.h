#ifndef GPUPROC_WORKQUEUE_H
#define GPUPROC_WORKQUEUE_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "Interface/SmartPtr.h"
#include "PerformanceCounter.h"
#include "OpenCL_Support.h"

#include <string>
#include <map>

namespace LOFAR
{
    namespace RTCP 
    {
        class WorkQueue
        {
        public:
            WorkQueue(cl::Context &context, cl::Device		&device,  unsigned gpuNumber, const Parset	&ps);

            const unsigned	gpu;
            cl::Device		&device;
            cl::CommandQueue	queue;

            std::map<std::string, SmartPtr<PerformanceCounter> > counters;

        protected:
            const Parset	&ps;

            void addCounter(const std::string &name);
        };

    }
}
#endif
