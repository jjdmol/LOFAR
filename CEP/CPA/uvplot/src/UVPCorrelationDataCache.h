// Copyright notice

#if !defined(UVPCORRELATIONDATACACHE_H)
#define UVPCORRELATIONDATACACHE_H

// $Id$

#include <UVPDataCacheElement.h>
#include <UVPDataAtom.h>


#include <vector>



//!
/*!
 */

class UVPCorrelationDateCache : public UVPDataCacheElement
{
 public:

                               UVPCorrelationDatacache(unsigned int size = 0);
  
  virtual unsigned int         size() const;
  virtual unsigned int         resize(unsigned int newSize);
  virtual bool                 addSpectrum(const UVPDataAtomHeader &header,
                                           const UVPDataAtom       &data);
  virtual const UVPDataAtom   *getSpectrum(const UVPDataAtomHeader &header) const;
  virtual UVPDataCacheElement *getLocationOfSpectrum(const UVPDataAtomHeader &header);

 protected:
 private:
  std::vector<UVPDataAtom> itsSpectrumDataCache;
};


#endif // UVPCORRELATIONDATACACHE_H
