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

#ifndef LOFAR_BBS3_MMAPMSINFO_H
#define LOFAR_BBS3_MMAPMSINFO_H

// \file
// Info about mapped MS.

//# Includes
#include <Common/LofarTypes.h>

namespace LOFAR
{

// \ addtogroup BBS3
// @{
// 


class MMapMSInfo
{
public:
  // Create the object for given nr of correlations in MS
  // and given size of one time slot (is #corr*#chan*#bl).
  explicit MMapMSInfo (int nrCorr=0, int nrChan=0, int nrBL=0,
		       bool reverseChan=false);

  // Set pointer to the mapped data chunk that is to be used next.
  // <group>
  void setInData (fcomplex* data)
    { itsInData = data; }
  void setOutData (fcomplex* data)
    { itsOutData = data; }
  // </group>

  // Set the time step in the part mapped in.
  void setTimeStep (int timeStep)
    { itsTimeStep = timeStep; }

  // Get the info.
  // <group>
  int nrCorr() const
    { return itsNrCorr; }
  int nrChan() const
    { return itsNrChan; }
  int nrBL() const
    { return itsNrBL; }
  int timeSize() const
    { return itsTimeSize; }
  int reverseChan() const
    { return itsReverseChan; }
  fcomplex* inData() const
    { return itsInData; }
  fcomplex* outData() const
    { return itsOutData; }
  int timeStep() const
    { return itsTimeStep; }
  int64 timeOffset() const
    { return int64(itsTimeStep) * itsTimeSize; }
  // </group>

private:
  int       itsNrCorr;
  int       itsNrChan;
  int       itsNrBL;
  int       itsTimeSize;
  bool      itsReverseChan;
  fcomplex* itsInData;
  fcomplex* itsOutData;
  int       itsTimeStep;
};

// @}

} // namespace LOFAR

#endif
