#ifndef GPUPROC_CORRELATORWORKQUEUEINPUTITEM_H
#define GPUPROC_CORRELATORWORKQUEUEINPUTITEM_H

#include "CL/cl.hpp"
#include "OpenCL_Support.h"
#include "global_defines.h"

#include "BeamletBufferToComputeNode.h"
#include <SubbandMetaData.h>

namespace LOFAR
{
  namespace RTCP 
  {
    class WorkQueueInputItem
    {
    public:  // TODO: public to allow moving 
      // Reference to an boost mapped input buffer
      // Owned by WorkQueue
      MultiArraySharedBuffer<char, 4> &inputSamples;

      // Header as received from the input stream
      BeamletBufferToComputeNode<i16complex>::header &header;

      // Meta data as red from the input stream
      SubbandMetaData &metaData;

    public:
      WorkQueueInputItem(MultiArraySharedBuffer<char, 4> &samples,
                         BeamletBufferToComputeNode<i16complex>::header &head,
                         SubbandMetaData &meta_data);

    };
  }
}
#endif