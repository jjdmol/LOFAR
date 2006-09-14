//#  MMapMSInfo.h: Info about mapped MS
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_BBS_MMAPMSINFO_H
#define LOFAR_BBS_MMAPMSINFO_H

// \file
// Info about mapped MS.

//# Includes
#include <Common/LofarTypes.h>

namespace LOFAR
{
  // Forward Declarations.
  class MSDesc;

namespace BBS
{

// \ addtogroup BBS
// @{
// 


class MMapMSInfo
{
public:
  MMapMSInfo()
    {}

  // Create the object for the given MS description and dd (spw).
  MMapMSInfo (const MSDesc& msdesc, uint ddid, bool reverseChan);

  // Set pointer to the mapped data chunk that is to be used next.
  // It adds the offset to get the pointer to the wanted spw.
  // <group>
  void setInData (fcomplex* data)
    { itsInData = data; }
  void setOutData (fcomplex* data)
    { itsOutData = data; }
  // </group>

  // Set the time slot in the part mapped in.
  void setTimes (int timeSlot, int nrTimes)
    { itsTimeSlot = timeSlot; itsNrTimes = nrTimes; }

  // Get the info.
  // <group>
  int nrCorr() const
    { return itsNrCorr; }
  int nrChan() const
    { return itsNrChan; }
  int nrBL() const
    { return itsNrBL; }
  int nrTimes() const
    { return itsNrTimes; }
  int timeSize() const
    { return itsTimeSize; }
  int ddOffset() const
    { return itsDDOffset; }
  int reverseChan() const
    { return itsReverseChan; }
  fcomplex* inData() const
    { return itsInData; }
  fcomplex* outData() const
    { return itsOutData; }
  int timeSlot() const
    { return itsTimeSlot; }
  int64 timeOffset() const
    { return int64(itsTimeSlot) * itsTimeSize; }
  // </group>

private:
  int       itsNrCorr;
  int       itsNrChan;
  int       itsNrBL;
  int       itsTimeSize;
  bool      itsReverseChan;
  fcomplex* itsInData;
  fcomplex* itsOutData;
  int       itsTimeSlot;
  int       itsNrTimes;
  int       itsDDOffset;       //# Offset for the dd (spw) to process
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
