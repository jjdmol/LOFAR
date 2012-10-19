#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Stream/Stream.h>
#include <Stream/SocketStream.h>
#include <RSP.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SmartPtr.h>
#include <Interface/Stream.h>
#include <WallClockTime.h>
#include "SharedMemory.h"
#include "Ranges.h"
#include "OMPThread.h"
#include "mpi.h"

#include <vector>
#include <omp.h>
#include <map>
#include <string>
#include <ostream>
#include <cstring>
#include <boost/format.hpp>

#define NR_RSPBOARDS 4

using namespace LOFAR;
using namespace RTCP;

template<typename T> struct SampleType {
  std::complex<T> x;
  std::complex<T> y;
};

struct StationID {
  char stationName[64];
  char antennaSet[64];

  unsigned clock;
  unsigned bitmode;

  StationID( const std::string &stationName = "", const std::string &antennaSet = "", unsigned clock = 200 * 1000000, unsigned bitmode = 16)
  :
    clock(clock),
    bitmode(bitmode)
  {
    snprintf(this->stationName, sizeof this->stationName, "%s", stationName.c_str());
    snprintf(this->antennaSet, sizeof this->antennaSet, "%s", antennaSet.c_str());
  }

  bool operator==(const struct StationID &other) const {
    return !strncmp(stationName, other.stationName, sizeof stationName)
        && !strncmp(antennaSet, other.antennaSet, sizeof antennaSet)
        && clock == other.clock
        && bitmode == other.bitmode;
  }

  bool operator!=(const struct StationID &other) const {
    return !(*this == other);
  }

  uint32 hash() const {
    // convert to 32 bit value (human-readable in hexadecimal)
    uint32 stationNr = 0;

    const std::string stationNameStr(stationName);
    const std::string antennaSetStr(antennaSet);

    for(std::string::const_iterator c = stationNameStr.begin(); c != stationNameStr.end(); ++c)
      if(*c >= '0' && *c <= '9')
        stationNr = stationNr * 16 + (*c - '0');

    uint32 antennaSetNr = 0;

    if (antennaSetStr == "HBA_ONE" || antennaSetStr == "HBA1" )
      antennaSetNr = 1;
    else
      antennaSetNr = 0;

    ASSERT( stationNr    < (1L << 16) );
    ASSERT( antennaSetNr < (1L << 4)  );

    ASSERT( clock/1000000 == 200 || clock/1000000 == 160 );
    ASSERT( bitmode == 4 || bitmode == 8 || bitmode == 16 );

    unsigned clockNr = clock/1000000 == 200 ? 0x20 : 0x16;
    unsigned bitmodeNr = bitmode == 16 ? 0xF : bitmode;

    return (stationNr << 16) + (antennaSetNr << 12) + (clockNr << 4) + bitmodeNr;
  }

};

std::ostream& operator<<( std::ostream &str, const struct StationID &s ) {
  str << "station " << s.stationName << " antennaset " << s.antennaSet << " clock " << s.clock/1000000 << " bitmode " << s.bitmode;

  return str;
}

struct StationSettings {
private:
  static const unsigned currentVersion = 1;

  unsigned version;

  bool valid() const { return version == currentVersion; }

public:
  struct StationID station;

  unsigned nrBeamlets;

  size_t   nrSamples;

  unsigned nrBoards;
  size_t   nrFlagRanges;

  key_t    dataKey;

  StationSettings();

  // read settings from shared memory, using the given stationID
  StationSettings(struct StationID station);

  bool operator==(const struct StationSettings &other) const {
    return station == other.station
        && nrBeamlets == other.nrBeamlets 
        && nrSamples == other.nrSamples
        && nrBoards == other.nrBoards
        && nrFlagRanges == other.nrFlagRanges
        && dataKey == other.dataKey;
  }

};

std::ostream& operator<<( std::ostream &str, const struct StationSettings &s ) {
  str << s.station << " beamlets: " << s.nrBeamlets << " buffer: " << (1.0 * s.nrSamples / s.station.clock * 1024) << "s";

  return str;
}

StationSettings::StationSettings()
:
  version(currentVersion)
{
}


StationSettings::StationSettings(struct StationID station)
:
  version(currentVersion),
  station(station)
{
  SharedStruct<struct StationSettings> shm(station.hash(), false);

  *this = shm.get();

  ASSERT( valid() );
}


