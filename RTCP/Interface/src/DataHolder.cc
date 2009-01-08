//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Interface/DataHolder.h>
#include <Interface/CorrelatedData.h>
#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/StokesData.h>

namespace LOFAR {
namespace RTCP {

StreamableData *newDataHolder( CN_Mode &mode, const Parset &ps, Allocator &allocator )
{
  switch( mode.outputDataType() ) {
    case CN_Mode::CORRELATEDDATA:
      return new CorrelatedData( ps.nrBaselines(), ps.nrChannelsPerSubband(), allocator );

    case CN_Mode::FILTEREDDATA:
      return new FilteredData( ps.nrStations(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), allocator );

    case CN_Mode::PENCILBEAMDATA:
      return new PencilBeamData( ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), allocator );

    case CN_Mode::STOKESDATA:
      return new StokesData( mode, ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), ps.stokesIntegrationSteps(), allocator );

    default:
      std::cerr << "newDataHolder: Cannot create data object for mode " << mode << std::endl;
      return 0;
  }
}

} // namespace RTCP
} // namespace LOFAR
