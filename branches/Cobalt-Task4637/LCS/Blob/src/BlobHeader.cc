//# BlobHeader.tcc: Standard header for a blob
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>


#include <Blob/BlobHeader.h>
#include <Common/DataFormat.h>
#include <Common/LofarLogger.h>


LOFAR::BlobHeader::BlobHeader (int version, uint level)
: itsLength         (0),
  itsMagicValue     (bobMagicValue()),
  itsVersion        (version),
  itsDataFormat     (LOFAR::dataFormat()),
  itsLevel          (level),
  itsNameLength     (0)
{
  ASSERT (version > -128  &&  version < 128);
  ASSERT (level < 256);
}
    
void LOFAR::BlobHeader::setLocalDataFormat()
{
  itsLength     = LOFAR::dataConvert (getDataFormat(), itsLength);
  itsDataFormat = LOFAR::dataFormat();
}
