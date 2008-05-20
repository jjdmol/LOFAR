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
#include <tinyCEP/ApplicationHolderController.h>
#include <CS1_Interface/BGL_Command.h>
#include <CS1_Interface/BGL_Configuration.h>
#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/CS1_Parset.h>
#include <InputSection.h>
#include <AH_ION_Gather.h>
#include <BGL_Personality.h>
#include <Transport/TH_Null.h>
#include <TH_ZoidServer.h>
#include <Package__Version.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#if defined HAVE_ZOID
extern "C" {
#include <lofar.h>
}
#endif


using namespace LOFAR;
using namespace LOFAR::CS1;


static char	 **global_argv;
static unsigned  nrCoresPerPset;
static unsigned  nrInputSectionRuns;
static std::vector<TransportHolder *> clientTHs;


static void checkParset(const CS1_Parset &parset)
{
#if defined HAVE_BGLPERSONALITY
  BGLPersonality *personality = getBGLpersonality();

  if (parset.nrPsets() > personality->numPsets())
    std::cerr << "needs " << parset.nrPsets() << " psets; only " << personality->numPsets() << " available" << std::endl;
#endif

#if 0
  if (parset.sizeBeamletAndSubbandList);
    std::cerr << "size of the beamletlist must be equal to the size of subbandlist." << std::endl;

  parset.parseBeamletList();
#endif 
}


void createClientTHs(unsigned nrClients)
{
  clientTHs.resize(nrClients);

  for (unsigned core = 0; core < nrClients; core ++)
#if defined HAVE_ZOID
    clientTHs[core] = new TH_ZoidServer(core);
#else
    clientTHs[core] = new TH_Null;
#endif
}


void deleteClientTHs()
{
  for (unsigned core = 0; core < clientTHs.size(); core ++)
    delete clientTHs[core];
}


static void configureCNs(const CS1_Parset &parset)
{
  BGL_Command	    command(BGL_Command::PREPROCESS);
  BGL_Configuration configuration;

  configuration.nrStations()              = parset.nrStations();
  configuration.nrBeams()                 = parset.nrBeams();
  configuration.nrSamplesPerIntegration() = parset.BGLintegrationSteps();
  configuration.nrSamplesToBGLProc()      = parset.nrSamplesToBGLProc();
  configuration.nrUsedCoresPerPset()      = parset.nrCoresPerPset();
  configuration.nrSubbandsPerPset()       = parset.nrSubbandsPerPset();
  configuration.delayCompensation()       = parset.getBool("OLAP.delayCompensation");
  configuration.sampleRate()              = parset.sampleRate();
  configuration.inputPsets()              = parset.getUint32Vector("OLAP.BGLProc.inputPsets");
  configuration.outputPsets()             = parset.getUint32Vector("OLAP.BGLProc.outputPsets");
  configuration.refFreqs()                = parset.refFreqs();
  configuration.beamlet2beams()           = parset.beamlet2beams();
  configuration.subband2Index()           = parset.subband2Index();

  for (unsigned core = 0; core < parset.nrCoresPerPset(); core ++) {
    std::clog << "configure core " << core << std::endl;
    command.write(clientTHs[core]);
    configuration.write(clientTHs[core]);
  }
}


static void unconfigureCNs(CS1_Parset &parset)
{
  BGL_Command command(BGL_Command::POSTPROCESS);

  for (unsigned core = 0; core < parset.nrCoresPerPset(); core ++) {
    std::clog << "unconfigure core " << core << std::endl;
    command.write(clientTHs[core]);
  }
}


static void stopCNs()
{
  BGL_Command command(BGL_Command::STOP);

  for (unsigned core = 0; core < nrCoresPerPset; core ++) {
    std::clog << "stopping core " << core << std::endl;
    command.write(clientTHs[core]);
  }
}


