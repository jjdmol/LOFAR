#include <lofar_config.h>

#include <Common/Thread/Mutex.h>
#include <Common/LofarLogger.h>
#include <Stream/SocketStream.h>
#include "TimeSync.h"

#include <RSP.h>
#include <Interface/RSPTimeStamp.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace LOFAR;
using namespace RTCP;

const unsigned nrInputs  = 10;
const unsigned nrOutputs = 10;

const float sampleClock = 8 * 1e6;
const float subbandWidth = sampleClock / 1024;
const size_t samplesPerBlock = subbandWidth * 0.05;

const size_t packetSize = 9000;

class Consumer;

/*
 * Lock-free buffer for RSP packets, including
 * producer primitives. Supports a single producer.
 */
class PacketBuffer {
public:
  struct Item {
    // the converted timestamp for this packet,
    // or 0 if this packet is being written.
    volatile int64 timestamp;

    // the timestamp of the previous packet,
    // or 0 for the first packet.
    volatile int64 prev_timestamp;

    // the payload
    struct RSP packet;

    Item(): timestamp(0), prev_timestamp(0) {}
  };

  class CircularPtr {
  public:
    CircularPtr( PacketBuffer &buffer, struct Item *item ): item(item), buffer(buffer) {}

    struct Item *item;

    void operator++() {
      if (++item == buffer.end)
        item = buffer.begin;
    }

    void operator--() {
      if (--item == buffer.begin)
        item = buffer.end - 1;
    }

  private:
    PacketBuffer &buffer;
  };

private:
  vector<struct Item> packets;
  struct Item * const begin;
  struct Item * const end;

  vector<Consumer *> consumers;
  Mutex consumersMutex;

  // points to where input will be written
  CircularPtr head;

  // points to the packet at or right after head
  CircularPtr next;

  // last recorded timestamp
  int64 prev_timestamp;

public:
  PacketBuffer( size_t bufsize, unsigned sampleClock, unsigned timesPerSlot, bool realtime );

  // timestamp of the last written packet (exclusive,
  // so actually points to one timestamp beyond the youngest
  // packet).
  //
  // (or 0 if no data has been written yet)
  volatile int64 youngest;

  // timestamp of the oldest slot in the buffer (or 0 if
  // the buffer has not been fully filled yet)
  volatile int64 oldest;
  struct Item * volatile oldest_item;

  // call before starting to write at head
  void startWrite();

  // pointer to the head packet
  struct RSP &writePtr();

  // call after completing a write at head
  void stopWrite();

  // call when data stream ends
  void noMoreData();

  // add/remove consumers
  void registerConsumer( Consumer &consumer );
  void unregisterConsumer( Consumer &consumer );

  // configuration parameters
  const unsigned sampleClock;
  const bool realtime;
  const size_t timesPerSlot;
};

/*
 * An abstract class describing a data processor.
 */
class PacketSink {
public:
  virtual void processPacket( Consumer *consumer, const int64 &timestamp, struct RSP &packet ) = 0;
  virtual void missingData( Consumer *consumer, const int64 &from, const int64 to ) = 0;
};

class Consumer {
public:
  Consumer( PacketBuffer &buffer, PacketSink &sink );

  void wait( int64 to, struct timespec &timeout );
  void read( int64 from, int64 to );

private:
  PacketBuffer &buffer;
  PacketSink &sink;
  const bool realtime;

  PacketBuffer::CircularPtr tail;

  // needed for synchronisation
  TimeSync needFrom, haveUntil;
  friend class PacketBuffer;
};

PacketBuffer::PacketBuffer( size_t bufsize, unsigned sampleClock, unsigned timesPerSlot, bool realtime )
:
  packets(bufsize),
  begin(&packets[0]),
  end(&packets[bufsize]),
  head(*this, begin),
  next(*this, begin),
  prev_timestamp(0),

  youngest(0),
  oldest(0),
  oldest_item(0),

  sampleClock(sampleClock),
  realtime(realtime),
  timesPerSlot(timesPerSlot)
{
  // head and next are distinct (once data is read)
  ASSERT( bufsize >= 2 );
}

void PacketBuffer::startWrite()
{
  if (!realtime) {
    // make sure that our consumers do not need the head.item
    // that we're about to overwrite

    const int64 headTime = head.item->timestamp;

    if (headTime != 0) {
      ScopedLock sl(consumersMutex);

      for (vector<Consumer *>::const_iterator i = consumers.begin(); i != consumers.end(); ++i)
        (*i)->needFrom.wait( headTime + timesPerSlot );
    }
  }

  // keep next pointed at the oldest packet
  ++ next;

  // if we overwrite data because we wrapped around,
  // we have to update oldest.
  if (next.item->timestamp != 0) {
    oldest_item = next.item;
    oldest = next.item->timestamp;
  }

  // invalidate head
  head.item->timestamp = 0;
}

