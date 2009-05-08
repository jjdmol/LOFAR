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

#if defined HAVE_FCNP && defined HAVE_BGP
#include <FCNP/fcnp_cn.h>
#endif

// if exceptions are not caught, an attempt is made to create a backtrace
// from the place where the exception is thrown.
#undef CATCH_EXCEPTIONS


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
#endif

    LocationInfo locationInfo;
    
    std::stringstream sysInfo;
    sysInfo << basename(argv[0]) << "@" << locationInfo.rank();

#if defined HAVE_BGP 
    INIT_BGP_LOGGER(sysInfo.str());
#endif
  
#if !defined HAVE_PKVERSION    
    if (locationInfo.rank() == 0) {
      std::string type = "brief";
      Version::show<CNProcVersion> (std::cout, "CNProc", type);
    }
#endif
    LOG_DEBUG("creating connection to ION ...");
    
    Stream *ionStream;
#if defined HAVE_ZOID && defined HAVE_BGL
    ionStream = new ZoidClientStream;
#elif 1 &&  defined HAVE_FCNP && defined HAVE_BGP
    FCNP_CN::init();
    ionStream = new FCNP_ClientStream;
#elif 0
    ionStream = new NullStream;
#elif 0   
    usleep(10000 * locationInfo.rankInPset()); // do not connect all at the same time

    ionStream = new SocketStream("127.0.0.1", 5000 + locationInfo.rankInPset(), SocketStream::TCP, SocketStream::Client);
#else
    THROW(CNProcException, "unknown Stream type between ION and CN");
#endif    

    LOG_DEBUG("connection successful");

    CN_Configuration	configuration;
    CN_Processing_Base	*proc = 0;
    CN_Command		command;

    do {
      command.read(ionStream);

      switch (command.value()) {
	case CN_Command::PREPROCESS :	configuration.read(ionStream);

					switch (configuration.nrBitsPerSample()) {
					  case 4:  proc = new CN_Processing<i4complex>(ionStream, locationInfo);
						   break;

					  case 8:  proc = new CN_Processing<i8complex>(ionStream, locationInfo);
						   break;

					  case 16: proc = new CN_Processing<i16complex>(ionStream, locationInfo);
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

	case CN_Command::STOP :	break;

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
