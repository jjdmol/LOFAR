#ifndef GPUPROC_UHEP_TRIGGERTEST_H
#define GPUPROC_UHEP_TRIGGERTEST_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/UHEP_TriggerKernel.h"
#include <iostream>

namespace LOFAR
{
    namespace RTCP 
    {   
        struct UHEP_TriggerTest : public UnitTest
        {
            UHEP_TriggerTest(const Parset &ps)
                :
            UnitTest(ps, "UHEP/Trigger.cl")
            {
                if (ps.nrTABs(0) >= 4 && 1024 * ps.nrSamplesPerChannel() > 100015) {
                    MultiArraySharedBuffer<float, 3> inputData(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel() * 1024], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<TriggerInfo, 1> triggerInfo(boost::extents[ps.nrTABs(0)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    UHEP_TriggerKernel trigger(ps, program, triggerInfo, inputData);

                    inputData[3][1][100015] = 1000;

                    inputData.hostToDevice(CL_FALSE);
                    trigger.enqueue(queue, counter);
                    triggerInfo.deviceToHost(CL_TRUE);

                    std::cout << "trigger info: mean = " << triggerInfo[3].mean << ", variance = " << triggerInfo[3].variance << ", bestValue = " << triggerInfo[3].bestValue << ", bestApproxIndex = " << triggerInfo[3].bestApproxIndex << std::endl;
                    //check(triggerInfo[3].mean, (float) (1000.0f * 1000.0f) / (float) (ps.nrSamplesPerChannel() * 1024));
                    check(triggerInfo[3].bestValue, 1000.0f * 1000.0f);
                    check(triggerInfo[3].bestApproxIndex, 100016U);
                }
            }
        };

    }
}
#endif