struct RSP &PacketBuffer::writePtr()
{
  return head.item->packet;
}

void PacketBuffer::stopWrite()
{
  // complete our bookkeeping on head
  const struct RSP &packet = head.item->packet;
  const int64 timestamp = TimeStamp(packet.header.timestamp, packet.header.blockSequenceNumber, sampleClock);

  head.item->prev_timestamp = prev_timestamp;
  prev_timestamp = timestamp;

  // make head valid again
  head.item->timestamp = timestamp;

  ++ head;

  // the very first packet initialises oldest
  if (!oldest) {
    oldest_item = head.item;
    oldest = timestamp;
  }

  youngest = timestamp + timesPerSlot;

  {
    // readers could be waiting for this data both in realtime and in non-realtime modes

    // The consumersMutex only blocks if consumers are added or removed.
    ScopedLock sl(consumersMutex);

    // make sure that our consumers unlock once we've written data they need
    for (vector<Consumer *>::const_iterator i = consumers.begin(); i != consumers.end(); ++i) {
      // These TimeSync locks block only if the consumer is about to wait
      // for data. This is still cheaper than letting the consumers actively
      // poll `youngest'.
      (*i)->haveUntil.set( youngest );
    }
  }
}

void PacketBuffer::noMoreData()
{
  ScopedLock sl(consumersMutex);

  // make sure that our consumers unlock
  for (vector<Consumer *>::const_iterator i = consumers.begin(); i != consumers.end(); ++i)
    (*i)->haveUntil.noMoreData();
}

Consumer::Consumer( PacketBuffer &buffer, PacketSink &sink )
:
  buffer(buffer),
  sink(sink),
  realtime(buffer.realtime),
  tail(buffer, 0)
{
}

void Consumer::wait( int64 to, struct timespec &timeout )
{
  if (!realtime) {
    // sync will occur in read()
    return;
  }

  if (!haveUntil.wait( to, timeout )) {
    LOG_WARN_STR( "Data arrived too late for " << to );
  } else {
    LOG_DEBUG_STR( "Data arrived on time for " << to );
  }
}

