//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

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

