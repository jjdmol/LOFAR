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
#include <Stream/Stream.h>

#include <cassert>


namespace LOFAR {
namespace CS1 {

class ION_to_CN
{
  public:
    SparseSet<unsigned> &flags(const unsigned beam);
    float		&delayAtBegin(const unsigned beam), &delayAfterEnd(const unsigned beam);
    unsigned		&alignmentShift(const unsigned beam);

    void		read(Stream *, const unsigned nrBeams);
    void		write(Stream *, const unsigned nrBeams);
    
    static const unsigned MAX_BEAMLETS    = 8;

  private:
    std::vector<SparseSet<unsigned> > itsFlags;

    struct MarshalledData
    {
      float		delayAtBegin, delayAfterEnd;
      unsigned		alignmentShift;
      unsigned char	flagsBuffer[132];
    };
    
    std::vector<struct MarshalledData> itsMarshalledData;
   public:
      ION_to_CN(){itsMarshalledData.resize(MAX_BEAMLETS);
                  itsFlags.resize(MAX_BEAMLETS);};
 };

inline SparseSet<unsigned> &ION_to_CN::flags(const unsigned beam)
{
  return itsFlags[beam];
}

inline float &ION_to_CN::delayAtBegin(const unsigned beam)
{
  return itsMarshalledData[beam].delayAtBegin;
}

inline float &ION_to_CN::delayAfterEnd(const unsigned beam)
{
  return itsMarshalledData[beam].delayAfterEnd;
}

inline unsigned &ION_to_CN::alignmentShift(const unsigned beam)
{
  return itsMarshalledData[beam].alignmentShift;
}

inline void ION_to_CN::read(Stream *str, const unsigned nrBeams)
{
  str->read(&itsMarshalledData[0], sizeof(struct MarshalledData) * nrBeams);

  for (unsigned beam = 0; beam < nrBeams; beam ++)
    itsFlags[beam].unmarshall(itsMarshalledData[beam].flagsBuffer);
}

inline void ION_to_CN::write(Stream *str, const unsigned nrBeams)
{
  for (unsigned beam = 0; beam < nrBeams; beam ++) {
    ssize_t size = itsFlags[beam].marshall(&itsMarshalledData[beam].flagsBuffer, sizeof itsMarshalledData[beam].flagsBuffer);
    
    assert(size >= 0);
  }  

  str->write(&itsMarshalledData[0], sizeof(struct MarshalledData) * nrBeams);
}

} // namespace CS1
} // namespace LOFAR

#endif 
