//#  CS1_ION_main.cc:
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

#include <PLC/ACCmain.h>
#include <Common/LofarLogger.h>
#include <tinyCEP/ApplicationHolderController.h>
#include <CS1_IONProc/AH_ION_Scatter.h>
#include <CS1_IONProc/AH_ION_Gather.h>
#include <CS1_IONProc/TH_ZoidServer.h>

#include <pthread.h>

extern "C" {
#include <lofar.h>
}


using namespace LOFAR;
using namespace LOFAR::CS1;


static int  global_argc;
static char **global_argv;

#if 0
static char *argv[] = {
  "CS1_ION_Gather",
  "9999999",	// FIXME get this from real argv
  0,
};
#endif


void *scatter_thread(void *)
{
  INIT_LOGGER("CS1_ION_Scatter");

  std::clog << "starting scatter_thread" << std::endl;
  AH_ION_Scatter myAH;
  ApplicationHolderController myAHController(myAH, 1); //listen to ACC every 1 runs
  ACC::PLC::ACCmain(global_argc, global_argv, &myAHController);
  //ACC::PLC::ACCmain(sizeof argv / sizeof *argv - 1, argv, &myAHController);
  return 0;
}


void *gather_thread(void *)
{
  INIT_LOGGER("CS1_ION_Gather");

  std::clog << "starting gather_thread" << std::endl;
  AH_ION_Gather myAH;
  ApplicationHolderController myAHController(myAH, 1); //listen to ACC every 1 runs
  ACC::PLC::ACCmain(global_argc, global_argv, &myAHController);
  //ACC::PLC::ACCmain(sizeof argv / sizeof *argv - 1, argv, &myAHController);
  return 0;
}


extern "C"
{
  void lofar__init(int);
  void lofar__fini(void);
}

static pthread_t scatter_thread_id, gather_thread_id;

void lofar__init(int nrComputeNodes)
{
  std::clog << "begin of lofar__init" << std::endl;

  TH_ZoidServer::createAllTH_ZoidServers(nrComputeNodes);

  global_argv	    = 0;
  scatter_thread_id = 0;
  gather_thread_id  = 0;
}


void lofar_init(char   **argv /* in:arr2d:size=+1 */,
		size_t *lengths /* in:arr:size=+1 */,
		int    argc /* in:obj */)

{
  std::clog << "ok, begin of lofar_init(..., ..., " << argc << ")" << std::endl;

  for (int i = 0; i < argc; i ++)
    std::clog << "lofar_init(): arg = " << argv[i] << std::endl;

  // copy argv
  global_argc = argc;
  global_argv = new char * [argc + 1];

  for (int arg = 0; arg < argc; arg ++) {
    global_argv[arg] = new char[lengths[arg]];
    memcpy(global_argv[arg], argv[arg], lengths[arg]);
  }

  global_argv[argc] = 0; // terminating zero pointer

  try {
    std::string fileName = std::string(basename(argv[0])) + ".parset";
    std::clog << "trying to use " << fileName << " as ParameterSet" << std::endl;
    ACC::APS::ParameterSet parameterSet(fileName);

    if (parameterSet.getBool("OLAP.IONProc.useScatter")) {
      if (pthread_create(&scatter_thread_id, 0, scatter_thread, 0) != 0) {
	perror("pthread_create");
	exit(1);
      }
    }

    if (parameterSet.getBool("OLAP.IONProc.useGather")) {
      if (pthread_create(&gather_thread_id, 0, gather_thread, 0) != 0) {
	perror("pthread_create");
	exit(1);
      }
    }
  } catch (std::exception &ex) {
    std::cerr << "caught exception: " << ex.what() << std::endl;
  }
}


void lofar__fini(void)
{
  std::clog << "begin of lofar__fini" << std::endl;

  if (scatter_thread_id != 0) {
    if (pthread_join(scatter_thread_id, 0) != 0) {
      perror("pthread join");
      exit(1);
    }
  }

  if (gather_thread_id != 0) {
    if (pthread_join(gather_thread_id, 0) != 0) {
      perror("pthread join");
      exit(1);
    }
  }

  TH_ZoidServer::deleteAllTH_ZoidServers();

  if (global_argv != 0) {
    for (int arg = 0; arg < global_argc; arg ++)
      delete global_argv[arg];

    delete global_argv;
  }

  std::clog << "end of lofar__fini" << std::endl;
}
