#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "Stream/Stream.h"
#include "Stream/FileStream.h"
#include "Stream/SharedMemoryStream.h"
#include "Stream/NullStream.h"

#include "Pipeline.h"

namespace LOFAR
{
    namespace RTCP 
    {      


                Pipeline::Pipeline(const Parset &ps)
            :
        ps(ps),
            stationInputs16(ps.nrStations()),
            stationInputs8(ps.nrStations()),
            stationInputs4(ps.nrStations()),
            bufferToGPUstreams(ps.nrStations()),
            GPUtoStorageStreams(ps.nrSubbands())
        {
            createContext(context, devices);

#ifdef USE_INPUT_SECTION
            for (unsigned stat = 0; stat < ps.nrStations(); stat ++) {
                bufferToGPUstreams[stat] = new SharedMemoryStream;

                switch (ps.nrBitsPerSample()) {
                default:
                case 16:
                    stationInputs16[stat].init(ps, stat);
                    break;

                case 8:
                    stationInputs8[stat].init(ps, stat);
                    break;

                case 4:
                    stationInputs4[stat].init(ps, stat);
                    break;
                }
            }
#else
            for (unsigned stat = 0; stat < ps.nrStations(); stat ++)
                bufferToGPUstreams[stat] = new NullStream;
#endif

            for (unsigned sb = 0; sb < ps.nrSubbands(); sb ++) 
                try {
                  GPUtoStorageStreams[sb] = new FileStream(ps.getFileName(CORRELATED_DATA, sb), 0666);
                } catch(InterfaceException &ex) {
                  LOG_ERROR_STR("Caught exception, using null stream for subband " << sb << ": " << ex);

                  GPUtoStorageStreams[sb] = new NullStream;
                }
        }


        cl::Program Pipeline::createProgram(const char *sources)
        {
            return LOFAR::RTCP::createProgram(ps, context, devices, sources);
        }


        void Pipeline::sendNextBlock(unsigned station)
        {
            (void)station;
#ifdef USE_INPUT_SECTION
            unsigned bitsPerSample = ps.nrBitsPerSample();

            Stream *stream = bufferToGPUstreams[station];

            switch(bitsPerSample) {
            default:
            case 16:
                stationInputs16[station].beamletBufferToComputeNode->process(stream);
                break;

            case 8:
                stationInputs8[station].beamletBufferToComputeNode->process(stream);
                break;

            case 4:
                stationInputs4[station].beamletBufferToComputeNode->process(stream);
                break;
            }
#endif
        }

    }
}
