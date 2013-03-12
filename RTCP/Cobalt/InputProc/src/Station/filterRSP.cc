#include <lofar_config.h>
#include "PacketReader.h"
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <Common/LofarLogger.h>
#include <ApplCommon/PosixTime.h>
#include <CoInterface/Stream.h>
#include <CoInterface/SmartPtr.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace LOFAR;
using namespace Cobalt;

time_t parseTime(const char *str)
{
  return to_time_t(boost::posix_time::time_from_string(str));
}

int main(int argc, char **argv)
{
  INIT_LOGGER("filterRSP");

  if (argc < 3) {
    puts("Usage: filterRSP '2012-01-01 11:12:00' '2012-01-01 11:13:00' < input.udp > output.udp");
    puts("");
    puts("Writes all packets between the given timestamps: [from,to).");
    exit(1);
  }

  time_t from = parseTime(argv[1]);
  time_t to = parseTime(argv[2]);

  SmartPtr<Stream> inputStream = createStream("file:/dev/stdin", true);
  PacketReader reader("", *inputStream);
  struct RSP packet;

  try {
    for(;; ) {
      if( reader.readPacket(packet) ) {
        if (packet.header.timestamp < from || packet.header.timestamp >= to)
          continue;

        fwrite(&packet, packet.packetSize(), 1, stdout);
      }
    }
  } catch(Stream::EndOfStreamException&) {
  }
}

