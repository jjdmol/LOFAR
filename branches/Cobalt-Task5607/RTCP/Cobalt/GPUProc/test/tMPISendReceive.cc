#include <lofar_config.h>

#include <iostream>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


//using namespace LOFAR;
//using namespace LOFAR::Cobalt;
using namespace std;

static void usage(const char *argv0)
{
  cerr << "RTCP: Real-Time Central Processing of the LOFAR radio telescope." << endl;
  cerr << "RTCP provides correlation for the Standard Imaging mode and" << endl;
  cerr << "beam-forming for the Pulsar mode." << endl;
  // one of the roll-out scripts greps for the version x.y

  cerr << endl;
  cerr << "Usage: " << argv0 << " parset" << " [-p]" << endl;
  cerr << endl;
  cerr << "  -h: print this message" << endl;
}

main(int argc, char **argv)
{
  std::cout << "Hello World!";


  /*
  * Parse command-line options
  */

  int opt;
  while ((opt = getopt(argc, argv, "ph")) != -1) {
    switch (opt) {

    case 'h':
      usage(argv[0]);
      exit(0);

    default: /* '?' */
      usage(argv[0]);
      exit(1);
    }
  }

  // we expect a parset filename as an additional parameter
  if (optind >= argc) {
    usage(argv[0]);
    exit(1);
  }


  return 0;
}