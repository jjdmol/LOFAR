#ifndef GPUPROC_PERFORMANCECOUNTER_H
#define GPUPROC_PERFORMANCECOUNTER_H

#include "CL/cl.hpp"
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Condition.h>

namespace LOFAR
{
    namespace RTCP 
    {
        class PerformanceCounter
        {
        public:
            PerformanceCounter(const std::string &name);
            ~PerformanceCounter();

            // register an operation. runtime will be determined by OpenCL, the
            // rest of the figures have to be provided.
            void doOperation(cl::Event &, size_t nrOperations, size_t nrBytesRead, size_t nrBytesWritten);

            // performance figures
            struct figures {
              size_t nrOperations;
              size_t nrBytesRead;
              size_t nrBytesWritten;
              double runtime;

              size_t nrEvents;

              figures(): nrOperations(0), nrBytesRead(0), nrBytesWritten(0), runtime(0.0), nrEvents(0) {}

              struct figures &operator+=(const struct figures &other) {
                nrOperations   += other.nrOperations;
                nrBytesRead    += other.nrBytesRead;
                nrBytesWritten += other.nrBytesWritten;
                runtime        += other.runtime;
                nrEvents       += other.nrEvents;

                return *this;
              }

              double avrRuntime() const { return runtime/nrEvents; }
              double FLOPs() const      { return nrOperations/runtime; }
              double readSpeed() const  { return nrBytesRead/runtime; }
              double writeSpeed() const { return nrBytesWritten/runtime; }
            };

            // Return current running total figures
            struct figures getTotal();

            // Log the total figures
            void logTotal();

        private:
            // name of counter, for logging purposes
            const std::string name;

            // performance totals, plus lock
            struct figures total;

            // number of events that still have a callback waiting
            size_t nrActiveEvents;
            Condition activeEventsLowered;

            // lock for total and nrActiveEvents
            Mutex mutex;

            // call-back to get runtime information
            struct callBackArgs {
              PerformanceCounter *this_;
              struct figures figures;
            };

            static void eventCompleteCallBack(cl_event, cl_int /*status*/, void *userdata);
        };
    }
}
#endif
