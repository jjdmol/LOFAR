#ifndef LOFAR_INTERFACE_PIPELINE_OUTPUT_H
#define LOFAR_INTERFACE_PIPELINE_OUTPUT_H

#include <string>
#include <iostream>
#include <boost/noncopyable.hpp>
#include <Stream/Stream.h>
#include <Common/Exceptions.h>
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

class PipelineOutput: boost::noncopyable
{
  public:
    enum OutputDataType {
      CORRELATEDDATA = 0,
      FILTEREDDATA,
      PENCILBEAMDATA,
      STOKESDATA,
      STOKESDATAINTEGRATEDCHANNELS,

      RAWDATA = -1
    };

    enum StorageWriterType {
      RAWWRITER = 0,
      NULLWRITER,
      CASAWRITER
    };

    PipelineOutput( const unsigned id, const OutputDataType type = RAWDATA );
    virtual ~PipelineOutput();

    OutputDataType      dataType() const            { return itsType; }
    StorageWriterType   writerType() const          { return itsWriterType; }
    const string        &filenameSuffix()           { return itsFilenameSuffix; }
    StreamableData      *data() const               { return itsData; }
    unsigned            IONintegrationSteps() const { return itsIONintegrationSteps; }

    StreamableData      *extractData();

    friend class PipelineOutputSet;

  protected:
    unsigned            itsId;

    OutputDataType	itsType;
    StorageWriterType   itsWriterType;
    StreamableData	*itsData;
    string              itsFilenameSuffix;
    unsigned            itsIONintegrationSteps;
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
  const CN_Mode mode = CN_Mode(ps);
  PipelineOutput *o;

  // add optional incoherentStokesI output
  if( ps.outputIncoherentStokesI() ) {
    o = new PipelineOutput( id++, PipelineOutput::STOKESDATA );
    o->itsData = new StokesData( false, 1, ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), ps.stokesIntegrationSteps() );
    o->itsFilenameSuffix = ".incoherentStokesI";
    itsOutputs.push_back( o );
  }

  // add main output
  switch( mode.mode() ) {
    case CN_Mode::FILTER:
	o = new PipelineOutput( id++, PipelineOutput::FILTEREDDATA );
        o->itsData = new FilteredData( ps.nrStations(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), ps.nrPencilBeams() );
        break;

    case CN_Mode::CORRELATE:
	o = new PipelineOutput( id++, PipelineOutput::CORRELATEDDATA );
        o->itsWriterType = PipelineOutput::CASAWRITER;
        o->itsData = new CorrelatedData( ps.nrBaselines(), ps.nrChannelsPerSubband() );
        break;

    case CN_Mode::COHERENT_COMPLEX_VOLTAGES:
	o = new PipelineOutput( id++, PipelineOutput::PENCILBEAMDATA );
        o->itsData = new PencilBeamData( ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps() );
        break;

    case CN_Mode::COHERENT_STOKES_I:
    case CN_Mode::COHERENT_ALLSTOKES:
    case CN_Mode::INCOHERENT_STOKES_I:
    case CN_Mode::INCOHERENT_ALLSTOKES:
        if( ps.stokesIntegrateChannels() ) {
          o = new PipelineOutput( id++, PipelineOutput::STOKESDATAINTEGRATEDCHANNELS );
          o->itsData = new StokesDataIntegratedChannels( mode.isCoherent(), mode.nrStokes(), ps.nrPencilBeams(), ps.CNintegrationSteps(), ps.stokesIntegrationSteps() );
	} else {
          o = new PipelineOutput( id++, PipelineOutput::STOKESDATA );
          o->itsData = new StokesData( mode.isCoherent(), mode.nrStokes(), ps.nrPencilBeams(), ps.nrChannelsPerSubband(), ps.CNintegrationSteps(), ps.stokesIntegrationSteps() );
        }
        break;

    default:
    	throw InterfaceException("Invalid pipeline mode. Cannot determine output data type.");
  }

  if( ps.IONintegrationSteps() > 1 && !o->itsData->isIntegratable() ) {
    LOG_WARN("Not integrating output because the output data type does not support integration.");
  } else {
    o->itsIONintegrationSteps = ps.IONintegrationSteps();
  }

  itsOutputs.push_back( o );

  for( unsigned i = 0; i < itsOutputs.size(); i++ ) {
    itsOutputs[i]->data()->allocate( allocator );
  }
}

} // namespace RTCP
} // namespace LOFAR

#endif

