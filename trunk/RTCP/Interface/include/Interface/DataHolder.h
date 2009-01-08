#ifndef LOFAR_INTERFACE_DATAHOLDER_H
#define LOFAR_INTERFACE_DATAHOLDER_H

#include <Interface/StreamableData.h>
#include <Interface/Parset.h>
#include <Interface/Allocator.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_Mode.h>
#include <string>

namespace LOFAR {
namespace RTCP {

StreamableData *newDataHolder( CN_Mode &mode, const Parset &ps, Allocator &allocator = heapAllocator );

} // namespace RTCP
} // namespace LOFAR

#endif

