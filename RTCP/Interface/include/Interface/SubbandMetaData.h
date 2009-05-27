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

#ifndef LOFAR_INTERFACE_SUBBAND_META_DATA_H
#define LOFAR_INTERFACE_SUBBAND_META_DATA_H

#include <Interface/SparseSet.h>
#include <Interface/MultiDimArray.h>
#include <Stream/Stream.h>
#include <Common/LofarLogger.h>

#include <cassert>


namespace LOFAR {
namespace RTCP {

// Note: struct must remain copyable to avoid ugly constructions when passing it around
struct SubbandMetaData
{
  public:
    SubbandMetaData();
    SubbandMetaData( const unsigned nrBeams );

    SparseSet<unsigned>	getFlags() const;
    void		setFlags(const SparseSet<unsigned> &);

    unsigned            alignmentShift() const;
    unsigned            &alignmentShift();

    void read( Stream *str );
    void write( Stream *str ) const;

    struct beamInfo {
      float delayAtBegin, delayAfterEnd;
      double beamDirectionAtBegin[3], beamDirectionAfterEnd[3];
    };

    // Note: CNProc/AsyncTranspose reads the information below directly
    std::vector<struct beamInfo> beams;

    struct marshallData {
      unsigned char	flagsBuffer[132];
      unsigned		alignmentShift;
    } itsMarshalledData;
};

inline SubbandMetaData::SubbandMetaData()
{
  // Used for constructing vectors, such as Vector<SubbandMetaData> metaData( itsNrStations, 32, allocator )
  // Without a default constructor, construction of such an array is possible, but quite awkward as it
  // requires the manual construction and destruction of each element:
  //
  // SubbandMetaData *metaData = allocator.allocate(itsNrStations*sizeof(SubbandMetaData),32)
  // for( unsigned i = 0; i < itsNrStations; i++ ) {
  //   metaData[i] = new SubbandMetaData( ... );
  // }
  // ...
  // for( unsigned i = 0; i < itsNrStations; i++ ) {
  //   delete metaData[i];
  // }
  // allocator.deallocate( metaData );
  //
  // NOTE: While C++ has a new[](...) construction especially for this use case, it is not part of ISO C++.
}

inline SubbandMetaData::SubbandMetaData( const unsigned nrBeams )
: 
  beams(nrBeams)
{
}

inline SparseSet<unsigned> SubbandMetaData::getFlags() const
{
  SparseSet<unsigned> flags;

  flags.unmarshall(itsMarshalledData.flagsBuffer);
  return flags;
}

inline void SubbandMetaData::setFlags(const SparseSet<unsigned> &flags)
{
  ssize_t size = flags.marshall(&itsMarshalledData.flagsBuffer, sizeof itsMarshalledData.flagsBuffer);
  
  assert(size >= 0);
}

inline unsigned SubbandMetaData::alignmentShift() const
{
  return itsMarshalledData.alignmentShift;
}

inline unsigned &SubbandMetaData::alignmentShift()
{
  return itsMarshalledData.alignmentShift;
}

inline void SubbandMetaData::read( Stream *str )
{
  // TODO: endianness

  str->read(&itsMarshalledData, sizeof itsMarshalledData);
  str->read(&beams.front(), beams.size() * sizeof beams[0] );
}

inline void SubbandMetaData::write( Stream *str ) const
{
  // TODO: endianness

  str->write(&itsMarshalledData, sizeof itsMarshalledData);
  str->write(&beams.front(), beams.size() * sizeof beams[0] );
}

} // namespace RTCP
} // namespace LOFAR

#endif 
