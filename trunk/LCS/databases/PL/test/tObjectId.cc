#include <PL/ObjectId.h>
#include <iostream>
#include <iomanip>

#include <sys/time.h>

using namespace std;
using namespace LOFAR::PL;

int main()
{
  ObjectId oid1;
  ObjectId oid2;
  cout << hex << setfill('0');
  oid1.get();
  cout << endl << "oid1.get() = >>>0x" << oid1.get() << "<<<";
  cout << endl << "oid2.get() = >>>0x" << oid2.get() << "<<<";
  cout << endl << "(oid1 == oid2) = " << (oid1 == oid2 ? "true" : "false") 
       << endl;

  cout << endl << "Assiging oid1 to oid2...";
  oid2.set(oid1.get());
  cout << endl << "oid2.get() = >>>0x" << oid2.get() << "<<<";
  cout << endl << "(oid1 == oid2) = " << (oid1 == oid2 ? "true" : "false") 
       << endl;

  cout << endl << "Making oid1 and oid2 equal to global null object-id ...";
  oid1.set(0);
  oid2.set(0);
  cout << endl << "oid1.get() = 0x" << oid1.get();
  cout << endl << "oid2.get() = 0x" << oid2.get();
  cout << endl << "(oid1 == oid2) = " << (oid1 == oid2 ? "true" : "false") 
       << endl;

  timeval tv_start, tv_end;
  double sec;
  cout << endl << "Generating 1.000.000 uninitialized Object-IDs ...";
  gettimeofday(&tv_start,0);
  for (int i=0; i<1000000; i++) {
    ObjectId(0).get();
  }
  gettimeofday(&tv_end,0);
  sec = (tv_end.tv_sec - tv_start.tv_sec) + 
        (tv_end.tv_usec - tv_start.tv_usec) * 1e-6;
  cout << endl << "Elapsed time: >>>" << sec << "<<< seconds." << endl;

  cout << endl << "Generating 1.000.000 initialized Object-IDs ...";
  gettimeofday(&tv_start,0);
  for (int i=0; i<1000000; i++) {
    ObjectId().get();
  }
  gettimeofday(&tv_end,0);
  sec = (tv_end.tv_sec - tv_start.tv_sec) + 
        (tv_end.tv_usec - tv_start.tv_usec) * 1e-6;
  cout << endl << "Elapsed time: >>>" << sec << "<<< seconds." << endl;

  return 0;
}
