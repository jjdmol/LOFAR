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

#if !defined(UVPCORRELATIONDATACACHE_H)
#define UVPCORRELATIONDATACACHE_H

// $Id$

#include <uvplot/UVPDataCacheElement.h>
#include <uvplot/UVPDataAtom.h>


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
