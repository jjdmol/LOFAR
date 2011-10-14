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

#include <CNProc/LocationInfo.h>
#include <CNProc/CN_Processing.h>
#include <CNProc/Package__Version.h>
#include <Common/Exception.h>
#include <Common/LofarLogger.h>
#include <Common/NewHandler.h>
#include <Interface/CN_Command.h>
#include <Interface/Exceptions.h>
#include <Interface/Parset.h>
#include <Interface/SmartPtr.h>
#include <Interface/Stream.h>

#include <boost/format.hpp>
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

  std::string descriptor = getStreamDescriptorBetweenIONandCN(ionStreamType, psetNumber, rankInPset, nrPsets, psetSize, channel);

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
    (void) argc;
    (void) argv;
#endif

    LocationInfo locationInfo;

#if defined HAVE_LOG4CPLUS
    INIT_LOGGER( "CNProc" );
#elif defined HAVE_LOG4CXX
    #error LOG4CXX support is broken (nonsensical?) -- please fix this code if you want to use it
    Context::initialize();
    setLevel("Global",8);
#else
    INIT_LOGGER_WITH_SYSINFO(str(boost::format("CNProc@%04d") % locationInfo.rank()));
#endif

    if (locationInfo.rank() == 0) {
      locationInfo.print();

#if !defined HAVE_PKVERSION    
      std::string type = "brief";
      Version::show<CNProcVersion> (std::cout, "CNProc", type);
#endif
    }

    LOG_INFO_STR("Core " << locationInfo.rank() << " is core " << locationInfo.rankInPset() << " in pset " << locationInfo.psetNumber());

    getIONstreamType();

#if defined CLUSTER_SCHEDULING
    LOG_DEBUG("Creating connections to IONs ...");

    std::vector<SmartPtr<Stream> > ionStreams(locationInfo.nrPsets());

    for (unsigned ionode = 0; ionode < locationInfo.nrPsets(); ionode ++) {
      std::string descriptor = getStreamDescriptorBetweenIONandCN(ionStreamType, ionode, locationInfo.rankInPset(), locationInfo.nrPsets(), locationInfo.psetSize(), 0);
      ionStreams[ionode] = createStream(descriptor, false);
    }

    LOG_DEBUG("Creating connections to IONs done");
    SmartPtr<Stream> &ionStream = ionStreams[0];
#else
    if (locationInfo.rankInPset() == 0)
      LOG_DEBUG("Creating connection to ION ...");

    SmartPtr<Stream> ionStream(createIONstream(0, locationInfo));

    if (locationInfo.rankInPset() == 0)
      LOG_DEBUG("Creating connection to ION: done");
#endif

    SmartPtr<Parset>		 parset;
    SmartPtr<CN_Processing_Base> proc;
    CN_Command			 command;

    do {
      LOG_DEBUG("Wait for command");
      command.read(ionStream);
      LOG_DEBUG_STR("Received command " << command.value());

      switch (command.value()) {
	case CN_Command::PREPROCESS :	try {
					  parset = new Parset(ionStream);

				          switch (parset->nrBitsPerSample()) {
#if defined CLUSTER_SCHEDULING
                                            case 4:  proc = new CN_Processing<i4complex>(*parset, ionStreams, &createIONstream, locationInfo);
                                                     break;

                                            case 8:  proc = new CN_Processing<i8complex>(*parset, ionStreams, &createIONstream, locationInfo);
                                                     break;

                                            case 16: proc = new CN_Processing<i16complex>(*parset, ionStreams, &createIONstream, locationInfo);
                                                     break;
#else
                                            case 4:  proc = new CN_Processing<i4complex>(*parset, ionStream, &createIONstream, locationInfo);
                                                     break;

                                            case 8:  proc = new CN_Processing<i8complex>(*parset, ionStream, &createIONstream, locationInfo);
                                                     break;

                                            case 16: proc = new CN_Processing<i16complex>(*parset, ionStream, &createIONstream, locationInfo);
                                                     break;
#endif
                                          }
                                        } catch (Exception &ex) {
                                          LOG_ERROR_STR("Caught Exception: " << ex);
                                        } catch (std::exception &ex) {
                                          LOG_ERROR_STR("Caught Exception: " << ex.what());
                                        } catch (...) {
                                          LOG_ERROR_STR("Caught Exception: unknown");
                                        }

#if 0 // FIXME: leads to deadlock when using TCP
					{
					  char failed = proc == 0;
					  ionStream->write(&failed, sizeof failed);
					}
#endif

					break;

	case CN_Command::PROCESS :	proc->process(command.param());
					break;

	case CN_Command::POSTPROCESS :	// proc == 0 if PREPROCESS threw an exception, after which all cores receive a POSTPROCESS message
					delete proc.release();
					delete parset.release();
					break;

	case CN_Command::STOP :		break;

	default :			LOG_FATAL("Bad command!");
					abort();
      }
    } while (command.value() != CN_Command::STOP);

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
