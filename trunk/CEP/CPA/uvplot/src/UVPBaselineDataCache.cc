// Copyright notice

#include <uvplot/UVPBaselineDataCache.h>


//=========>>>  UVPBaselineDataCache::UVPBaselineDataCache  <<<=========

UVPBaselineDataCache::UVPBaselineDataCache(unsigned int size)
  : itsCorrelationDataCache(size)
{
}





//==================>>>  UVPBaselineDataCache::size  <<<==================

unsigned int UVPBaselineDataCache::size() const
{
  return itsCorrelationDataCache.size();
}





//==================>>>  UVPBaselineDataCache::resize  <<<==================

unsigned int UVPCorrelationDataCache::resize(unsigned int newSize) const
{
  return itsCorrelationDataCache.resize(newSize);
}







//===============>>>  UVPBaselineDataCache::addSpectrum  <<<===============

bool UVPBaselineDataCache::addSpectrum(const UVPDataAtomHeader &header,
                                          const UVPDataAtom       &data)
{
  bool ReturnValue = true;

  if(header.itsCorrelationIndex >= itsCorrelationDataCache.size()) {
    ReturnValue = false; // No space... //    resize(header.itsTimeslot+1); // Make space for new data
  } else { 
    ReturnValue = itsCorrelationDataCache[header.itsCorrelationIndex].addSpectrum(header, data);
    setDirty(true);
  }
  
  return ReturnValue;
}







//===============>>>  UVPBaselineDataCache::getSpectrum  <<<===============

const UVPDataAtom *UVPBaselineDataCache::getSpectrum(const UVPDataAtomHeader &header) const
{
  const UVPDataAtom *Spectrum = 0;

  if(header.itsCorrelationIndex < itsCorrelationDataCache.size()) {
    Spectrum = itsCorrelationDataCache[header.itsCorrelationIndex].getSpectrum(header);
  }

  return Spectrum;
}






//==========>>>  UVPBaselineDataCache::getLocationOfSpectrum  <<<==========

UVPDataCacheElement *UVPBaselineDataCache::getLocationOfSpectrum(const UVPDataAtomHeader &header)
{
  UVPDataCacheElement *Location = 0;
  
  if(header.itsCorrelationIndex < itsCorrelationDataCache.size()) {
    Location = itsCorrelationDataCache[header.itsCorrelationIndex].getLocationOfSpectrum(header);
  }
  return Location;
}

