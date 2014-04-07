#ifndef GPUPROC_PERFORMANCECOUNTER_H
#define GPUPROC_PERFORMANCECOUNTER_H

#include "CL/cl.hpp"

namespace LOFAR
{
    namespace RTCP 
    {
        class PerformanceCounter
        {
        public:
            PerformanceCounter(const std::string &name);
            ~PerformanceCounter();

            void doOperation(cl::Event &, size_t nrOperations, size_t nrBytesRead, size_t nrBytesWritten);

        private:
            static void eventCompleteCallBack(cl_event, cl_int /*status*/, void *counter);

            size_t	      totalNrOperations, totalNrBytesRead, totalNrBytesWritten;
            double	      totalTime;
            unsigned	      totalEvents;
            const std::string name;
        };
    }
}
#endif
