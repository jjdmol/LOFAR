#include <LofarFT/VisBufferProxy.h>

using namespace casa;

namespace LOFAR {
  ValueHolder VisBufferProxy::get_uvw() const {
    Array<Double> v(IPosition(2, 3, itsConstVisBuffer->nRow()));
    Vector<RigidVector<Double,3> > uvw(itsConstVisBuffer->uvw());
    for(int i=0; i<itsVisBuffer->nRow(); i++) {
      v(IPosition(2,0,i)) = uvw(i)(0);
      v(IPosition(2,1,i)) = uvw(i)(1);
      v(IPosition(2,2,i)) = uvw(i)(2);
    }
    return ValueHolder(v);
  }
}
