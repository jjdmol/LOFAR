#include <PL/ObjectId.h>
#include <Common/Trace.h>
#include <iostream>
#include <iomanip>

#include <sys/time.h>

using namespace std;
using namespace LCS::PL;

int main()
{
  ObjectId oid1;
  ObjectId oid2;
  cout << hex << setfill('0');
  oid1.get();
  cout << "oid1.get() = 0x" << oid1.get() << endl;
  cout << "oid2.get() = 0x" << oid2.get() << endl;
  cout << "Assiging oid1 to oid2..." << endl;
  oid2.set(oid1.get());
  cout << "oid2.get() = 0x" << oid2.get() << endl;

  timeval tv_start, tv_end;
  double sec;
  cout << "Generating 1.000.000 uninitialized Object-IDs ..." << endl;
  gettimeofday(&tv_start,0);
  for (int i=0; i<1000000; i++) {
    ObjectId();
  }
  gettimeofday(&tv_end,0);
  sec = (tv_end.tv_sec - tv_start.tv_sec) + 
        (tv_end.tv_usec - tv_start.tv_usec) * 1e-6;
  cout << "Elapsed time: " << sec << " seconds." << endl;

  cout << "Generating 1.000.000 initialized Object-IDs ..." << endl;
  gettimeofday(&tv_start,0);
  for (int i=0; i<1000000; i++) {
    ObjectId().get();
  }
  gettimeofday(&tv_end,0);
  sec = (tv_end.tv_sec - tv_start.tv_sec) + 
        (tv_end.tv_usec - tv_start.tv_usec) * 1e-6;
  cout << "Elapsed time: " << sec << " seconds." << endl;

  return 0;
}
