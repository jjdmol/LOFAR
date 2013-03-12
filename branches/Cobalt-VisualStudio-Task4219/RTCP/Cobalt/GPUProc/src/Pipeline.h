#ifndef GPUPROC_PIPELINECLASS_H
#define GPUPROC_PIPELINECLASS_H
#include "CL/cl.hpp"

#include "Common/Thread/Queue.h"
#include "Common/Thread/Semaphore.h"
#include "CoInterface/Parset.h"
#include "CoInterface/StreamableData.h"
#include "OpenCL_Support.h"

#include "global_defines.h"
#include "Input/InputSection.h"
#include "Input/BeamletBufferToComputeNode.h"
#include "createProgram.h"
#include "BestEffortQueue.h"

namespace LOFAR
{
    namespace RTCP 
    {

        template <typename SAMPLE_TYPE> class StationInput
        {
        public:
            SmartPtr<InputSection<SAMPLE_TYPE> >               inputSection;
            SmartPtr<BeamletBufferToComputeNode<SAMPLE_TYPE> > beamletBufferToComputeNode;

            void init(const Parset &ps, unsigned psetNumber);
        };

        template<typename SAMPLE_TYPE> void StationInput<SAMPLE_TYPE>::init(const Parset &ps, unsigned psetNumber)
        {
            string stationName = ps.getStationNamesAndRSPboardNumbers(psetNumber)[0].station; // TODO: support more than one station
            std::vector<Parset::StationRSPpair> inputs = ps.getStationNamesAndRSPboardNumbers(psetNumber);

            inputSection = new InputSection<SAMPLE_TYPE>(ps, inputs);
            beamletBufferToComputeNode = new BeamletBufferToComputeNode<SAMPLE_TYPE>(ps, stationName, inputSection->itsBeamletBuffers, 0);
        }

        class Pipeline
        {
        public:
            Pipeline(const Parset &);

            cl::Program		    createProgram(const char *sources);

            const Parset	    &ps;
            cl::Context		    context;
            std::vector<cl::Device> devices;

            std::vector<StationInput<i16complex> > stationInputs16; // indexed by station
            std::vector<StationInput<i8complex> >  stationInputs8; // indexed by station
            std::vector<StationInput<i4complex> >  stationInputs4; // indexed by station

            std::vector<SmartPtr<Stream> >  bufferToGPUstreams; // indexed by station

            SlidingPointer<uint64_t> inputSynchronization;

#if defined USE_B7015
            OMP_Lock hostToDeviceLock[4], deviceToHostLock[4];
#endif

            void doWork();

            // Write an output block. Takes ownership of data, because the
            // block will be kept in the background until it can be written.
            void writeOutput(unsigned block, unsigned subband, StreamableData *data);

            // signal that we've written all blocks (because we can drop some)
            void noMoreOutput();

            //private:
            void                    sendNextBlock(unsigned station);

        private:
            struct Output {
              // synchronisation to write blocks in-order
              SlidingPointer<size_t> sync;

              // output data queue
              SmartPtr< BestEffortQueue< SmartPtr<StreamableData> > > bequeue;
            };

            std::vector<struct Output> outputs; // indexed by subband

            void handleOutput();
        };
    }
}
#endif