void *input_thread(void *parset)
{
  std::clog << "starting input thread" << std::endl;

  try {
    InputSection inputSection(clientTHs);

    inputSection.preprocess(static_cast<CS1_Parset *>(parset));

    for (unsigned run = 0; run < nrInputSectionRuns; run ++)
      inputSection.process();

    inputSection.postprocess();
  } catch (Exception &ex) {
    std::cerr << "input thread caught Exception: " << ex << std::endl;
  } catch (std::exception &ex) {
    std::cerr << "input thread caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "input thread caught non-std::exception: " << std::endl;
  }

  std::clog << "input thread finished" << std::endl;
  return 0;
}


void *gather_thread(void *argv)
{
  std::clog << "starting gather thread, nrRuns = " << ((char **) argv)[2] << std::endl;

  try {
    AH_ION_Gather myAH(clientTHs);
    ApplicationHolderController myAHController(myAH, 1); //listen to ACC every 1 runs
    ACC::PLC::ACCmain(3, (char **) argv, &myAHController);
  } catch (Exception &ex) {
    std::cerr << "gather thread caught Exception: " << ex << std::endl;
  } catch (std::exception &ex) {
    std::cerr << "gather thread caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "gather thread caught non-std::exception: " << std::endl;
  }

  std::clog << "gather thread finished" << std::endl;
  return 0;
}


void *master_thread(void *)
{
  std::string type = "brief";
  Version::show<CS1_IONProcVersion> (std::clog, "CS1_IONProc", type);
  
  std::clog << "starting master_thread" << std::endl;

  try {
    pthread_t input_thread_id = 0, gather_thread_id = 0;

    std::clog << "trying to use " << global_argv[1] << " as ParameterSet" << std::endl;
    ACC::APS::ParameterSet parameterSet(global_argv[1]);
    CS1_Parset cs1_parset(&parameterSet);

    cs1_parset.adoptFile("OLAP.parset");
    checkParset(cs1_parset);

#if !defined HAVE_ZOID
    nrCoresPerPset = cs1_parset.nrCoresPerPset();
    createClientTHs(nrCoresPerPset);
#endif

    configureCNs(cs1_parset);

#if defined HAVE_BGLPERSONALITY
    unsigned myPsetNumber = getBGLpersonality()->getPsetNum();
#else
    unsigned myPsetNumber = 0;
#endif

    if (cs1_parset.inputPsetIndex(myPsetNumber) >= 0) {
#if 0
      static char nrRuns[16], *argv[] = {
	global_argv[0],
	global_argv[1],
	nrRuns,
	0
      };

      sprintf(nrRuns, "%u", atoi(global_argv[2]) * cs1_parset.nrCoresPerPset() / cs1_parset.nrSubbandsPerPset());

      if (pthread_create(&input_thread_id, 0, input_thread, argv) != 0) {
	perror("pthread_create");
	exit(1);
      }
#else
      // Passing this via a global variable is a real hack
      nrInputSectionRuns = atoi(global_argv[2]) * cs1_parset.nrCoresPerPset() / cs1_parset.nrSubbandsPerPset();

      if (pthread_create(&input_thread_id, 0, input_thread, static_cast<void *>(&cs1_parset)) != 0) {
	perror("pthread_create");
	exit(1);
      }
#endif
    }

    if (cs1_parset.useGather() && cs1_parset.outputPsetIndex(myPsetNumber) >= 0) {
      static char nrRuns[16], *argv[] = {
	global_argv[0],
	global_argv[1],
	nrRuns,
	0
      };

      sprintf(nrRuns, "%u", atoi(global_argv[2]) * cs1_parset.nrCoresPerPset());

      if (pthread_create(&gather_thread_id, 0, gather_thread, argv) != 0) {
	perror("pthread_create");
	exit(1);
      }
    }

    if (gather_thread_id != 0 && input_thread_id == 0) {
      // quick hack to send PROCESS commands to CNs

      BGL_Command command(BGL_Command::PROCESS);
      unsigned	  nrRuns  = atoi(global_argv[2]);
      unsigned	  nrCores = cs1_parset.nrCoresPerPset();

      for (unsigned run = 0; run < nrRuns; run ++)
	for (unsigned core = 0; core < nrCores; core ++)
	  command.write(clientTHs[BGL_Mapping::mapCoreOnPset(core, myPsetNumber)]);
    }

    if (input_thread_id != 0) {
      if (pthread_join(input_thread_id, 0) != 0) {
	perror("pthread join");
	exit(1);
      }

      std::clog << "lofar__fini: input thread joined" << std::endl;
    }

    unconfigureCNs(cs1_parset);
    stopCNs();

    if (gather_thread_id != 0) {
      if (pthread_join(gather_thread_id, 0) != 0) {
	perror("pthread join");
	exit(1);
      }

      std::clog << "lofar__fini: gather thread joined" << std::endl;
    }
  } catch (Exception &ex) {
    std::cerr << "main thread caught Exception: " << ex << std::endl;
  } catch (std::exception &ex) {
    std::cerr << "main thread caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "main thread caught non-std::exception: " << std::endl;
  }

  if (pthread_detach(pthread_self()) != 0) {
    std::cerr << "could not detach master thread" << std::endl;
  }

#if defined HAVE_ZOID
  if (global_argv != 0) {
    for (char **arg = &global_argv[0]; *arg != 0; arg ++)
      free(*arg);

    delete [] global_argv;
  }
#endif

  std::clog << "master thread finishes" << std::endl;
  return 0;
}