void Consumer::read( int64 from, int64 to )
{
   /*
    * Read data from the circular buffer, lock free
    * under the following conditions:
    *   - running in real-time mode
    *   - no logging anywhere
    *
    * This means that any information we retrieve
    * from `buffer' can already be outdated at the next
    * memory access. So we often store a local copy. Also, we
    * let the buffer cache the following info, and update it
    * atomically:
    *
    * youngest:    timestamp of the latest packet that was written
    * oldest:      timestamp of the oldest packet still in the buffer
    * oldest_item: pointer to the oldest item
    *
    * Each item has the following properties:
    *
    * timestamp:   timestamp of the first sample, or 0 if this packet
    *              is either not used yet or being overwritten.
    * prev_timestamp:
    *              timestamp of the previous received packet, needed
    *              to detect some forms of packet loss.
    * item:        pointer to the payload.
    *
    * Note that even dereferencing multiple properties of the same
    * item might yield results from different packets, if the
    * writer passed by in between reads.
    *
    * In the cases that multiple properties are needed, make sure
    * that the code functions correctly if a second property comes
    * from a later packet than the first property.
    */
   ASSERT(from < to);

   /*
    * Sync with writer if needed
    */

   if (!realtime) {
     // reserve all data since from
     needFrom.set( from );

     // make sure all data (that exists)
     // until to is available.
     haveUntil.wait( to );
   }

   const int64 youngest = buffer.youngest;

   /*
    * We will exit once we encounter data up to 'to'
    * or later. So we have to make sure that exists,
    * as we cannot count on more data to arrive.
    */ 

   if (youngest <= from) {
     // no data available -- don't look for it
     LOG_DEBUG_STR( "Data loss because no data is available" );
     sink.missingData( this, from, to );
     return;
   }

   if (youngest < to) {
     // partial data available -- only look for what might exist
     LOG_DEBUG_STR( "Data loss because end is not available" );
     sink.missingData( this, youngest, to );

     to = youngest;
   }

   /*
    * Make an initial guess where to start looking.
    * We just have to make sure that we do not jump
    * between [from, to) as we'd be forced to consider
    * anything before `tail' as a loss.
    */ 

   if (!tail.item) {
     // We'll need to scan -- start at the oldest data
     tail.item = buffer.oldest_item;
   } else {
     // We'll continue from the last read()
     //
     // Rewind if needed. Note that prev_timestamp is only 0
     // if there was no previous packet.
     while( tail.item->prev_timestamp >= from ) {
       LOG_DEBUG_STR("Rewinding, am " << (tail.item->prev_timestamp - from) << " ahead");
       -- tail;
     }
   }

   /*
    * Locate any data we can find and process it.
    *
    * We drop out of the loop when we either:
    *    - have all our data (from == to)
    *    - notice that the rest of the data is lost
    */
 
   while( from < to ) {
     const int64 tailTime = tail.item->timestamp;

     if (tailTime == 0) {
       // invalid or no data

       // prevent infinite loops if we run in lock-step with the writer
       if (buffer.oldest >= to)
         break;

       ++ tail;
       continue;
     }

     if (tailTime < from) {
       // data is not useful to us
       ++ tail;
       continue;
     }

     if (tailTime >= to) {
       // Two possibilities:
       //   1. Genuine data loss beyond the end of the packet.
       //   2. Writer gained on us and wrote a new packet here.

       if (tail.item->prev_timestamp < from) {
         // 1: this packet belongs here, so the loss
         // spans across the end of the packet.
         break;
       }

       // 2: We're somewhere we should not be, most likely
       // because the writer overwrote these packets.

       ASSERT(realtime);

       if (buffer.oldest >= to) {
         // there is no data for us anymore
         break;
       }

       // valid data after head
       LOG_DEBUG_STR( "Sync with head" );
       tail.item = buffer.oldest_item;
       continue;
     }

     if (tailTime != from) {
       // data loss
       LOG_DEBUG_STR( "Data loss within packet" );
       sink.missingData( this, from, tailTime );
     }

     // a packet!
     sink.processPacket( this, tailTime, tail.item->packet );

     /*
      * If the writer did *anything* with this packet, the
      * timestamp will have been changed.
      */ 

     if (tailTime != tail.item->timestamp) {
       // we got interrupted
       ASSERT(realtime);

       // mark packet as missing
       LOG_DEBUG_STR( "Data loss due to read/write conflict" );
       sink.missingData( this, tailTime, tailTime + buffer.timesPerSlot );
     }

     // look for the next packet
     from = tailTime + buffer.timesPerSlot;

     // no need to reconsider this packet
     ++ tail;
   }

   if (from < to) {
     LOG_DEBUG_STR( "Data loss at end of packet" );
     sink.missingData( this, from, to );
   }
}

void PacketBuffer::registerConsumer( Consumer &consumer )
{
  ScopedLock sl(consumersMutex);

  consumers.push_back(&consumer);
}

void PacketBuffer::unregisterConsumer( Consumer &consumer )
{
  ScopedLock sl(consumersMutex);

  for (vector<Consumer*>::iterator i = consumers.begin(); i != consumers.end(); ++i) {
    if (*i == &consumer) {
      consumers.erase(i);
      break;
    }
  }
}

#if 0
/*
 * Input from one RSP board
 */
class RSPBoardInput {
public:
  RSPBoardInput( FileDescriptorBasedStream &inputStream, unsigned subbandsPerPacket, unsigned timesPerSlot, unsigned nrPolarizations, unsigned sampleSize );

  void read();

  PacketBuffer buffer;

private:
  FileDescriptorBasedStream &inputStream;

  const unsigned subbandsPerPacket;
  const unsigned timesPerSlot;
  const unsigned nrPolarizations;
  const unsigned sampleSize;

  const size_t subbandSize;
  const size_t packetSize;
};

RSPBoardInput::RSPBoardInput( FileDescriptorBasedStream &inputStream, unsigned subbandsPerPacket, unsigned timesPerSlot, unsigned nrPolarizations, unsigned sampleSize )
:
  buffer(100),
  inputStream(inputStream),
  packetReadOffset(0),

  subbandsPerPacket(subbandsPerPacket),
  timesPerSlot(timesPerSlot),
  nrPolarizations(nrPolarizations),
  sampleSize(sampleSize),

  subbandSize(timesPerSlot * nrPolarizations * sampleSize),
  packetSize(sizeof(struct RSP::Header) + subbandsPerPacket * subbandSize)
{
  ASSERT(packetSize <= sizeof(struct RSP));
}

