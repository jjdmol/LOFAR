/*
  The main of the API test manager
  This will copy the values from one datapoint to another.
  To do this the manager will connect to the first datapoint and
  for everey hotlink it receives it will set the second one.
  The names of both datapoints can be given in the config file.
*/

#include  <GPI_Controller.hxx>
#include  <GPI_Resources.hxx>
#include  <PVSSAPI/GCF_PVSSAPI.hxx>
#include  <signal.h>

int main(int argc, char *argv[])
{
  CharString arg;
  
  // Let Manager handle SIGINT and SIGTERM (Ctrl+C, kill)
  // Manager::sigHdl will call virtual function Manager::signalHandler 
	signal(SIGINT,  GPIController::signalHandler);
	signal(SIGTERM, GPIController::signalHandler);

  // Initialize Resources, i.e. 
  //  - interpret commandline arguments
  //  - interpret config file
  for (int i = 1; i < argc; i++)
  {
    arg = argv[i];
    if (arg.toUpper() == CharString("-PROJ") && argv[i + 1][0] != '-')
    {
      if (i + 1 >= argc) Manager::exit(-1);
      
      char buf[256];
      GPIResources::setProjectName(argv[i + 1]);
      sprintf(buf, "%s/%s/config/config", getenv("LOFARHOME"), argv[i + 1]);
      setenv("PVSS_II", buf, true);
    }
  }
	GPIResources::init(argc, argv);

  // Are we called with -helpDbg or -help ?
  if (GPIResources::getHelpDbgFlag())
  {
    GPIResources::printHelpDbg();
    return 0;
  }

  if (GPIResources::getHelpFlag())
  {
    GPIResources::printHelp();
    return 0;
  }

	GPIController* gpiController = new GPIController();

  new GCFPvssApi(gpiController);
  
  gpiController->run();

  delete gpiController;

  cerr << "Bye Bye" << endl;

  // Exit gracefully :) 
  // Call Manager::exit instead of ::exit, so we can clean up
	Manager::exit(0);


  // Just make the compilers happy...
	return 0;
}
