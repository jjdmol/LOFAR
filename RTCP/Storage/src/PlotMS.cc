//#  Plot_MS.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id: Storage_main.cc 18363 2011-06-30 13:06:44Z mol $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Stream/FileStream.h>
#include <Interface/Parset.h>
#include <Interface/DataFactory.h>
#include <Interface/CorrelatedData.h>
#include <Common/DataConvert.h>
#include <string>
#include <cstdio>

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace std;

bool shouldSwap = false;

float power( fcomplex s ) {
  float r = real(s);
  float i = imag(s);

  if (shouldSwap) {
    byteSwap32(&r);
    byteSwap32(&i);
  }

  return r*r + i*i;
}

int main(int argc, char *argv[])
{
#if defined HAVE_LOG4CPLUS
  INIT_LOGGER( CMAKE_INSTALL_PREFIX "/etc/Storage.log_prop" );
#elif defined HAVE_LOG4CXX
  #error LOG4CXX support is broken (nonsensical?) -- please fix this code if you want to use it
  Context::initialize();
  setLevel("Global",8);
#else
  INIT_LOGGER_WITH_SYSINFO(str(boost::format("Storage@%02d") % (argc > 1 ? atoi(argv[1]) : -1)));
#endif

  try {
    if (argc != 5)
      throw StorageException(str(boost::format("usage: %s parset table.f0data baseline channel") % argv[0]), THROW_ARGS);

    Parset parset(argv[1]);
    FileStream datafile(argv[2]);
    unsigned baseline(atoi(argv[3]));
    unsigned channel(atoi(argv[4]));
    CorrelatedData *data = dynamic_cast<CorrelatedData*>(newStreamableData(parset, CORRELATED_DATA));

    ASSERT( data );
    ASSERT( baseline < parset.nrBaselines() );
    ASSERT( channel < parset.nrChannelsPerSubband() );

    for(;;) {
      try {
        data->read(&datafile, true, 512);
      } catch (Stream::EndOfStreamException &) {
        break;
      }
      data->peerMagicNumber = 0xda7a0000; // fake wrong endianness to circumvent bug
      shouldSwap = data->shouldByteSwap();

      printf( "# valid samples: %u\n", data->nrValidSamples(baseline,channel));

      printf( "%6d %10g %10g %10g %10g\n",
        data->sequenceNumber(),
        power( data->visibilities[baseline][channel][0][0] ),
        power( data->visibilities[baseline][channel][0][1] ),
        power( data->visibilities[baseline][channel][1][0] ),
        power( data->visibilities[baseline][channel][1][1] ) );

    }

  } catch (Exception &ex) {
    LOG_FATAL_STR("[obs unknown] Caught Exception: " << ex);
    exit(1);
  } catch (std::exception &ex) {
    LOG_FATAL_STR("[obs unknown] Caught std::exception: " << ex.what());
    exit(1);
  } catch (...) {
    LOG_FATAL_STR("[obs unknown] Caught non-std::exception");
    exit(1);
  }

  LOG_INFO_STR("[obs unknown] Program end");
  return 0;
}
