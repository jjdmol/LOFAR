#include "lofar_config.h"

#include <iostream>
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include "Common/LofarLogger.h"
#include "Common/Exception.h"
#include "CoInterface/Parset.h"

#include "OpenCL_Support.h"

#include "UnitTest.h"
#include "UnitTests/IncoherentStokesTest.h"
#include "UnitTests/IntToFloatTest.h"
#include "UnitTests/BeamFormerTransposeTest.h"
#include "UnitTests/DedispersionChirpTest.h"
#include "UnitTests/CoherentStokesTest.h"
#include "UnitTests/UHEP_BeamFormerTest.h"
#include "UnitTests/UHEP_TransposeTest.h"
#include "UnitTests/BeamFormerTest.h"
#include "UnitTests/CorrelateTriangleTest.h"
#include "UnitTests/UHEP_TriggerTest.h"
#include "UnitTests/CorrelateRectangleTest.h"
#include "UnitTests/CorrelatorTest.h"
#include "UnitTests/FFT_Test.h"
//#include "UnitTests/AMD_FFT_Test.h"
#include "UnitTests/FIR_FilterTest.h"

//#include  <UnitTest++.h>  

using namespace LOFAR;
using namespace LOFAR::RTCP;

// Use our own terminate handler
Exception::TerminateHandler t(OpenCL_Support::terminate);

int main(int argc, char **argv)
{

    INIT_LOGGER("RTCP");
    std::cout << "running ..." << std::endl;

    if (argc < 2)
    {
        std::cerr << "usage: " << argv[0] << " parset" << std::endl;
        return 1;
    }

    Parset ps(argv[1]);

    // TODO: defines to vars + loop over val ranges.
    std::cout << "Obs ps: nSt=" << ps.nrStations() << " nPol=" << NR_POLARIZATIONS
              << " nSampPerCh=" << ps.nrSamplesPerChannel() << " nChPerSb="
              << ps.nrChannelsPerSubband() << " nTaps=" << ps.nrPPFTaps()
              << " nBitsPerSamp=" << ps.nrBitsPerSample() << std::endl;


    (FIR_FilterTest)(ps);
    (FFT_Test)(ps);
    //(AMD_FFT_Test)(ps);

    //(CorrelatorTest)(ps);       //needs parset AARTFAAC!!
    //(CorrelateRectangleTest)(ps); //needs parset AARTFAAC!!

    //works with all parsets
    //Correlate unittest 
    (CorrelateTriangleTest)(ps);

    ////UHEP unittest
    (UHEP_BeamFormerTest)(ps);
    (UHEP_TransposeTest)(ps);
    (UHEP_TriggerTest)(ps);

    //// beamformed unittest 
    (IncoherentStokesTest)(ps);
    (IntToFloatTest)(ps);
    (BeamFormerTest)(ps);
    (BeamFormerTransposeTest)(ps);
    (DedispersionChirpTest)(ps);
    (CoherentStokesTest)(ps);


    //the actual unittests!

    return 0;
    //return UnitTest::RunAllTests();
    
}
