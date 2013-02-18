#include "lofar_config.h"

#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#include <global_defines.h>

#include <omp.h>
#include <string.h>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <boost/multi_array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include "Align.h"
#include "ApplCommon/PosixTime.h"
#include "BandPass.h"
#include "Common/LofarLogger.h"
#include "Common/SystemUtil.h"
#include "Stream/SharedMemoryStream.h"
#include "FilterBank.h"
#include "BeamletBufferToComputeNode.h"
#include "InputSection.h"
#include "Interface/Parset.h"
#include "Interface/SmartPtr.h"
#include "OpenCL_FFT/clFFT.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include "SlidingPointer.h"
#include "Stream/Stream.h"
#include "Stream/NullStream.h"
#include "UHEP/InvertedStationPPFWeights.h"
//#include "clAmdFft/include/clAmdFft.h"

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

#if defined __linux__
#include <sched.h>
#include <sys/time.h>
#endif

namespace LOFAR {
    namespace RTCP {


#undef USE_CUSTOM_FFT
#undef USE_TEST_DATA
#undef USE_B7015

#if defined __linux__

        inline void set_affinity(unsigned device)
        {
#if 0
            static const char mapping[1][12] = {
                0,  1,  2,  3,  8,  9, 10, 11,
            };
#else
            static const char mapping[8][12] = {
                { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
                { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
                { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
                { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
                { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
                { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
                { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
                { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
            };
#endif

            cpu_set_t set;

            CPU_ZERO(&set);

            for (unsigned coreIndex = 0; coreIndex < 12; coreIndex ++)
                CPU_SET(mapping[device][coreIndex], &set);

            if (sched_setaffinity(0, sizeof set, &set) < 0)
                perror("sched_setaffinity");
        }

#endif


#if defined USE_TEST_DATA

        void CorrelatorWorkQueue::setTestPattern()
        {
            if (ps.nrStations() >= 3) {
                double centerFrequency = 384 * ps.nrSamplesPerSubband();
                double baseFrequency = centerFrequency - .5 * ps.nrSamplesPerSubband();
                unsigned testSignalChannel = ps.nrChannelsPerSubband() >= 231 ? 230 : ps.nrChannelsPerSubband() / 2;
                double signalFrequency = baseFrequency + testSignalChannel * ps.nrSamplesPerSubband() / ps.nrChannelsPerSubband();

                for (unsigned time = 0; time < (NR_TAPS - 1 + ps.nrSamplesPerChannel()) * ps.nrChannelsPerSubband(); time ++) {
                    double phi = 2.0 * M_PI * signalFrequency * time / ps.nrSamplesPerSubband();

                    switch (ps.nrBytesPerComplexSample()) {
                    case 4 : reinterpret_cast<std::complex<short> &>(inputSamples[2][time][1][0]) = std::complex<short>((short) rint(32767 * cos(phi)), (short) rint(32767 * sin(phi)));
                        break;

                    case 2 : reinterpret_cast<std::complex<signed char> &>(inputSamples[2][time][1][0]) = std::complex<signed char>((signed char) rint(127 * cos(phi)), (signed char) rint(127 * sin(phi)));
                        break;
                    }
                }
            }
        }


        void CorrelatorWorkQueue::printTestOutput()
        {
            if (ps.nrBaselines() >= 6)
#pragma omp critical (cout)
            {
                std::cout << "newgraph newcurve linetype solid pts" << std::endl;

                //for (int channel = 0; channel < ps.nrChannelsPerSubband(); channel ++)
                if (ps.nrChannelsPerSubband() == 256)
                    for (int channel = 228; channel <= 232; channel ++)
                        std::cout << channel << ' ' << visibilities[5][channel][1][1] << std::endl;
            }
        }

#endif




    } // namespace RTCP
} // namespace LOFAR


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

    // Run the actual code in a try block
    try 
    {       
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
    } 
    catch (cl::Error &error)
    {
#pragma omp critical (cerr)
        std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl;
        exit(1);
    }

    return 0;
}

