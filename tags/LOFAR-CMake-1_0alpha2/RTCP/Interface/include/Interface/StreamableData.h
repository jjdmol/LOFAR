#ifndef LOFAR_INTERFACE_STREAMABLE_DATA_H
#define LOFAR_INTERFACE_STREAMABLE_DATA_H

#include <Stream/Stream.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <Interface/Parset.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/Allocator.h>
#include <Interface/Align.h>
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
    StreamableData(bool isIntegratable): integratable(isIntegratable) {}

    // suppress warning by defining a virtual destructor
    virtual ~StreamableData() {}

    virtual size_t requiredSize() const = 0;
    virtual void allocate(Allocator &allocator = heapAllocator) = 0;

    virtual void read(Stream*, bool withSequenceNumber);
    virtual void write(Stream*, bool withSequenceNumber, unsigned align = 0);

    bool isIntegratable() const
    { return integratable; }

    virtual StreamableData &operator+= (const StreamableData &) 
    { LOG_WARN("Integration not implemented."); return *this; }

    uint32_t sequenceNumber;

  protected:
    const bool integratable;

    // a subclass should override these to marshall its data
    virtual void readData(Stream *) = 0;
    virtual void writeData(Stream *) = 0;
};

// A typical data set contains a MultiDimArray of tuples and a set of flags.
template <typename T = fcomplex, unsigned DIM = 4> class SampleData : public StreamableData
{
  public:
    typedef typename MultiDimArray<T,DIM>::ExtentList ExtentList;

    SampleData(bool isIntegratable, const ExtentList &extents, unsigned nrFlags);

    virtual size_t requiredSize() const;
    virtual void allocate(Allocator &allocator = heapAllocator);

    MultiDimArray<T,DIM> samples;
    std::vector<SparseSet<unsigned> >  flags;

  protected:
    virtual void checkEndianness();

    virtual void readData(Stream*);
    virtual void writeData(Stream*);

  private:
    // copy the ExtentList instead of using a reference, as boost by default uses a global one (boost::extents)
    const ExtentList     extents;
    const unsigned       nrFlags;

    bool                 itsHaveWarnedLittleEndian;
};

inline void StreamableData::read(Stream *str, bool withSequenceNumber)
{
  if (withSequenceNumber) {
    str->read(&sequenceNumber, sizeof sequenceNumber);

#if !defined WORDS_BIGENDIAN
    dataConvert(LittleEndian, &sequenceNumber, 1);
#endif
  }

  readData(str);
}

inline void StreamableData::write(Stream *str, bool withSequenceNumber, unsigned align)
{
  if (withSequenceNumber) {
#if !defined WORDS_BIGENDIAN
    if (align > 1) {
      if (align < sizeof(uint32_t))
	THROW(AssertError, "Sizeof alignment < sizeof sequencenumber");

      void *sn_buf;
      uint32_t sn = sequenceNumber;

      posix_memalign(&sn_buf, align, align);

      try {
        dataConvert(LittleEndian, &sn, 1);
        memcpy(sn_buf, &sn, sizeof sn);

        str->write(sn_buf, align);
      } catch (...) {
        free(sn_buf);
        throw;
      }

      free(sn_buf);
    } else {
      uint32_t sn = sequenceNumber;

      dataConvert(LittleEndian, &sn, 1);
      str->write(&sn, sizeof sn);
    }
#else
    if (align > 1) {
      if (align < sizeof(uint32_t)) THROW(AssertError, "Sizeof alignment < sizeof sequencenumber");

      void *sn_buf;
      posix_memalign(&sn_buf, align, align);

      try {
        memcpy(sn_buf, &sequenceNumber, sizeof sequenceNumber);

        str->write(sn_buf, align);
      } catch(...) {
        free(sn_buf);
        throw;
      }

      free(sn_buf);
    } else {
      str->write(&sequenceNumber, sizeof sequenceNumber);
    }

#endif
  }
  writeData(str);
}

template <typename T, unsigned DIM> inline SampleData<T,DIM>::SampleData(bool isIntegratable, const ExtentList &extents, unsigned nrFlags):
  StreamableData(isIntegratable),
  flags(0),
  extents(extents),
  nrFlags(nrFlags),
  itsHaveWarnedLittleEndian(false)
{
}

template <typename T, unsigned DIM> inline size_t SampleData<T,DIM>::requiredSize() const
{
  return align(MultiDimArray<T,DIM>::nrElements(extents) * sizeof(T),32);
}

template <typename T, unsigned DIM> inline void SampleData<T,DIM>::allocate(Allocator &allocator)
{
  samples.resize(extents, 32, allocator);
  flags.resize( nrFlags );
}

template <typename T, unsigned DIM> inline void SampleData<T,DIM>::checkEndianness()
{
#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples.origin(), samples.num_elements());
#endif
}

template <typename T, unsigned DIM> inline void SampleData<T,DIM>::readData(Stream *str)
{
  str->read(samples.origin(), samples.num_elements() * sizeof (T));

  checkEndianness();
}

template <typename T, unsigned DIM> inline void SampleData<T,DIM>::writeData(Stream *str)
{
#if !defined WORDS_BIGENDIAN
  if (!itsHaveWarnedLittleEndian) {
    itsHaveWarnedLittleEndian = true;

     LOG_WARN("writing data in little endian.");
  }
  //THROW(AssertError, "not implemented: think about endianness");
#endif

  str->write(samples.origin(), samples.num_elements() * sizeof (T));
}

} // namespace RTCP
} // namespace LOFAR

#endif
