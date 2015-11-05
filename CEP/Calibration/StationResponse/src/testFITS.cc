#include <lofar_config.h>
#include <iostream>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/OS/File.h>
#include <casa/Containers/OrderedMap.h>
#include <fits/FITS/BasicFITS.h>

using namespace std;
using namespace casa;

int main(int argc, char *argv[])
{

  // Get the name of the parset file.
  if (argc<2) {
    cout<<"Usage: testFITS FITSFILE"<<endl;
    return 0;
  }

  Bool ok=true;
  String message;

  Array<Float> m;

  m = ReadFITS(argv[1],ok,message);
  if (!ok) {
    cout << "Read failed: " << message << endl;
  }



  cout<<m.shape()<<endl;
  return 0;
}

