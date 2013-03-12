#include <lofar_config.h>
#include <Station/PacketReader.h>
#include <Common/LofarLogger.h>
#include <Stream/FileStream.h>

using namespace LOFAR;
using namespace Cobalt;

void test(const std::string &filename, unsigned bitmode, unsigned nrPackets)
{
  FileStream fs(filename);

  PacketReader reader("", fs);

  struct RSP packet;

  // We should be able to read these packets
  for( size_t i = 0; i < nrPackets; ++i) {
    ASSERT( reader.readPacket(packet) );
    ASSERT( packet.bitMode() == bitmode );
    ASSERT( packet.clockMHz() == 200 );
  }

  // The file contains no more packets; test if readPacket survives
  // a few calls on the rest of the stream.
  for( size_t i = 0; i < 3; ++i) {
    try {
      ASSERT( !reader.readPacket(packet) );
    } catch (Stream::EndOfStreamException &ex) {
      // expected
    }
  }
}

int main()
{
  INIT_LOGGER("tPacketReader");

  test("tPacketReader.in_16bit", 16, 2);
  test("tPacketReader.in_8bit",   8, 2);
}
