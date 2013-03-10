#include <lofar_config.h>
#include "PacketsToBuffer.h"
#include <Common/LofarLogger.h>
#include <Interface/SmartPtr.h>
#include <Interface/Stream.h>
#include <Stream/Stream.h>
#include "RSP.h"
#include "SampleBuffer.h"
#include "BufferSettings.h"
#include "PacketReader.h"
#include "PacketWriter.h"
#include <boost/format.hpp>
#include <string>
#include <ios>

namespace LOFAR {
namespace RTCP {


PacketsToBuffer::PacketsToBuffer( Stream &inputStream, const BufferSettings &settings, unsigned boardNr )
:
  logPrefix(str(boost::format("[station %s board %u] ") % settings.station % boardNr)),
  inputStream(inputStream),
  settings(settings),
  boardNr(boardNr)
{
  LOG_INFO_STR( logPrefix << "Initialised" );
}



void PacketsToBuffer::process()
{
  // Holder for packet
  struct RSP packet;

  // Whether packet has been read already
  bool packetValid = false;

  // Keep reading if mode changes
  for(;;) {
    try {
      // Process packets based on (expected) bit mode
      switch(settings.station.bitMode) {
        case 16:
          process< SampleType<i16complex> >(packet, packetValid);
          break;

        case 8:
          process< SampleType<i8complex> >(packet, packetValid);
          break;

        case 4:
          process< SampleType<i4complex> >(packet, packetValid);
          break;
      }

      // process<>() exited gracefully, so we're done
      break;
    } catch (PacketReader::BadModeException &ex) {
      // Mode switch detected
      unsigned bitMode    = packet.bitMode();
      unsigned clockMHz   = packet.clockMHz();
      unsigned nrBeamlets = packet.header.nrBeamlets;

      LOG_INFO_STR( logPrefix << "Mode switch detected to " << clockMHz << " MHz, " << bitMode << " bit, " << nrBeamlets << " beamlets");

      // update settings
      settings.station.bitMode = bitMode;
      settings.station.clockMHz = clockMHz;
      settings.nrBeamletsPerBoard = nrBeamlets;

      // Process packet again
      packetValid = true;
    }
  }
}


template<typename T> void PacketsToBuffer::process( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException)
{
  // Create input structures
  PacketReader reader(logPrefix, inputStream);

  // Create output structures
  SampleBuffer<T> buffer(settings, true);
  PacketWriter<T> writer(logPrefix, buffer, buffer.flags[boardNr], settings.nrBeamletsPerBoard * boardNr, settings);

  try {
    // Process lingering packet from previous run, if any
    if (writeGivenPacket)
      writer.writePacket(packet);

    // Transport packets from reader to writer
    for(;;)
      if (reader.readPacket(packet, settings))
        writer.writePacket(packet);

  } catch (PacketReader::BadModeException &ex) {
    // Packet has different clock or bitmode
    throw;
  } catch (Stream::EndOfStreamException &ex) {
    // Ran out of data
    LOG_INFO_STR( logPrefix << "End of stream");

  } catch (SystemCallException &ex) {
    if (ex.error == EINTR)
      LOG_INFO_STR( logPrefix << "Aborted: " << ex.what());
    else
      LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);

  } catch (Exception &ex) {
    LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
  }

  LOG_INFO_STR( logPrefix << "End");
}


template void PacketsToBuffer::process< SampleType<i16complex> >( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException);
template void PacketsToBuffer::process< SampleType<i8complex> >( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException);
template void PacketsToBuffer::process< SampleType<i4complex> >( struct RSP &packet, bool writeGivenPacket ) throw(PacketReader::BadModeException);


}
}