extern "C"
{
  void lofar__init(int);
  void lofar__fini(void);
}


inline static void redirect_output()
{
#if defined HAVE_BGLPERSONALITY
  int  fd;
  char file_name[64], block_id[17];
  
  getBGLpersonality()->BlockID(block_id, 16);
  block_id[16] = '\0'; // just in case it was not already '\0' terminated
 
  sprintf(file_name, "run.CS1_IONProc.%s.%u", block_id, getBGLpersonality()->getPsetNum());

  if ((fd = open(file_name, O_CREAT | O_TRUNC | O_RDWR, 0666)) < 0 || dup2(fd, 1) < 0 || dup2(fd, 2) < 0)
      perror("redirecting stdout/stderr");
#endif
}


#if defined HAVE_ZOID

void lofar__init(int nrComputeCores)
{
  nrCoresPerPset = nrComputeCores;
  redirect_output();
  std::clog << "begin of lofar__init" << std::endl;

  createClientTHs(nrComputeCores);

  global_argv = 0;
}



void lofar_init(char   **argv /* in:arr2d:size=+1 */,
		size_t * /*lengths*/ /* in:arr:size=+1 */,
		int    argc /* in:obj */)

{
  std::clog << "ok, begin of lofar_init(..., ..., " << argc << ")" << std::endl;

  for (int i = 0; i < argc; i ++)
    std::clog << "lofar_init(): arg = " << argv[i] << std::endl;

  // copy argv
  global_argv = new char * [argc + 1];

  for (int arg = 0; arg < argc; arg ++)
    global_argv[arg] = strdup(argv[arg]);

  global_argv[argc] = 0; // terminating zero pointer

  if (argc != 3) {
    std::cerr << "unexpected number of arguments" << std::endl;
    exit(1);
  }

  pthread_t master_thread_id;

  if (pthread_create(&master_thread_id, 0, master_thread, 0) != 0) {
    perror("pthread_create");
    exit(1);
  }
}


void lofar__fini(void)
{
  std::clog << "begin of lofar__fini" << std::endl;
  deleteClientTHs();
  std::clog << "end of lofar__fini" << std::endl;
}

#else

int main(int argc, char **argv)
{
  global_argv = argv;

  if (argc != 3) {
    std::cerr << "unexpected number of arguments" << std::endl;
    exit(1);
  }

  master_thread(0);
  deleteClientTHs();
  return 0;
}

#endif