template<typename T> class SampleBuffer {
public:
  SampleBuffer( const struct StationSettings &settings, bool create );

private:
  const std::string logPrefix;
  SharedMemoryArena data;
  SparseSetAllocator allocator;

  struct StationSettings *initSettings( const struct StationSettings &localSettings, bool create );

  static size_t dataSize( const struct StationSettings &settings ) {
    return sizeof settings
         + NR_RSPBOARDS * (Ranges::size(settings.nrFlagRanges) + 8)
         + settings.nrBeamlets * (settings.nrSamples * N_POL * 2 * settings.station.bitmode / 8 + 128);
  }

public:
  struct StationSettings *settings;

  const size_t nrBeamlets;
  const size_t nrSamples;
  const size_t nrFlagRanges;

  MultiDimArray<T,2>  beamlets; // [subband][sample]
  std::vector<Ranges> flags;    // [rspboard]
};


template<typename T> SampleBuffer<T>::SampleBuffer( const struct StationSettings &_settings, bool create )
:
  logPrefix(str(boost::format("[station %s %s board] [SampleBuffer] ") % _settings.station.stationName % _settings.station.antennaSet)),
  data(_settings.dataKey, dataSize(_settings), create ? SharedMemoryArena::CREATE_EXCL : SharedMemoryArena::READ),
  allocator(data),
  settings(initSettings(_settings, create)),

  nrBeamlets(settings->nrBeamlets),
  nrSamples(settings->nrSamples),
  nrFlagRanges(settings->nrFlagRanges),

  beamlets(boost::extents[nrBeamlets][nrSamples], 128, allocator, false, create),
  flags(settings->nrBoards)
{
  // bitmode must coincide with our template
  ASSERT( sizeof(T) == N_POL * 2 * settings->station.bitmode / 8 );

  // typical #slots/packet
  ASSERT( settings->nrSamples % 16 == 0 );

  for (size_t f = 0; f < flags.size(); f++) {
    size_t numBytes = Ranges::size(nrFlagRanges);

    flags[f] = Ranges(static_cast<int64*>(allocator.allocate(numBytes, 8)), numBytes, nrSamples, create);
  }

  LOG_INFO_STR( logPrefix << "Initialised" );
}

template<typename T> struct StationSettings *SampleBuffer<T>::initSettings( const struct StationSettings &localSettings, bool create )
{
  //struct StationSettings *sharedSettings = allocator.allocateTyped<struct StationSettings>();
  struct StationSettings *sharedSettings = allocator.allocateTyped();

  if (create) {
    // register settings
    LOG_INFO_STR( logPrefix << "Registering " << localSettings.station );
    *sharedSettings = localSettings;
  } else {
    // verify settings
    ASSERT( *sharedSettings == localSettings );
    LOG_INFO_STR( logPrefix << "Connected to " << localSettings.station );
  }

  return sharedSettings;
}

template<typename T> class RSPBoard {
public:
  RSPBoard( Stream &inputStream, SampleBuffer<T> &buffer, unsigned boardNr, const struct StationSettings &settings );

  const unsigned nr;

  bool readPacket();
  void writePacket();

  static size_t packetSize( struct RSP &packet ) {
    return sizeof(struct RSP::Header) + packet.header.nrBeamlets * packet.header.nrBlocks * sizeof(T);
  }
  
private:
  const std::string logPrefix;

  Stream &inputStream;
  const bool supportPartialReads;
  struct RSP packet;
  TimeStamp last_timestamp;
  TimeStamp last_logtimestamp;

  SampleBuffer<T> &buffer;
  Ranges &flags;
  const struct StationSettings settings;
  const size_t firstBeamlet;

  size_t nrReceived, nrBadSize, nrBadTime, nrOutOfOrder;

  void logStatistics();
};

template<typename T> RSPBoard<T>::RSPBoard( Stream &inputStream, SampleBuffer<T> &buffer, unsigned boardNr, const struct StationSettings &settings )
:
  nr(boardNr),
  logPrefix(str(boost::format("[station %s %s board %u] [RSPBoard] ") % settings.station.stationName % settings.station.antennaSet % nr)),
  inputStream(inputStream),
  supportPartialReads(dynamic_cast<SocketStream *>(&inputStream) == 0 || dynamic_cast<SocketStream &>(inputStream).protocol != SocketStream::UDP),

  buffer(buffer),
  flags(buffer.flags[boardNr]),
  settings(settings),
  firstBeamlet(settings.nrBeamlets / settings.nrBoards * boardNr),

  nrReceived(0),
  nrBadSize(0),
  nrBadTime(0),
  nrOutOfOrder(0)
{
}

