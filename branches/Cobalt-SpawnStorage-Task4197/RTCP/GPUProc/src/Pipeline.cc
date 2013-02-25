#include "lofar_config.h"    

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "Interface/Stream.h"
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
            outputs(ps.nrSubbands())
        {
            createContext(context, devices);

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

            for (unsigned sb = 0; sb < ps.nrSubbands(); sb ++) {
                struct Output &output = outputs[sb];

                try {
                    if (ps.getHostName(CORRELATED_DATA, sb) == "") {
                      // an empty host name means 'write to disk directly', to
                      // make debugging easier for now
                      output.stream = new FileStream(ps.getFileName(CORRELATED_DATA, sb), 0666);
                    } else {
                      // connect to the Storage_main process for this output
                      const std::string desc = getStreamDescriptorBetweenIONandStorage(ps, CORRELATED_DATA, sb);

                      // TODO: Create these connections asynchronously!
                      output.stream = createStream(desc, false, 0);
                    }
                } catch(Exception &ex) {
                  LOG_ERROR_STR("Caught exception, using null stream for subband " << sb << ": " << ex);

                  output.stream = new NullStream;
                }
            }
        }


        cl::Program Pipeline::createProgram(const char *sources)
        {
            return LOFAR::RTCP::createProgram(ps, context, devices, sources);
        }


        void Pipeline::sendOutput(unsigned block, unsigned subband, StreamableData &data)
        {
            struct Output &output = outputs[subband];

            // Force blocks to be written in-order
            output.sync.waitFor(block);

            try {
              // We do the ordering, so we set the sequence numbers
              data.setSequenceNumber(block);

              // Try to write the data
              data.write(output.stream.get(), true);
            } catch (Stream::EndOfStreamException &ex) {
              LOG_WARN_STR("Caught EndOfStream while writing data: " << ex);

              // Prevent future warnings
              output.stream = new NullStream;
            }

            // Allow the next block to be written
            output.sync.advanceTo(block + 1);
        }


        void Pipeline::sendNextBlock(unsigned station)
        {
            (void)station;

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
        }

    }
}
