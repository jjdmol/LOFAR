#include "lofar_config.h"

#define __CL_ENABLE_EXCEPTIONS
#include "PerformanceCounter.h"

#include <global_defines.h>

#include "CL/cl.hpp"

#include <iomanip>
#include <iostream>

#include "OpenMP_Support.h"
#include "OpenCL_Support.h"

namespace LOFAR
{
    namespace RTCP 
    {

        extern bool	 profiling;


        PerformanceCounter::PerformanceCounter(const std::string &name)
            :
        totalNrOperations(0),
            totalNrBytesRead(0),
            totalNrBytesWritten(0),
            totalTime(0),
            totalEvents(0),
            name(name)
        {
        }


        PerformanceCounter::~PerformanceCounter()
        {
            if (totalTime > 0)
#pragma omp critical (cout)
                std::cout << std::setw(12) << name
                << std::setprecision(3)
                << ": avg. time = " << 1000 * totalTime / totalEvents << " ms, "
                "GFLOP/s = " << totalNrOperations / totalTime / 1e9 << ", "
                "R/W = " << totalNrBytesRead / totalTime / 1e9 << '+'
                << totalNrBytesWritten / totalTime / 1e9 << '='
                << (totalNrBytesRead + totalNrBytesWritten) / totalTime / 1e9 << " GB/s"
                << std::endl;
        }


        void PerformanceCounter::eventCompleteCallBack(cl_event ev, cl_int /*status*/, void *counter)
        {
            try {
                cl::Event event(ev);

                size_t queued = event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
                size_t submitted = event.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>();
                size_t start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
                size_t stop = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
                double seconds = (stop - start) / 1e9;

                if (seconds < 0 || seconds > 15)
#pragma omp critical (cout)
                    std::cout << "BAH! " << omp_get_thread_num() << ": " << queued << ' ' << submitted - queued << ' ' << start - queued << ' ' << stop - queued << std::endl;

#pragma omp atomic
                static_cast<PerformanceCounter *>(counter)->totalTime += seconds;

                // cl::~Event() decreases ref count
            } catch (cl::Error &error) {
                // ignore errors in callback function (OpenCL library not exception safe)
            }
        }


        void PerformanceCounter::doOperation(cl::Event &event, size_t nrOperations, size_t nrBytesRead, size_t nrBytesWritten)
        {
            // reference count between C and C++ conversions is serously broken in C++ wrapper
            cl_event ev = event();
            cl_int error = clRetainEvent(ev);

            if (error != CL_SUCCESS)
                throw cl::Error(error, "clRetainEvent");

            if (profiling) {
                event.setCallback(CL_COMPLETE, &PerformanceCounter::eventCompleteCallBack, this);

#pragma omp atomic
                totalNrOperations   += nrOperations;
#pragma omp atomic
                totalNrBytesRead    += nrBytesRead;
#pragma omp atomic
                totalNrBytesWritten += nrBytesWritten;
#pragma omp atomic
                ++ totalEvents;
            }
        }

    }
}