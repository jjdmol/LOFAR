//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Interface/DataHolder.h>
#include <Interface/CorrelatedData.h>
#include <Interface/FilteredData.h>

namespace LOFAR {
namespace RTCP {

StreamableData *newDataHolder( const string name, const Parset &ps, Allocator &allocator )
{
  if( name == "CorrelatedData" ) {
    return new CorrelatedData( ps.nrBaselines(), ps.nrChannelsPerSubband(), allocator );
  } else if( name == "FilteredData" ) {
    return new FilteredData( ps.nrStations(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), allocator );
  }

  return 0;
}

} // namespace RTCP
} // namespace LOFAR


