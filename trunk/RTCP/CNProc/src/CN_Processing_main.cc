//#  CN_Processing_main.cc:
//#
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#include <Common/Exception.h>
#include <Common/NewHandler.h>
#include <Interface/CN_Command.h>
#include <Interface/CN_Configuration.h>
#include <Interface/Exceptions.h>
#include <Interface/Stream.h>
#include <CNProc/LocationInfo.h>
#include <CNProc/CN_Processing.h>
#include <Common/LofarLogger.h>
#include <CNProc/Package__Version.h>
#include <boost/lexical_cast.hpp>
#include <execinfo.h>

#if defined HAVE_MPI
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>
#endif

#if defined HAVE_FCNP && defined HAVE_BGP_CN && !defined USE_VALGRIND
#include <FCNP_ClientStream.h>
#include <FCNP/fcnp_cn.h>
#endif

#include <cstdio>
#include <cstring>

#include <boost/format.hpp>
using boost::format;


// install a new handler to produce backtraces for std::bad_alloc
LOFAR::NewHandler h(LOFAR::BadAllocException::newHandler);


// if exceptions are not caught, an attempt is made to create a backtrace
// from the place where the exception is thrown.
#define CATCH_EXCEPTIONS


using namespace LOFAR;
using namespace LOFAR::RTCP;

#if !defined CATCH_EXCEPTIONS

void terminate_with_backtrace()
{
  LOG_FATAL("terminate_with_backtrace()");

  void *buffer[100];
  int  nptrs	 = backtrace(buffer, 100);
  char **strings = backtrace_symbols(buffer, nptrs);

  for (int i = 0; i < nptrs; i ++)
    LOG_FATAL_STR(i << ": " << strings[i]);

  free(strings);
  abort();
}

#endif

static const char *ionStreamType;


static void getIONstreamType()
{
  if ((ionStreamType = getenv("CN_STREAM_TYPE")) == 0)
#if !defined HAVE_BGP_CN
    ionStreamType = "NULL";
#elif defined HAVE_FCNP && defined __PPC__ && !defined USE_VALGRIND
    ionStreamType = "FCNP";
#else
    ionStreamType = "TCPKEY";
#endif

#if defined HAVE_FCNP && defined HAVE_BGP_CN && !defined USE_VALGRIND
  if (ionStreamType == "FCNP")
    FCNP_CN::init();
#endif
}


static Stream *createIONstream(unsigned channel, const LocationInfo &locationInfo)
{
#if defined HAVE_FCNP && defined HAVE_BGP_CN && !defined USE_VALGRIND
  if (strcmp(ionStreamType, "FCNP") == 0)
    return new FCNP_ClientStream(channel);
#endif

  unsigned nrPsets = locationInfo.nrPsets();
  unsigned psetSize = locationInfo.psetSize();
  unsigned psetNumber = locationInfo.psetNumber();
  unsigned rankInPset = locationInfo.rankInPset();

  string descriptor = getStreamDescriptorBetweenIONandCN( ionStreamType, psetNumber, rankInPset, nrPsets, psetSize, channel );

  return createStream(descriptor, false);
}

int main(int argc, char **argv)
{
  std::clog.rdbuf(std::cout.rdbuf());
  
#if !defined CATCH_EXCEPTIONS
  std::set_terminate(terminate_with_backtrace);
#endif

#if defined CATCH_EXCEPTIONS
  try {
#endif

#if defined HAVE_MPI
    MPI_Init(&argc, &argv);
#else
    (void)argc;
    (void)argv;
#endif

    LocationInfo locationInfo;

#if defined HAVE_LOG4CPLUS
    INIT_LOGGER( "CNProc" );
#elif defined HAVE_LOG4CXX
    #error LOG4CXX support is broken (nonsensical?) -- please fix this code if you want to use it
    Context::initialize();
    setLevel("Global",8);
#else
    INIT_LOGGER_WITH_SYSINFO(str(format("CNProc@%04d") % locationInfo.rank()));
#endif

    if (locationInfo.rank() == 0) {
      locationInfo.print();

#if !defined HAVE_PKVERSION    
      std::string type = "brief";
      Version::show<CNProcVersion> (std::cout, "CNProc", type);
#endif
    }

    LOG_INFO_STR("Core " << locationInfo.rank() << " is core " << locationInfo.rankInPset() << " in pset " << locationInfo.psetNumber());

    if (locationInfo.rankInPset() == 0)
      LOG_DEBUG("Creating connection to ION ...");
    
    getIONstreamType();
    Stream *ionStream = createIONstream(0, locationInfo);

    if (locationInfo.rankInPset() == 0)
      LOG_DEBUG("Creating connection to ION: done");

    CN_Configuration	configuration;
    CN_Processing_Base	*proc = 0;
    CN_Command		command;

    do {
      char failed = 0;

      //LOG_DEBUG("Wait for command");
      command.read(ionStream);
      //LOG_DEBUG("Received command");

      switch (command.value()) {
	case CN_Command::PREPROCESS :	configuration.read(ionStream);

                                        failed = 0;

                                        try {

                                          switch (configuration.nrBitsPerSample()) {
                                            case 4:  proc = new CN_Processing<i4complex>(ionStream, &createIONstream, locationInfo);
                                                     break;

                                            case 8:  proc = new CN_Processing<i8complex>(ionStream, &createIONstream, locationInfo);
                                                     break;

                                            case 16: proc = new CN_Processing<i16complex>(ionStream, &createIONstream, locationInfo);
                                                     break;
                                          }

                                          proc->preprocess(configuration);
                                        } catch (Exception &ex) {
                                          LOG_ERROR_STR("Caught Exception: " << ex);
                                          failed = 1;
                                        } catch (std::exception &ex) {
                                          LOG_ERROR_STR("Caught Exception: " << ex.what());
                                          failed = 1;
                                        } catch (...) {
                                          LOG_ERROR_STR("Caught Exception: unknown");
                                          failed = 1;
                                        }

                                        ionStream->write(&failed, sizeof failed);

                                        if (failed) {
                                          if (proc) {
                                            delete proc;
                                            proc = 0;
                                          }
                                        }
					break;

	case CN_Command::PROCESS :	proc->process(command.param());
					break;

	case CN_Command::POSTPROCESS :	if (proc) {
                                          // proc == 0 if PREPROCESS threw an exception, after which all cores receive a POSTPROCESS message
                                          proc->postprocess();
					  delete proc;
					  proc = 0;
                                        }  
					break;

	case CN_Command::STOP :		break;

	default :			LOG_FATAL("Bad command!");
					abort();
      }
    } while (command.value() != CN_Command::STOP);

    delete ionStream;
    
#if defined HAVE_MPI
    MPI_Finalize();
    usleep(500 * locationInfo.rank()); // do not dump stats all at the same time
#endif
    
    return 0;
#if defined CATCH_EXCEPTIONS
  } catch (Exception &ex) {
    LOG_FATAL_STR("Uncaught Exception: " << ex);
    return 1;
  } catch (std::exception &ex) {
    LOG_FATAL_STR("Uncaught Exception: " << ex.what());
    return 1;
  }
#endif
}
