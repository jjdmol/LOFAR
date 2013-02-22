#include "lofar_config.h"
#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#include <global_defines.h>

#include "OpenMP_Support.h"
#include "Common/LofarLogger.h"
#include "Common/Exception.h"
#include <cstdlib>
#include "Interface/Parset.h"
#include <iostream>
#include <cstring>
#include "OpenCL_Support.h"
#include <cstdlib>
#include <cstdio>

//functionality moved to individual sources
#include "createProgram.h"
#include "PerformanceCounter.h"
#include "UnitTest.h"
#include "Kernel.h"
#include "FFT_Kernel.h"
#include "FFT_Plan.h"

#include "Kernels/FIR_FilterKernel.h"
#include "Kernels/DelayAndBandPassKernel.h"
#include "Kernels/CorrelatorKernel.h"
#include "Kernels/IntToFloatKernel.h"
#include "Kernels/IncoherentStokesKernel.h"
#include "Kernels/BeamFormerKernel.h"
#include "Kernels/BeamFormerTransposeKernel.h"
#include "Kernels/DedispersionChirpKernel.h"
#include "Kernels/CoherentStokesKernel.h"
#include "Kernels/UHEP_BeamFormerKernel.h"
#include "Kernels/UHEP_TransposeKernel.h"
#include "Kernels/UHEP_InvFFT_Kernel.h"
#include "Kernels/UHEP_InvFIR_Kernel.h"
#include "Kernels/UHEP_TriggerKernel.h"
#include "Kernels/Filter_FFT_Kernel.h"
#include "Kernels/DedispersionForwardFFTkernel.h"
#include "Kernels/DedispersionBackwardFFTkernel.h"

#include "Pipeline.h"
#include "Pipelines/CorrelatorPipeline.h"
#include "Pipelines/BeamFormerPipeline.h"
#include "Pipelines/UHEP_Pipeline.h"

#include "WorkQueues/WorkQueue.h"
#include "WorkQueues/CorrelatorWorkQueue.h"
#include "WorkQueues/BeamFormerWorkQueue.h"
#include "WorkQueues/UHEP_WorkQueue.h"

using namespace LOFAR;
using namespace LOFAR::RTCP;

// Use our own terminate handler
Exception::TerminateHandler t(OpenCL_Support::terminate);

void usage(char **argv)
{
    std::cerr << "usage: " << argv[0] << " parset" <<  " [correlator|beam|UHEP]" << std::endl;
}

enum SELECTPIPELINE { correlator, beam, UHEP,unittest};

// Coverts the input argument from string to a valid 'function' name
SELECTPIPELINE to_select_pipeline(char *argument)
{
    if (!strcmp(argument,"correlator")) 
        return correlator;

    if (!strcmp(argument,"beam")) 
        return beam;

    if (!strcmp(argument,"UHEP")) 
        return UHEP;

    std::cout << "incorrect third argument supplied." << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    //Allow usage of nested omp calls
    omp_set_nested(true);

    using namespace LOFAR::RTCP;

    INIT_LOGGER("RTCP");
    std::cout << "running ..." << std::endl;

    // Set parts of the environment
    if (setenv("DISPLAY", ":0", 1) < 0)
    {
        perror("error setting DISPLAY");
        exit(1);
    }

#if 0 && defined __linux__
    set_affinity(0); //something with processor affinity, define at start of rtcp
#endif


    // Display usage on incorrect number parameters
    if (argc < 2)
    {
        usage(argv);
        exit(1);
    }

        // Parse the type of computation to perform
        // TODO: place holder enum for types of pipelines and unittest: Should we use propper argument parsing?
        SELECTPIPELINE option;
        if (argc == 3)
            option = to_select_pipeline(argv[2]);
        else
            option = correlator;

        // Create a parameters set object based on the inputs
        Parset ps(argv[1]);

        // Set the number of stations: Code is currently non functional
        //bool set_num_stations = false;
        //if (set_num_stations)
        //{
        //    const char *str = getenv("NR_STATIONS");
        //    ps.nrStations() = str ? atoi(str) : 77;
        //}
        std::cout << "nr stations = " << ps.nrStations() << std::endl;        

        // Select number of GPUs to run on



        // use a switch to select between modes
        switch (option)
        {
        case correlator:
            std::cout << "We are in the correlator part of the code." << std::endl;
            profiling = false; 
            CorrelatorPipeline(ps).doWork();

            profiling = true; 
            CorrelatorPipeline(ps).doWork();
            break;

        case beam:
            std::cout << "We are in the beam part of the code." << std::endl;
            profiling = false; 
            BeamFormerPipeline(ps).doWork();

            profiling = true; 
            BeamFormerPipeline(ps).doWork();
            break;

        case UHEP:
            std::cout << "We are in the UHEP part of the code." << std::endl;
            profiling = false;
            UHEP_Pipeline(ps).doWork();
            profiling = true;
            UHEP_Pipeline(ps).doWork();
            break;

        default:
            std::cout << "None of the types matched, do nothing" << std::endl;
        }

    return 0;
}

