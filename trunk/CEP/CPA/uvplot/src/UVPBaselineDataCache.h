// Copyright notice

#if !defined(UVPBASELINEDATACACHE_H)
#define UVPBASELINEDATACACHE_H

// $Id$

#include <UVPDataCacheElement.h>
#include <UVPCorrelationDataCache.h>
#include <UVPDataAtom.h>



#include <vector>




//!
/*!
 */

class UVPBaselineDateCache : public UVPDataCacheElement
{
 public:

                               UVPBaselineDatacache(unsigned int size = 0);
  
  virtual unsigned int         size() const;
  virtual unsigned int         resize(unsigned int newSize);
  virtual bool                 addSpectrum(const UVPDataAtomHeader &header,
                                           const UVPDataAtom       &data);
  virtual const UVPDataAtom   *getSpectrum(const UVPDataAtomHeader &header) const;
  virtual UVPDataCacheElement *getLocationOfSpectrum(const UVPDataAtomHeader &header);

 protected:
 private:
  std::vector<UVPCorrelationDataCache> itsCorrelationDataCache;
};


#endif // UVPBASELINEDATACACHE_H
