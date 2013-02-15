#ifndef GPUPROC_PIPELINE_H
#define GPUPROC_PIPELINE_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"
#include "InputSection.h"
#include "BeamletBufferToComputeNode.h"
#include "createProgram.h"

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
            std::vector<SmartPtr<Stream> >  GPUtoStorageStreams; // indexed by subband
            SlidingPointer<uint64_t> inputSynchronization, outputSynchronization;

#if defined USE_B7015
            OMP_Lock hostToDeviceLock[4], deviceToHostLock[4];
#endif

            //private:
            void                    sendNextBlock(unsigned station);
        };
    }
}
#endif
