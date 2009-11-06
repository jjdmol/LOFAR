//# BlobHeader.tcc: Standard header for a blob
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>


#include <Blob/BlobHeader.h>
#include <Common/DataFormat.h>
#include <Common/LofarLogger.h>


LOFAR::BlobHeader::BlobHeader (int version, uint level)
: itsMagicValue     (bobMagicValue()),
  itsLength         (0),
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
