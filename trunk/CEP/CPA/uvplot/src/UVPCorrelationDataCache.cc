// Copyright notice

#include <UVPCorrelationDataCache.h>


//=========>>>  UVPCorrelationDataCache::UVPCorrelationDataCache  <<<=========

UVPCorrelationDataCache::UVPCorrelationDataCache(unsigned int size)
  : itsSpectrumDataCache(size)
{
}





//==================>>>  UVPCorrelationDataCache::size  <<<==================

unsigned int UVPCorrelationDataCache::size() const
{
  return itsSpectrumDataCache.size();
}





//==================>>>  UVPCorrelationDataCache::resize  <<<==================

unsigned int UVPCorrelationDataCache::resize(unsigned int newSize) const
{
  return itsSpectrumDataCache.resize(newSize);
}







//===============>>>  UVPCorrelationDataCache::addSpectrum  <<<===============

bool UVPCorrelationDataCache::addSpectrum(const UVPDataAtomHeader &header,
                                          const UVPDataAtom       &data)
{
  bool ReturnValue = true;

  if(header.itsTimeslot >= itsSpectrumDataCache.size()) {
    ReturnValue = false; // No space... //    resize(header.itsTimeslot+1); // Make space for new data
  } else { 
    itsSpectrumDataCache[header.itsTimeslot] = data;
    setDirty(true);
  }
  
  return ReturnValue;
}







//===============>>>  UVPCorrelationDataCache::getSpectrum  <<<===============

const UVPDataAtom *UVPCorrelationDataCache::getSpectrum(const UVPDataAtomHeader &header) const
{
  const UVPDataAtom *Spectrum = 0;

  if(header.itsTimeslot < itsSpectrumDataCache.size()) {
    Spectrum = &(itsSpectrumDataCache[header.itsTimeslot]);
  }

  return Spectrum;
}






//==========>>>  UVPCorrelationDataCache::getLocationOfSpectrum  <<<==========

UVPDataCacheElement *UVPCorrelationDataCache::getLocationOfSpectrum(const UVPDataAtomHeader &header)
{
  UVPDataCacheElement *Location = 0;
  
  if(header.itsTimeslot < itsSpectrumDataCache.size()) {
    Location = this;
  }
  return Location;
}

