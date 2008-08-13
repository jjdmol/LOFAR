//#  CS1_BGL_Processing_main.cc:
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
#include <CS1_Interface/BGL_Command.h>
#include <CS1_Interface/BGL_Configuration.h>
#include <FCNP_ClientStream.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <CS1_BGLProc/LocationInfo.h>
#include <CS1_BGLProc/BGL_Processing.h>
#include <CS1_BGLProc/Package__Version.h>

#include <boost/lexical_cast.hpp>

#if defined HAVE_MPI
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>
#endif

#if defined HAVE_FCNP && defined HAVE_BGP
#include <fcnp_cn.h>
#endif


using namespace LOFAR;
using namespace LOFAR::CS1;

int main(int argc, char **argv)
{
  std::clog.rdbuf(std::cout.rdbuf());

  try {
    BGL_Processing::original_argv = argv;

#if defined HAVE_MPI
    MPI_Init(&argc, &argv);
#endif

    LocationInfo locationInfo;
    
    if (locationInfo.rank() == 0) {
      std::string type = "brief";
      Version::show<CS1_BGLProcVersion> (std::cout, "CS1_BGLProc", type);
    }

    std::clog << "creating connection to ION ..." << std::endl;

    Stream *ionStream;

#if defined HAVE_ZOID && defined HAVE_BGL
    ionStream = new ZoidClientStream;
#elif 1 &&  defined HAVE_FCNP && defined HAVE_BGP
    std::vector<unsigned> psetDimensions(3);

    psetDimensions[0] = 4;
    psetDimensions[1] = 2;
    psetDimensions[2] = 2;

    FCNP_CN::init(psetDimensions);
    ionStream = new FCNP_ClientStream;
#elif 0
    ionStream = new NullStream;
#elif 0   
    usleep(10000 * locationInfo.rankInPset()); // do not connect all at the same time

    ionStream = new SocketStream("127.0.0.1", 5000 + locationInfo.rankInPset(), SocketStream::TCP, SocketStream::Client);
#else
    throw std::runtime_error("unknown Stream type between ION and CN");
#endif    

    std::clog << "connection successful" << std::endl;

    BGL_Processing proc(ionStream, locationInfo);
    BGL_Command	   command;

    do {
      command.read(ionStream);

      switch (command.value()) {
	case BGL_Command::PREPROCESS :	{
					  BGL_Configuration configuration;

					  configuration.read(ionStream);
					  proc.preprocess(configuration);
					}
					break;

	case BGL_Command::PROCESS :	proc.process();
					break;

	case BGL_Command::POSTPROCESS :	proc.postprocess();
					break;

	default :			break;
      }
    } while (command.value() != BGL_Command::STOP);

    delete ionStream;
    
#if defined HAVE_MPI
    MPI_Finalize();
#endif
    
    return 0;
  } catch (Exception &ex) {
    std::cerr << "Uncaught Exception: " << ex.what() << std::endl;
    return 1;
  } catch (std::exception &ex) {
    std::cerr << "Uncaught exception: " << ex.what() << std::endl;
    return 1;
  }
}
