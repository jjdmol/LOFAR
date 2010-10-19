//# ION_to_CN.h:
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

#ifndef LOFAR_CS1_INTERFACE_ION_TO_CN_H
#define LOFAR_CS1_INTERFACE_ION_TO_CN_H

#include <CS1_Interface/SparseSet.h>
#include <Transport/TransportHolder.h>

#include <cassert>


namespace LOFAR {
namespace CS1 {

class ION_to_CN
{
  public:
    SparseSet<unsigned> &flags();
    double		&delayAtBegin(), &delayAfterEnd();
    unsigned		&alignmentShift();

    void		read(TransportHolder *);
    void		write(TransportHolder *);

  private:
    SparseSet<unsigned>	itsFlags;

    struct MarshalledData
    {
      double		delayAtBegin, delayAfterEnd;
      unsigned		alignmentShift;
      unsigned char	flagsBuffer[132];
    } itsMarshalledData;
};

inline SparseSet<unsigned> &ION_to_CN::flags()
{
  return itsFlags;
}

inline double &ION_to_CN::delayAtBegin()
{
  return itsMarshalledData.delayAtBegin;
}

inline double &ION_to_CN::delayAfterEnd()
{
  return itsMarshalledData.delayAfterEnd;
}

inline unsigned &ION_to_CN::alignmentShift()
{
  return itsMarshalledData.alignmentShift;
}

inline void ION_to_CN::read(TransportHolder *th)
{
  th->recvBlocking(&itsMarshalledData, sizeof itsMarshalledData, 1, 0, 0);
  itsFlags.unmarshall(itsMarshalledData.flagsBuffer);
}

inline void ION_to_CN::write(TransportHolder *th)
{
  assert(itsFlags.marshall(&itsMarshalledData.flagsBuffer, sizeof itsMarshalledData.flagsBuffer) >= 0);
  th->sendBlocking(&itsMarshalledData, sizeof itsMarshalledData, 1, 0);
}



} // namespace CS1
} // namespace LOFAR

#endif 
