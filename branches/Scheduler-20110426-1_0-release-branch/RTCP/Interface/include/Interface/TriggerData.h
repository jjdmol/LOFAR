#ifndef LOFAR_INTERFACE_TRIGGER_DATA_H
#define LOFAR_INTERFACE_TRIGGER_DATA_H

#include <Common/lofar_complex.h>
#include <Stream/Stream.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/StreamableData.h>

namespace LOFAR {
namespace RTCP {

class TriggerData: public StreamableData
{
  public:
    TriggerData(): StreamableData(false), trigger(false) {}

    virtual TriggerData *clone() const { return new TriggerData(*this); }

    bool trigger;

  protected:  
    virtual size_t requiredSize() const { return 0; }
    virtual void allocate(Allocator&) {}
    virtual void readData(Stream *str) { str->read( &trigger, sizeof trigger ); }
    virtual void writeData(Stream *str) { str->write( &trigger, sizeof trigger ); }
};


} // namespace RTCP
} // namespace LOFAR

#endif
