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
#include <Interface/CN_Command.h>
#include <Interface/CN_Configuration.h>
#include <Interface/Exceptions.h>
#include <FCNP_ClientStream.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <CNProc/LocationInfo.h>
#include <CNProc/CN_Processing.h>
#include <Common/LofarLogger.h>
#if !defined HAVE_PKVERSION
#include <CNProc/Package__Version.h>
#endif
#include <boost/lexical_cast.hpp>
#include <execinfo.h>

#if defined HAVE_MPI
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>
#endif

#if defined HAVE_FCNP && defined HAVE_BGP_CN
#include <FCNP/fcnp_cn.h>
#endif

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

static Stream *createIONstream( unsigned channel )
{
#if 1 && defined HAVE_FCNP && defined HAVE_BGP_CN && !defined VALGRIND
    /* preferred */
    FCNP_CN::init();
    return new FCNP_ClientStream(channel);
#elif 1
    LocationInfo locationInfo;

    /* used by default for !HAVE_FCNP && !HAVE_BGP */ 
    usleep(10000 * locationInfo.rankInPset()); // do not connect all at the same time

    return new SocketStream("127.0.0.1", 5000 + locationInfo.rankInPset() + 1000 * channel, SocketStream::TCP, SocketStream::Client);
#elif 0
    /* used for testing */
    return new NullStream;
#else
    THROW(CNProcException, "unknown Stream type between ION and CN");
#endif    
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
    
    std::stringstream sysInfo;
    sysInfo << basename(argv[0]) << "@" << locationInfo.rank();

#if defined HAVE_BGP
    INIT_BGP_LOGGER(sysInfo.str());
#endif
  
    if (locationInfo.rank() == 0) {
      locationInfo.print();

#if !defined HAVE_PKVERSION    
      std::string type = "brief";
      Version::show<CNProcVersion> (std::cout, "CNProc", type);
#endif
    }

    LOG_DEBUG("creating connection to ION ...");
    
    Stream *ionStream = createIONstream(0);

    LOG_DEBUG("connection successful");

    CN_Configuration	configuration;
    CN_Processing_Base	*proc = 0;
    CN_Command		command;

    do {
      command.read(ionStream);

      switch (command.value()) {
	case CN_Command::PREPROCESS :	configuration.read(ionStream);

					switch (configuration.nrBitsPerSample()) {
					  case 4:  proc = new CN_Processing<i4complex>(ionStream, &createIONstream, locationInfo);
						   break;

					  case 8:  proc = new CN_Processing<i8complex>(ionStream, &createIONstream, locationInfo);
						   break;

					  case 16: proc = new CN_Processing<i16complex>(ionStream, &createIONstream, locationInfo);
						   break;
					}

					proc->preprocess(configuration);
					break;

	case CN_Command::PROCESS :	proc->process();
					break;

	case CN_Command::POSTPROCESS :	proc->postprocess();
					delete proc;
					proc = 0;
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
    LOG_FATAL_STR("Uncaught exception: " << ex.what());
    return 1;
  }
#endif
}
