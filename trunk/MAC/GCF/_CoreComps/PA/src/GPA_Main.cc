/*
  The main of the API test manager
  This will copy the values from one datapoint to another.
  To do this the manager will connect to the first datapoint and
  for everey hotlink it receives it will set the second one.
  The names of both datapoints can be given in the config file.
*/

#include <GPA_Controller.h>
#include <TM/GCF_Control.h>

int main(int argc, char *argv[])
{
  const char* ns_file  = "tutorial";

  if (GTMNameService::instance()->init(ns_file) < 0)
  {
    cerr << "Could not open NameService configuration file: " << ns_file << endl;
    exit(1);
  }
  if (GTMTopologyService::instance()->init(ns_file) < 0)
  {
    cerr << "Could not open TopologyService configuration file: " << ns_file << endl;
    exit(1);
  }
  
  GCFTask::_argc = argc;
  GCFTask::_argv = argv;
  
  GPAController propertyAgent;  
  propertyAgent.start(); // make initial transition
  
  GCFTask::run();

  return 0;
}
