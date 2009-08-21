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
#include <Interface/Allocator.h>
#include <Stream/Stream.h>
#include <Common/LofarLogger.h>

#include <cassert>


namespace LOFAR {
namespace RTCP {

// Note: struct must remain copyable to avoid ugly constructions when passing it around
struct SubbandMetaData
{
  public:
    SubbandMetaData(unsigned nrSubbands, unsigned nrBeams, size_t alignment = 16, Allocator &allocator = heapAllocator);
    virtual ~SubbandMetaData();

    struct beamInfo {
      float  delayAtBegin, delayAfterEnd;
      double beamDirectionAtBegin[3], beamDirectionAfterEnd[3];
    };

    struct marshallData {
      unsigned char	flagsBuffer[132];
      unsigned		alignmentShift;

      // itsNrBeams elements will really be allocated, so this array needs to
      // be the last element. Also, ISO C++ forbids zero-sized arrays, so we use size 1.
      struct beamInfo   beams[1];
    };

    SparseSet<unsigned>	getFlags(unsigned subband) const;
    void		setFlags(unsigned subband, const SparseSet<unsigned> &);

    unsigned            alignmentShift(unsigned subband) const;
    unsigned            &alignmentShift(unsigned subband);

    struct beamInfo     *beams(unsigned subband) const;
    struct beamInfo     *beams(unsigned subband);

    struct marshallData &subbandInfo(unsigned subband) const;
    struct marshallData &subbandInfo(unsigned subband);
 
    void read(Stream *str);
    void write(Stream *str) const;

    // size of the information for one subband
    const unsigned      itsSubbandInfoSize;

 private:
    const unsigned      itsNrSubbands, itsNrBeams;

    // size of the information for all subbands
    const unsigned      itsMarshallDataSize;

    // the pointer to all our data, which consists of struct marshallData[itsNrSubbands],
    // except for the fact that the elements are spaces apart more than sizeof(struct marshallData)
    // to make room for extra beams which are not defined in the marshallData structure.
    //
    // Access elements through subbandInfo( subband ).
    char		*const itsMarshallData;

    // remember the pointer at which we allocated the memory for the marshallData
    Allocator           &itsAllocator;
};

inline SubbandMetaData::SubbandMetaData(unsigned nrSubbands, unsigned nrBeams, size_t alignment, Allocator &allocator)
: 
  // Size of the data we need to allocate. Note that marshallData already contains
  // the size of one beamInfo.
  itsSubbandInfoSize(sizeof(struct marshallData) + (nrBeams - 1) * sizeof(struct beamInfo)),

  itsNrSubbands(nrSubbands),
  itsNrBeams(nrBeams),

  itsMarshallDataSize(nrSubbands * itsSubbandInfoSize),

  itsMarshallData(static_cast<char*>(allocator.allocate(itsMarshallDataSize, alignment))),
  itsAllocator(allocator)
{
}

inline SubbandMetaData::~SubbandMetaData()
{
  itsAllocator.deallocate(itsMarshallData);
}

inline SparseSet<unsigned> SubbandMetaData::getFlags(unsigned subband) const
{
  SparseSet<unsigned> flags;

  flags.unmarshall(subbandInfo(subband).flagsBuffer);
  return flags;
}

inline void SubbandMetaData::setFlags(unsigned subband, const SparseSet<unsigned> &flags)
{
  ssize_t size = flags.marshall(&subbandInfo(subband).flagsBuffer, sizeof subbandInfo(subband).flagsBuffer);
  
  assert(size >= 0);
}

inline unsigned SubbandMetaData::alignmentShift(unsigned subband) const
{
  return subbandInfo(subband).alignmentShift;
}

inline unsigned &SubbandMetaData::alignmentShift(unsigned subband)
{
  return subbandInfo(subband).alignmentShift;
}

inline struct SubbandMetaData::beamInfo *SubbandMetaData::beams(unsigned subband) const
{
  return &subbandInfo(subband).beams[0];
}

inline struct SubbandMetaData::beamInfo *SubbandMetaData::beams(unsigned subband)
{
  return &subbandInfo(subband).beams[0];
}

inline struct SubbandMetaData::marshallData &SubbandMetaData::subbandInfo(unsigned subband) const
{
  // calculate the array stride ourself, since C++ does not know the proper size of the marshallData elements
  return *reinterpret_cast<struct marshallData*>(itsMarshallData + (subband * itsSubbandInfoSize));
}

inline struct SubbandMetaData::marshallData &SubbandMetaData::subbandInfo(unsigned subband)
{
  // calculate the array stride ourself, since C++ does not know the proper size of the marshallData elements
  return *reinterpret_cast<struct marshallData*>(itsMarshallData + (subband * itsSubbandInfoSize));
}

inline void SubbandMetaData::read(Stream *str)
{
  // TODO: endianness

  str->read(itsMarshallData, itsMarshallDataSize);
}

inline void SubbandMetaData::write(Stream *str) const
{
  // TODO: endianness

  str->write(itsMarshallData, itsMarshallDataSize);
}

} // namespace RTCP
} // namespace LOFAR

#endif 
