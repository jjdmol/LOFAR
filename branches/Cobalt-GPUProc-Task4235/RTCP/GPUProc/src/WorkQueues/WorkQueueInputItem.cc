#include "lofar_config.h"    

#include "WorkQueueItem.h"

#include "CL/cl.hpp"
#include "OpenCL_Support.h"
#include "global_defines.h"

#include "WorkQueueInputItem.h"

namespace LOFAR
{
  namespace RTCP 
  {

    WorkQueueInputItem::WorkQueueInputItem(
          MultiArraySharedBuffer<char, 4> &samples,
          BeamletBufferToComputeNode<i16complex>::header &head,
          SubbandMetaData &meta_data)
    :
      inputSamples(samples),
        header(head),
        metaData(meta_data)
    {}

  }
}
