#include <LofarFT/ROVisibilityIteratorProxy.h>
#include <boost/python.hpp>

namespace LOFAR {

  ROVisibilityIteratorProxy* ROVisibilityIteratorProxy::iter() {
    counter = 0;
    itsROVisibilityIterator->originChunks();
    itsROVisibilityIterator->origin();
    return this;
  }
  
  Int ROVisibilityIteratorProxy::next() {
    if ((!itsROVisibilityIterator->moreChunks()) && (!itsROVisibilityIterator->more())) {
        PyErr_SetString(PyExc_StopIteration, "No more data.");
        boost::python::throw_error_already_set();
    }
    if (!counter){
      return counter++;
    }
    (*itsROVisibilityIterator)++;
    if (itsROVisibilityIterator->more()) {
      return counter++;
    }
    itsROVisibilityIterator->nextChunk();
    if (itsROVisibilityIterator->moreChunks()) {
      itsROVisibilityIterator->origin();
      if (itsROVisibilityIterator->more()) {
        return counter++;
      }
    }
    PyErr_SetString(PyExc_StopIteration, "No more data.");
    boost::python::throw_error_already_set();
  }
  
}
