#include "lofar_config.h"
#include "Common/LofarLogger.h"
#include "Common/Exception.h"

#include <iostream>
#include <omp.h>

#include "Interface/Parset.h"
#include "OpenCL_Support.h"

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
//#include "UnitTests/CorrelateRectangleTest.h"
//#include "UnitTests/CorrelatorTest.h"
//#include "UnitTests/FFT_Test.h"

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

    // dunno what test
    //(FFT_Test)(ps);  unknown test
    //the actual unittests!

    return 0;
}
