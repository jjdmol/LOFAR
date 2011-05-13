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

#define magic 0xda7a

#include <cstring>

namespace LOFAR {
namespace RTCP {

// Data which needs to be transported between CN, ION and Storage.
// Apart from read() and write() functionality, the data is augmented
// with a sequence number in order to detect missing data. Furthermore,
// an integration operator += can be defined to reduce the data.

// Endianness:
// * Endianness is defined by the correlator. 
// * Both Data and sequence number will have endianness of the
//   correlator
//
// WARNING: We consider all data streams to have the endianness of the
// correlator. No conversion is done here.

class IntegratableData
{
  public:
    virtual ~IntegratableData() {}

    virtual IntegratableData &operator += (const IntegratableData &) = 0;
};

    
class StreamableData
{
  public:
    virtual ~StreamableData() {}

    void read(Stream *, bool withSequenceNumber);
    void write(Stream *, bool withSequenceNumber, unsigned align = 0);

    bool shouldByteSwap() const
    { return peerMagicNumber != magic; }

    uint32_t peerMagicNumber;    /// magic number received from peer
/*     uint32_t hostMagicNumber;    /// magic number in local endianness */
    uint32_t sequenceNumber;

    uint32_t byteSwappedSequenceNumber() const {
      if (shouldByteSwap()) {
        uint32_t seqno = sequenceNumber;

        byteSwap32(&seqno);

        return seqno;
      } else {
        return sequenceNumber;
      }
    }

  protected:
    // a subclass should override these to marshall its data
    virtual void readData(Stream *) = 0;
    virtual void writeData(Stream *) = 0;
};


// A typical data set contains a MultiDimArray of tuples and a set of flags.
template <typename T = fcomplex, unsigned DIM = 4> class SampleData : public StreamableData
{
  public:
    typedef typename MultiDimArray<T,DIM>::ExtentList ExtentList;

    SampleData(const ExtentList &extents, unsigned nrFlags, Allocator & = heapAllocator);

    MultiDimArray<T,DIM>	      samples;
    std::vector<SparseSet<unsigned> > flags;

  protected:
    virtual void checkEndianness();

    virtual void readData(Stream *);
    virtual void writeData(Stream *);

  private:
    //bool	 itsHaveWarnedLittleEndian;
};


inline void StreamableData::read(Stream *str, bool withSequenceNumber)
{
  if (withSequenceNumber) {

    str->read(&peerMagicNumber, sizeof(peerMagicNumber));
    str->read(&sequenceNumber, sizeof sequenceNumber);

#if 0 && !defined WORDS_BIGENDIAN
    dataConvert(LittleEndian, &sequenceNumber, 1);
#endif
  }

  readData(str);
}


inline void StreamableData::write(Stream *str, bool withSequenceNumber, unsigned alignment)
{
  
  if (withSequenceNumber) {
/*     std::vector<char> header(alignment > sizeof(uint32_t) ? alignment : sizeof(uint32_t)); */
    std::vector<char> header(alignment > 2*sizeof(uint32_t) ? alignment : 2*sizeof(uint32_t));
    uint32_t          &magicValue = * reinterpret_cast<uint32_t *>(&header[0]);
    uint32_t	      &seqNo      = * reinterpret_cast<uint32_t *>(&header[sizeof(uint32_t)]);

#if defined USE_VALGRIND
    memset(&header[0], 0, header.size());
#endif

    magicValue = magic;
    seqNo = sequenceNumber;

#if 0 && !defined WORDS_BIGENDIAN
    dataConvert(BigEndian, &seqNo, 1);
#endif

    str->write(&header[0], header.size());
  }
  
  writeData(str);
}


template <typename T, unsigned DIM> inline SampleData<T,DIM>::SampleData(const ExtentList &extents, unsigned nrFlags, Allocator &allocator)
:
  samples(extents, 32, allocator),
  flags(nrFlags)
  //itsHaveWarnedLittleEndian(false)
{
}


template <typename T, unsigned DIM> inline void SampleData<T,DIM>::checkEndianness()
{
#if 0 && !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples.origin(), samples.num_elements());
#endif
}


template <typename T, unsigned DIM> inline void SampleData<T,DIM>::readData(Stream *str)
{
  str->read(samples.origin(), samples.num_elements() * sizeof(T));

  checkEndianness();
}


template <typename T, unsigned DIM> inline void SampleData<T,DIM>::writeData(Stream *str)
{
#if 0 && !defined WORDS_BIGENDIAN
  if (!itsHaveWarnedLittleEndian) {
    itsHaveWarnedLittleEndian = true;

     LOG_WARN("writing data in little endian.");
  }
  //THROW(AssertError, "not implemented: think about endianness");
#endif

  str->write(samples.origin(), samples.num_elements() * sizeof(T));
}

} // namespace RTCP
} // namespace LOFAR

#endif
