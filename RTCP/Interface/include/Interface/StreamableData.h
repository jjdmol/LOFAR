#ifndef LOFAR_INTERFACE_STREAMABLE_DATA_H
#define LOFAR_INTERFACE_STREAMABLE_DATA_H

#include <Stream/Stream.h>
#include <Common/LofarTypes.h>
#include <Interface/Parset.h>
#include <Interface/Allocator.h>
#include <Common/DataConvert.h>

namespace LOFAR {
namespace RTCP {

// Data which needs to be transported between CN, ION and Storage.
// Apart from read() and write() functionality, the data is augmented
// with a sequence number in order to detect missing data. Furthermore,
// an integration operator += can be defined to reduce the data.

// Endianness:
//  * CN/ION are big endian (ppc)
//  * Storage is little endian (intel)
//  * Stations are little endian
//
// Endianness is swapped by:
//  * Data received by the CN from the stations (transported via the ION)
//  * Data received by Storage from the ION
//
// WARNING: We consider all data streams to be big endian

class StreamableData {
  public:
    // A stream is integratable if it supports the += operator to combine
    // several objects into one.
    StreamableData( bool isIntegratable ): integratable(isIntegratable) {}

    // suppress warning by defining a virtual destructor
    virtual ~StreamableData() {}

    void read(Stream*, bool withSequenceNumber);
    void write(Stream*, bool withSequenceNumber) const;

    bool isIntegratable()
    { return integratable; }

    virtual StreamableData &operator+=( const StreamableData & ) 
    { std::clog << "WARNING: Integration not implemented" << std::endl; return *this; }

    uint32_t sequenceNumber;

  protected:
    bool integratable;

    // a subclass should override these to marshall its data
    virtual void readData(Stream*) = 0;
    virtual void writeData(Stream*) const = 0;
};

inline void StreamableData::read( Stream *str, bool withSequenceNumber )
{
  readData( str );

  if( withSequenceNumber ) {
    str->read( &sequenceNumber, sizeof sequenceNumber );

#if !defined WORDS_BIGENDIAN
    dataConvert( LittleEndian, &sequenceNumber, 1 );
#endif
  }
}

inline void StreamableData::write( Stream *str, bool withSequenceNumber ) const
{
  writeData( str );

  if( withSequenceNumber ) {
#if !defined WORDS_BIGENDIAN
    uint32_t sn = sequenceNumber;

    dataConvert( LittleEndian, &sn, 1 );
    str->write( &sn, sizeof sn );
#else
    str->write( &sequenceNumber, sizeof sequenceNumber );
#endif
  }
}

} // namespace RTCP
} // namespace LOFAR

#endif
