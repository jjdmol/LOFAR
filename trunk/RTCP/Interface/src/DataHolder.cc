//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Interface/DataHolder.h>
#include <Interface/CorrelatedData.h>
#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/StokesData.h>
#include <Interface/CN_Mode.h>

namespace LOFAR {
namespace RTCP {

std::vector<StreamableData*> *newDataHolders( const Parset &ps, Allocator &allocator )
{
  CN_Mode mode = CN_Mode(ps);
  std::vector<StreamableData*> *outputs;

  outputs = new std::vector<StreamableData*>::vector();

  if( ps.outputIncoherentStokesI() ) {
    outputs->push_back( new StokesData( false, 1, ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), ps.stokesIntegrationSteps(), allocator ) );
  }

  switch( mode.finalOutputDataType() ) {
    case CN_Mode::CORRELATEDDATA:
      outputs->push_back( new CorrelatedData( ps.nrBaselines(), ps.nrChannelsPerSubband(), allocator ) );
      break;

    case CN_Mode::FILTEREDDATA:
      outputs->push_back( new FilteredData( ps.nrStations(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), allocator )  );
      break;

    case CN_Mode::PENCILBEAMDATA:
      outputs->push_back( new PencilBeamData( ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), allocator ) );
      break;

    case CN_Mode::STOKESDATA:
      outputs->push_back( new StokesData( mode.isCoherent(), mode.nrStokes(), ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), ps.stokesIntegrationSteps(), allocator ) );
      break;

    default:
      std::cerr << "newDataHolder: Cannot create data object for mode " << mode << std::endl;
      outputs->push_back( 0 );
      break;
  }

  return outputs;
}

} // namespace RTCP
} // namespace LOFAR
