#include "lofar_config.h"

#define __CL_ENABLE_EXCEPTIONS
#include "PerformanceCounter.h"

#include <Common/LofarLogger.h>

#include "CL/cl.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "OpenMP_Support.h"
#include "OpenCL_Support.h"

using namespace std;

namespace LOFAR
{
    namespace RTCP 
    {
        PerformanceCounter::PerformanceCounter(const std::string &name, bool profiling)
            :
            name(name),
            profiling(profiling),
            nrActiveEvents(0)
        {
        }


        PerformanceCounter::~PerformanceCounter()
        {
            waitForAllOperations();

            LOG_INFO_STR("Event " << setw(20) << name << ": " << total.log());
        }


        void PerformanceCounter::waitForAllOperations()
        {
            ScopedLock sl(mutex);

            while (nrActiveEvents > 0)
                activeEventsLowered.wait(mutex);
        }


        struct PerformanceCounter::figures PerformanceCounter::getTotal()
        {
          ScopedLock sl(mutex);
          
          return total;
        }


        std::string PerformanceCounter::figures::log() const
        {
          std::stringstream str;
          str << setprecision(3)
              << "n = " << nrEvents << " "
              << "avg. time = " << 1000 * avrRuntime() << " ms, "
              << "GFLOP/s = " << FLOPs() / 1e9 << ", "
              << "read = " << readSpeed() / 1e9 << " GB/s, "
              << "written = " << writeSpeed() / 1e9 << " GB/s, "
              << "total I/O = " << (readSpeed() + writeSpeed()) / 1e9 << " GB/s";

          return str.str();
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
