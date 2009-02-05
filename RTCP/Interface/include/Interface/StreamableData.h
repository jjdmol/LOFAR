#ifndef LOFAR_INTERFACE_STREAMABLE_DATA_H
#define LOFAR_INTERFACE_STREAMABLE_DATA_H

#include <Stream/Stream.h>
#include <Common/LofarTypes.h>
#include <Interface/Parset.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/Allocator.h>
#include <Common/DataConvert.h>

#include <boost/noncopyable.hpp>

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

    virtual void read(Stream*, bool withSequenceNumber);
    virtual void write(Stream*, bool withSequenceNumber);

    bool isIntegratable()
    { return integratable; }

    virtual StreamableData &operator+=( const StreamableData & ) 
    { std::clog << "WARNING: Integration not implemented" << std::endl; return *this; }

    uint32_t sequenceNumber;

  protected:
    bool integratable;

    // a subclass should override these to marshall its data
    virtual void readData(Stream*) = 0;
    virtual void writeData(Stream*) = 0;
};

// A typical data set contains a MultiDimArray of tuples and a set of flags.
template <typename T, unsigned DIM> class SampleData: public StreamableData, boost::noncopyable {
  public:
    typedef typename MultiDimArray<T,DIM>::ExtentList ExtentList;

    SampleData( bool isIntegratable, const ExtentList &extents, unsigned nrFlags, Allocator &allocator = heapAllocator );
    virtual ~SampleData();

    MultiDimArray<T,DIM> samples;
    SparseSet<unsigned>  *flags;

  protected:
    virtual void checkEndianness();

    virtual void readData(Stream*);
    virtual void writeData(Stream*);

  private:
    bool                 itsHaveWarnedLittleEndian;
};

inline void StreamableData::read( Stream *str, bool withSequenceNumber )
{
  if( withSequenceNumber ) {
    str->read( &sequenceNumber, sizeof sequenceNumber );

#if !defined WORDS_BIGENDIAN
    dataConvert( LittleEndian, &sequenceNumber, 1 );
#endif
  }

  readData( str );
}

inline void StreamableData::write( Stream *str, bool withSequenceNumber )
{
  if( withSequenceNumber ) {
#if !defined WORDS_BIGENDIAN
    uint32_t sn = sequenceNumber;

    dataConvert( LittleEndian, &sn, 1 );
    str->write( &sn, sizeof sn );
#else
    str->write( &sequenceNumber, sizeof sequenceNumber );
#endif
  }

  writeData( str );
}

template <typename T, unsigned DIM> inline SampleData<T,DIM>::SampleData( bool isIntegratable, const ExtentList &extents, unsigned nrFlags, Allocator &allocator ):
  StreamableData( isIntegratable ),
  samples( extents, 32, allocator ),
  flags( new SparseSet<unsigned>[nrFlags] ),
  itsHaveWarnedLittleEndian( false )
{
}

template <typename T, unsigned DIM> inline SampleData<T,DIM>::~SampleData()
{
  delete [] flags;
}


template <typename T, unsigned DIM> inline void SampleData<T,DIM>::checkEndianness()
{
#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples.origin(), samples.num_elements());
#endif
}

template <typename T, unsigned DIM> inline void SampleData<T,DIM>::readData( Stream *str )
{
  str->read(samples.origin(), samples.num_elements() * sizeof (T) );

  checkEndianness();
}

template <typename T, unsigned DIM> inline void SampleData<T,DIM>::writeData( Stream *str )
{
#if !defined WORDS_BIGENDIAN
  if( !itsHaveWarnedLittleEndian ) {
    itsHaveWarnedLittleEndian = true;

    std::clog << "Warning: writing data in little endian." << std::endl;
  }
  //THROW(AssertError, "not implemented: think about endianness");
#endif

  str->write(samples.origin(), samples.num_elements() * sizeof (T) );
}


} // namespace RTCP
} // namespace LOFAR

#endif