template<typename T> void RSPBoard<T>::writePacket()
{
  const uint8 &nrBeamlets  = packet.header.nrBeamlets;
  const uint8 &nrTimeslots = packet.header.nrBlocks;

  ASSERT( settings.nrSamples % nrTimeslots == 0 );

  // the timestamp is of the last read packet by definition
  const TimeStamp &timestamp = last_timestamp;

  const size_t bufferOffset = (int64)timestamp % settings.nrSamples;

  const T *beamlets = reinterpret_cast<const T*>(&packet.data);

  ASSERT( nrBeamlets <= settings.nrBeamlets / settings.nrBoards );

  // mark data we overwrite as invalid
  flags.excludeBefore(timestamp + nrTimeslots - settings.nrSamples);

  // transpose
  for (uint8 b = 0; b < nrBeamlets; ++b) {
    T *dst = &buffer.beamlets[firstBeamlet + b][bufferOffset];

    memcpy(dst, beamlets, nrTimeslots * sizeof(T));

    beamlets += nrTimeslots;
  }

  // mark as valid
  flags.include(timestamp, timestamp + nrTimeslots);
}

template<typename T> bool RSPBoard<T>::readPacket()
{
  if (supportPartialReads) {
    // read header first
    inputStream.read(&packet, sizeof(struct RSP::Header));

    // read rest of packet
    inputStream.read(&packet.data, packetSize(packet) - sizeof(struct RSP::Header));

    ++nrReceived;
  } else {
    // read full packet at once -- numbytes will tell us how much we've actually read
    size_t numbytes = inputStream.tryRead(&packet, sizeof packet);

    ++nrReceived;

    if( numbytes < sizeof(struct RSP::Header)
     || numbytes != packetSize(packet) ) {
      LOG_WARN_STR( logPrefix << "Packet is " << numbytes << " bytes, but should be " << packetSize(packet) << " bytes" );

      ++nrBadSize;
      return false;
    }
  }

  // check sanity of packet

  // detect bad timestamp
  if (packet.header.timestamp == ~0U) {
    ++nrBadTime;
    return false;
  }

  const TimeStamp timestamp(packet.header.timestamp, packet.header.blockSequenceNumber, settings.station.clock);

  // detect out-of-order data
  if (timestamp < last_timestamp) {
    ++nrOutOfOrder;
    return false;
  }

  // don't accept big jumps (>10s) in timestamp
  const int64 oneSecond = settings.station.clock / 1024;

  if (last_timestamp && packet.header.timestamp > last_timestamp + 10 * oneSecond) {
    ++nrBadTime;
    return false;
  }

  // packet was read and is sane

  last_timestamp = timestamp;

  if (timestamp > last_logtimestamp + oneSecond) {
    logStatistics();

    last_logtimestamp = timestamp;
  }

  return true;
}


template<typename T> void RSPBoard<T>::logStatistics()
{
  LOG_INFO_STR( logPrefix << "Received " << nrReceived << " packets: " << nrOutOfOrder << " out of order, " << nrBadTime << " bad timestamps, " << nrBadSize << " bad sizes" );

  nrReceived = 0;
  nrOutOfOrder = 0;
  nrBadTime = 0;
  nrBadSize = 0;
}






class StationStreams {
public:
  StationStreams( const std::string &logPrefix, const StationSettings &settings, const std::vector<std::string> &streamDescriptors );

  void process();

  void stop();

protected:
  const std::string logPrefix;
  const StationSettings settings;
  const std::vector<std::string> streamDescriptors;
  const size_t nrBoards;

  WallClockTime waiter;

  virtual void processBoard( size_t nr ) = 0;
};

StationStreams::StationStreams( const std::string &logPrefix, const StationSettings &settings, const std::vector<std::string> &streamDescriptors )
:
  logPrefix(logPrefix),
  settings(settings),
  streamDescriptors(streamDescriptors),
  nrBoards(streamDescriptors.size())
{
}

void StationStreams::process()
{
  std::vector<OMPThread> threads(nrBoards);

  ASSERT(nrBoards > 0);

  LOG_INFO_STR( logPrefix << "Start" );

  #pragma omp parallel sections num_threads(2)
  {
    #pragma omp section
    {
      // start all boards
      LOG_INFO_STR( logPrefix << "Starting all boards" );
      #pragma omp parallel for num_threads(nrBoards)
      for (size_t i = 0; i < nrBoards; ++i) {
        OMPThread::ScopedRun sr(threads[i]);
  
        processBoard(i);
      }
    }

    #pragma omp section
    {
      // wait until we have to stop
      LOG_INFO_STR( logPrefix << "Waiting for stop signal" );
      waiter.waitForever();     

      // kill all boards
      LOG_INFO_STR( logPrefix << "Stopping all boards" );
      #pragma omp parallel for num_threads(nrBo