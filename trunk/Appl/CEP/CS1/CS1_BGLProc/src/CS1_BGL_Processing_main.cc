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

#if 0
#include <CS1_Interface/CS1_Parset.h>
#else
#include <Common/Exception.h>
#include <CS1_Interface/BGL_Command.h>
#include <CS1_Interface/BGL_Configuration.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Null.h>
#include <Transport/TH_Socket.h>
#include <CS1_BGLProc/TH_ZoidClient.h>
#endif
#include <CS1_BGLProc/LocationInfo.h>
#include <CS1_BGLProc/BGL_Processing.h>
#include <CS1_BGLProc/Package__Version.h>
#include <Transport/TH_MPI.h>

#include <boost/lexical_cast.hpp>

using namespace LOFAR;
using namespace LOFAR::CS1;

int main(int argc, char **argv)
{
  std::clog.rdbuf(std::cout.rdbuf());

  try {
    BGL_Processing::original_argv = argv;

#if defined HAVE_MPI
    TH_MPI::initMPI(argc, argv);

    if (TH_MPI::getCurrentRank() == 0)
#endif
    {
      std::string type = "brief";
      Version::show<CS1_BGLProcVersion> (std::cout, "CS1_BGLProc", type);
    }

    LocationInfo   locationInfo;

#if defined HAVE_ZOID && defined HAVE_BGL
    TH_ZoidClient  th;
#elif 0
    TH_Null	   th;
#elif 1
    usleep(10000 * locationInfo.rankInPset()); // do not connect all at the same time

    std::clog << "creating connection ..." << std::endl;
    TH_Socket	   th("127.0.0.1", boost::lexical_cast<string>(5000 + locationInfo.rankInPset()));
    std::clog << "waiting for connection ..." << std::endl;

    while (!th.init())
      sleep(1);
    std::clog << "connection successful" << std::endl;
#else
    TH_File	   th(string("/tmp/sock.") + boost::lexical_cast<string>(locationInfo.rankInPset()), TH_File::Read);

    while (!th.init())
      sleep(1);
#endif

    BGL_Processing proc(&th, locationInfo);
    BGL_Command	   command;

    do {
      command.read(&th);

      switch (command.value()) {
	case BGL_Command::PREPROCESS :	{
					  BGL_Configuration configuration;

					  configuration.read(&th);
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

#if defined HAVE_MPI
    TH_MPI::finalize();
#endif

    //abort(); // quickly release the partition
    return 0;
  } catch (Exception &ex) {
    std::cerr << "Uncaught Exception: " << ex.what() << std::endl;
    //abort(); // quickly release the partition
    return 1;
  } catch (std::exception &ex) {
    std::cerr << "Uncaught exception: " << ex.what() << std::endl;
    //abort(); // quickly release the partition
    return 1;
  }
}
