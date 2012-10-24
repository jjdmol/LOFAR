#ifndef LOFAR_ROVISIBILITYITERATORPROXY_H
#define LOFAR_ROVISIBILITYITERATORPROXY_H

#include <msvis/MSVis/VisibilityIterator.h>
#include <tables/Tables/TableProxy.h>

namespace LOFAR {
  class ROVisibilityIteratorProxy {
  public :
    ROVisibilityIteratorProxy() : itsROVisibilityIterator(0) {}
    ROVisibilityIteratorProxy(casa::ROVisibilityIterator &vi) : itsROVisibilityIterator(&vi) {}
    ROVisibilityIteratorProxy(const ROVisibilityIteratorProxy &other) : itsROVisibilityIterator(other.itsROVisibilityIterator) {}
    casa::TableProxy ms() const {return TableProxy(itsROVisibilityIterator->ms());} 
    casa::ROVisibilityIterator* getROVisibilityIterator() const {return itsROVisibilityIterator;}
    Int next();
    ROVisibilityIteratorProxy* iter();
  protected:
    casa::ROVisibilityIterator *itsROVisibilityIterator;
  private:
    Int counter;
  };
}
#endif
 