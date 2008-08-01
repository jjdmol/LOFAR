//# SubbandMetaData.h:
//#
//#  Copyright (C) 2007
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

#ifndef LOFAR_CS1_INTERFACE_SUBBAND_META_DATA_H
#define LOFAR_CS1_INTERFACE_SUBBAND_META_DATA_H

#include <CS1_Interface/SparseSet.h>
#include <Stream/Stream.h>

#include <cassert>


namespace LOFAR {
namespace CS1 {

struct SubbandMetaData
{
  public:
    SparseSet<unsigned>	getFlags() const;
    void		setFlags(const SparseSet<unsigned> &);

    float		delayAtBegin, delayAfterEnd;
    unsigned		alignmentShift;

  private:
    unsigned char	flagsBuffer[132];
};


inline SparseSet<unsigned> SubbandMetaData::getFlags() const
{
  SparseSet<unsigned> flags;

  flags.unmarshall(flagsBuffer);
  return flags;
}


inline void SubbandMetaData::setFlags(const SparseSet<unsigned> &flags)
{
  ssize_t size = flags.marshall(&flagsBuffer, sizeof flagsBuffer);
  
  assert(size >= 0);
}


} // namespace CS1
} // namespace LOFAR

#endif 
