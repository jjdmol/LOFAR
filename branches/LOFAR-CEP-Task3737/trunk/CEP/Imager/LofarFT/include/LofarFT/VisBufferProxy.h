#ifndef LOFAR_VISBUFFERPROXY_H
#define LOFAR_VISBUFFERPROXY_H

#include <casa/Containers/ValueHolder.h>
#include <casa/Arrays/Vector.h>
#include <msvis/MSVis/VisBuffer.h>
#include <LofarFT/ROVisibilityIteratorProxy.h>

namespace LOFAR {
  class VisBufferProxy {
  public :
    VisBufferProxy() : itsVisBuffer(), itsConstVisBuffer() {}
    VisBufferProxy(const casa::VisBuffer &vb) : itsVisBuffer(0), itsConstVisBuffer(&vb) {}
    VisBufferProxy(casa::VisBuffer &vb) : itsVisBuffer(&vb), itsConstVisBuffer(&vb) {}
    VisBufferProxy(ROVisibilityIteratorProxy &vi) : itsVisBuffer(new casa::VisBuffer(*vi.getROVisibilityIterator())), itsConstVisBuffer(itsVisBuffer), itsVisBufferPtr(itsVisBuffer) {}
//       casa::VisBuffer *vb = new VisBuffer(vi);
//       itsVisBufferPtr = CountedPtr<
//     }
    casa::Int get_nRow() const {return itsConstVisBuffer->nRow();}
    casa::Int get_nChannel() const  {return itsConstVisBuffer->nChannel();}
    casa::Bool get_newMS() const  {return itsConstVisBuffer->newMS();}
    casa::Int get_spectralWindow() const  {return itsVisBuffer->spectralWindow();}
    casa::ValueHolder get_imagingWeight() const  {return casa::ValueHolder(itsConstVisBuffer->imagingWeight());}
    casa::ValueHolder get_uvw() const ;
    casa::ValueHolder get_antenna1() const  {return casa::ValueHolder(itsConstVisBuffer->antenna1());}
    casa::ValueHolder get_antenna2() const {return casa::ValueHolder(itsConstVisBuffer->antenna2());}
    casa::ValueHolder get_flagRow() const {return casa::ValueHolder(itsConstVisBuffer->flagRow());}
    casa::ValueHolder get_timeCentroid() const {return casa::ValueHolder(itsConstVisBuffer->timeCentroid());}
    casa::ValueHolder get_visCube() const {return casa::ValueHolder(itsConstVisBuffer->visCube());}
    casa::ValueHolder get_correctedVisCube() const {return casa::ValueHolder(itsConstVisBuffer->correctedVisCube());}
    casa::ValueHolder get_modelVisCube() const {return casa::ValueHolder(itsConstVisBuffer->modelVisCube());}
    casa::Int get_polFrame() const {return itsConstVisBuffer->polFrame();}
    casa::VisBuffer* getVisBuffer() const {return itsVisBuffer;}
    const casa::VisBuffer* getConstVisBuffer() const {return itsConstVisBuffer;}
//     Bool isConstant;
  protected:
    casa::VisBuffer *itsVisBuffer;
    const casa::VisBuffer *itsConstVisBuffer;
    casa::CountedPtr<casa::VisBuffer> itsVisBufferPtr;
  };
}
#endif
