#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenMP_Support.h"
#include <iostream>

#include "UHEP_Pipeline.h"

namespace LOFAR
{
    namespace RTCP 
    {
        UHEP_Pipeline::UHEP_Pipeline(const Parset &ps)
            :
        Pipeline(ps),
            beamFormerCounter("beamformer"),
            transposeCounter("transpose"),
            invFFTcounter("inv. FFT"),
            invFIRfilterCounter("inv. FIR"),
            triggerCounter("trigger"),
            beamFormerWeightsCounter("BF weights"),
            samplesCounter("samples")
        {
            double startTime = omp_get_wtime();

#pragma omp parallel sections
            {
#pragma omp section
                beamFormerProgram = createProgram("UHEP/BeamFormer.cl");
#pragma omp section
                transposeProgram = createProgram("UHEP/Transpose.cl");
#pragma omp section
                invFFTprogram = createProgram("UHEP/InvFFT.cl");
#pragma omp section
                invFIRfilterProgram = createProgram("UHEP/InvFIR.cl");
#pragma omp section
                triggerProgram = createProgram("UHEP/Trigger.cl");
            }

            std::cout << "compile time = " << omp_get_wtime() - startTime << std::endl;
        }

    }
}

