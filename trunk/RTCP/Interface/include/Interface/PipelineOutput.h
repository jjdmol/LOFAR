#ifndef LOFAR_INTERFACE_PIPELINE_OUTPUT_H
#define LOFAR_INTERFACE_PIPELINE_OUTPUT_H

#include <string>
#include <iostream>
#include <Stream/Stream.h>
#include <Interface/Parset.h>
#include <Interface/CN_Mode.h>
#include <Interface/Allocator.h>

#include <Interface/StreamableData.h>
#include <Interface/StokesData.h>
#include <Interface/FilteredData.h>
#include <Interface/CorrelatedData.h>
#include <Interface/PencilBeamData.h>

namespace LOFAR {
namespace RTCP {

// forward declaration
class PipelineOutputSet;

class PipelineOutput
{
  public:
    enum OutputDataType {
      CORRELATEDDATA = 0,
      FILTEREDDATA,
      PENCILBEAMDATA,
      STOKESDATA,

      RAWDATA = -1
    };

    enum StorageWriterType {
      RAWWRITER = 0,
      NULLWRITER,
      CASAWRITER
    };

    PipelineOutput( const unsigned id, const OutputDataType type = RAWDATA );
    virtual ~PipelineOutput();

    OutputDataType      dataType()   { return itsType; }
    StorageWriterType   writerType() { return itsWriterType; }
    const string        &filenameSuffix() { return itsFilenameSuffix; }
    StreamableData      *data()      { return itsData; }
    unsigned            IONintegrationSteps() { return itsIONintegrationSteps; }

    StreamableData      *extractData();

    friend class PipelineOutputSet;

  protected:
    unsigned            itsId;

    OutputDataType	itsType;
    StorageWriterType   itsWriterType;
    StreamableData	*itsData;
    string              itsFilenameSuffix;
    unsigned            itsIONintegrationSteps;

    PipelineOutput( const PipelineOutput & );
};

class PipelineOutputSet
{
  public:
    PipelineOutputSet( const Parset &ps, Allocator &allocator = heapAllocator );

    size_t                              size() const { return itsOutputs.size(); }
    PipelineOutput 			&operator[]( unsigned index ) { return *(itsOutputs[index]); }

  private:
    std::vector<PipelineOutput *>	itsOutputs;

    PipelineOutputSet( const PipelineOutputSet & );
};

inline PipelineOutput::PipelineOutput( const unsigned id, const PipelineOutput::OutputDataType type )
:
  itsId( id ),
  itsType( type ),
  itsWriterType( RAWWRITER ),
  itsData( 0 ),
  itsFilenameSuffix( "" ),
  itsIONintegrationSteps( 1 )
{
}

inline StreamableData *PipelineOutput::extractData()
{
  StreamableData *data = itsData;

  itsData = 0;
  return data;
}

inline PipelineOutput::~PipelineOutput()
{
  if( itsData ) {
    delete itsData;
    itsData = 0;
  }
}

inline PipelineOutputSet::PipelineOutputSet( const Parset &ps, Allocator &allocator )
{
  // This section tell IONProc and Storage which outputs are produced by the CNProcs,
  // and how the outputs should be treated.

  // !!! This section should match the actual pipeline as used by CNProcs !!!

  unsigned id = 0;
  CN_Mode mode = CN_Mode(ps);
  PipelineOutput *o;

  // add optional incoherentStokesI output
  if( ps.outputIncoherentStokesI() ) {
    o = new PipelineOutput( id++, PipelineOutput::STOKESDATA );
    o->itsData = new StokesData( false, 1, ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), ps.stokesIntegrationSteps(), allocator );
    o->itsFilenameSuffix = ".incoherentStokesI";
    itsOutputs.push_back( o );
  }

  // add main output
  switch( mode.mode() ) {
    case CN_Mode::FILTER:
	o = new PipelineOutput( id++, PipelineOutput::FILTEREDDATA );
        o->itsData = new FilteredData( ps.nrStations(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), allocator );
        break;

    case CN_Mode::CORRELATE:
	o = new PipelineOutput( id++, PipelineOutput::CORRELATEDDATA );
        o->itsWriterType = PipelineOutput::CASAWRITER;
        o->itsData = new CorrelatedData( ps.nrBaselines(), ps.nrChannelsPerSubband(), allocator );
        break;

    case CN_Mode::COHERENT_COMPLEX_VOLTAGES:
	o = new PipelineOutput( id++, PipelineOutput::PENCILBEAMDATA );
        o->itsData = new PencilBeamData( ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), allocator );
        break;

    case CN_Mode::COHERENT_STOKES_I:
    case CN_Mode::COHERENT_ALLSTOKES:
    case CN_Mode::INCOHERENT_STOKES_I:
    case CN_Mode::INCOHERENT_ALLSTOKES:
        o = new PipelineOutput( id++, PipelineOutput::STOKESDATA );
        o->itsData = new StokesData( mode.isCoherent(), mode.nrStokes(), ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), ps.stokesIntegrationSteps(), allocator );
        break;
  }
  o->itsIONintegrationSteps = ps.IONintegrationSteps();
  itsOutputs.push_back( o );
}

} // namespace RTCP
} // namespace LOFAR

#endif