void RSPBoardInput::read()
{
  /*
   * Read packets until we block.
   */
  for(;;) {
    struct RSP &packet = buffer.head.item->packet;

    void *dstPtr = reinterpret_cast<char*>(packet) + packetReadOffset;
    size_t bytesLeft = packetSize - packetReadOffset;
    size_t numbytes;

    if (packetReadOffset == 0) {
      // new packet
      buffer.startWrite();
    }

    try {
      numbytes = s.tryRead(dstPtr, bytesLeft);
    } catch(...) {
      buffer.noMoreData();

      throw;
    }

    if (numbytes == bytesLeft) {
      // finished reading packet
      buffer.stopWrite();

      packetReadOffset = 0;
    } else {
      // packet partially read
      packetReadOffset += numbytes;
    }
  }
}

/*
 *  Generates output from data generated by multiple RSP boards.
 */
class OutputGenerator {
public:
  OutputGenerator( const vector< RSPBoardInput * > &inputs, const int64 &startTime, size_t blocksize );

  int64 next_block_start;
  size_t blocksize;

  void write();

private:
  const vector< RSPboardInput * > &inputs;
  vector< PacketBuffer::Consumer * > consumers;
};

OutputGenerator::OutputGenerator( const vector< RSPBoardInput * > &inputs, const int64 &startTime, size_t blocksize )
:
  next_block_start(startTime),
  blocksize(blocksize),
  inputs(inputs),
  consumers(inputs.size(),0)
{
  for( size_t i = 0; i < inputs.size(); i++ )
    consumers[i] = new PacketBuffer::Consumer(inputs[i]->buffer);
}

void OutputGenerator::processPacket( const RSPBoardInput &input, const int64 &timestamp, struct RSP *packet )
{
/*
  char *srcPtr = packet->data;

  for (size_t sb = 0; sb < subbandsPerPacket; sb++) {
    // copy full subband
    memcpy( dstPtr, srcPtr, subbandSize );
    srcPtr += subbandSize;
  }
*/
}

void OutputGenerator::missingData( const RSPBoardInput &input, const int64 &from, const int64 to )
{
}

void OutputGenerator::write()
{
  /* todo: wait for T = next_block_start + max_wait_time */

  for( size_t i = 0; i < consumers.size(); i++ ) {
    RSPBoardInput &input = *inputs[i];
    PacketBuffer &buffer = input.buffer;
    PacketBuffer::Consumer &consumer = consumers[i];
    unsigned timesPerSlot = input.timesPerSlot;
  }
}


/*
 * Design: 
 *
 * Input is read from 
 */ 
#endif

#include <time.h>
#include <Common/Thread/Thread.h>
#include <WallClockTime.h>

time_t start;

class LogPacketSink: public PacketSink
{
public:
  virtual void processPacket( Consumer *consumer, const int64 &timestamp, struct RSP &packet ) {
    //LOG_INFO_STR( "Received packet " << timestamp );
  }
  virtual void missingData( Consumer *consumer, const int64 &from, const int64 to ) {
    LOG_INFO_STR( "Missed data from " << from << " to " << to );
  }
};

class ConsumerThread {
public:
  void run();

  Consumer *consumer;
  Thread thread;

  ConsumerThread( Consumer *consumer ): consumer(consumer), thread( this, &ConsumerThread::run ) {}
};

void ConsumerThread::run()
{
  TimeStamp cts(start, 0, sampleClock);

  for (size_t i = 0; i < 16; ++i) {
    TimeStamp from = cts;
    TimeStamp to   = cts + 16 * 1000;
    LOG_INFO_STR( ">>>> Reading from " << (int64)from << " to " << (int64)to << " <<<<<" );

    struct timespec ts = to;

    consumer->wait( to, ts );
    consumer->read( from, to );
    LOG_INFO_STR( "<<<< Done reading from " << (int64)from << " to " << (int64)to << " >>>>>" );
    cts = to;
  }
}

int main( int argc, char **argv ) {
  INIT_LOGGER(argv[0]);

  bool realtime = true;

  PacketBuffer buffer( 1100, sampleClock, 16, realtime );
  LogPacketSink logsink;
  Consumer consumer( buffer, logsink );
  buffer.registerConsumer(consumer);

  ConsumerThread cthread( &consumer );

  start = time(0) + 1;

  WallClockTime wct;

  TimeStamp ts(start, 0, sampleClock);

  for (size_t i = 0; i < 100; ++i) {
    wct.waitUntil(ts + 0000);

    for (size_t j = 0; j < 160; ++j) {
      buffer.startWrite();

      struct RSP &packet = buffer.writePtr();

      packet.header.timestamp = ts.getSeqId();
      packet.header.blockSequenceNumber = ts.getBlockId();
    
      buffer.stopWrite();

      ts += 16;
    }
  }

  buffer.noMoreData();
}
