#include "lofar_config.h"

#define __CL_ENABLE_EXCEPTIONS
#include "PerformanceCounter.h"

#include <Common/LofarLogger.h>

#include <global_defines.h>

#include "CL/cl.hpp"

#include <iomanip>
#include <iostream>

#include "OpenMP_Support.h"
#include "OpenCL_Support.h"

using namespace std;

namespace LOFAR
{
    namespace RTCP 
    {

        extern bool	 profiling;


        PerformanceCounter::PerformanceCounter(const std::string &name)
            :
            name(name),
            nrActiveEvents(0)
        {
        }


        PerformanceCounter::~PerformanceCounter()
        {
            // wait for all active events to finish
            {
               ScopedLock sl(mutex);

               while (nrActiveEvents > 0)
                 activeEventsLowered.wait(mutex);
            }

            // print final logs
            logTotal();
        }


        struct PerformanceCounter::figures PerformanceCounter::getTotal()
        {
          ScopedLock sl(mutex);
          
          return total;
        }


        void PerformanceCounter::logTotal()
        {
          ScopedLock sl(mutex);

          LOG_INFO_STR(
              "Event " << name << ": "
              << setprecision(3)
              << "avg. time = " << 1000 * total.avrRuntime() << " ms, "
              << "GFLOP/s = " << total.FLOPs() / 1e9 << ", "
              << "read = " << total.readSpeed() / 1e9 << " GB/s, "
              << "written = " << total.writeSpeed() / 1e9 << " GB/s, "
              << "total I/O = " << (total.readSpeed() + total.writeSpeed()) / 1e9 << " GB/s");
        }


        void PerformanceCounter::eventCompleteCallBack(cl_event ev, cl_int /*status*/, void *userdata)
        {
            struct callBackArgs *args = static_cast<struct callBackArgs *>(userdata);

            try {
                // extract performance information
                cl::Event event(ev);

                size_t queued    = event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
                size_t submitted = event.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>();
                size_t start     = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
                size_t stop      = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
                double seconds   = (stop - start) / 1e9;

                // sanity checks
                ASSERT(seconds >= 0);
                ASSERTSTR(seconds < 15, "Kernel took " << seconds << " seconds to execute: thread " << omp_get_thread_num() << ": " << queued << ' ' << submitted - queued << ' ' << start - queued << ' ' << stop - queued);

                args->figures.runtime = seconds;

                // add figures to total
                {
                  ScopedLock sl(args->this_->mutex);
                  args->this_->total += args->figures;
                }

                // cl::~Event() decreases ref count
            } catch (cl::Error &error) {
                // ignore errors in callBack function (OpenCL library not exception safe)
            }

            // we're done -- release event and possibly signal destructor
            {
              ScopedLock sl(args->this_->mutex);
              args->this_->nrActiveEvents--;
              args->this_->activeEventsLowered.signal();
            }

            delete args;
        }


        void PerformanceCounter::doOperation(cl::Event &event, size_t nrOperations, size_t nrBytesRead, size_t nrBytesWritten)
        {
            if (!profiling)
              return;

            // reference count between C and C++ conversions is serously broken in C++ wrapper
            cl_event ev = event();
            cl_int error = clRetainEvent(ev);

            if (error != CL_SUCCESS)
                throw cl::Error(error, "clRetainEvent");

            // obtain run time information
            struct callBackArgs *args = new callBackArgs;
            args->this_ = this;
            args->figures.nrOperations   = nrOperations;
            args->figures.nrBytesRead    = nrBytesRead;
            args->figures.nrBytesWritten = nrBytesWritten;
            args->figures.runtime        = 0.0;
            args->figures.nrEvents       = 1;

            {
              // allocate event as active
              ScopedLock sl(mutex);
              nrActiveEvents++;
            }

            event.setCallback(CL_COMPLETE, &PerformanceCounter::eventCompleteCallBack, args);
        }

    }
}
