#include "../src/Helpers.h"
#include <iostream>

using namespace std;
using namespace LOFAR::Messaging;

int main()
{
  cout << "TimeOutDuration(0.5) = " << TimeOutDuration(0.5).getMilliseconds()
       << endl;
  cout << "TimeOutDuration(1.5) = " << TimeOutDuration(1.5).getMilliseconds()
       << endl;
}
