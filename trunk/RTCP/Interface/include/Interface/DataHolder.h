#ifndef LOFAR_INTERFACE_DATAHOLDER_H
#define LOFAR_INTERFACE_DATAHOLDER_H

#include <Interface/StreamableData.h>
#include <Interface/Parset.h>
#include <Interface/Allocator.h>
#include <Interface/CN_Configuration.h>
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {

std::vector<StreamableData*> *newDataHolders( const Parset &ps, Allocator &allocator = heapAllocator );

} // namespace RTCP
} // namespace LOFAR

#endif

