#include <PL/SQLTimeStamp.h>
#include <Common/Exception.h>

#include <iostream>
#include <iomanip>

using namespace std;

int main()
{
  SQLTimeStamp::sql_timestamp_t ts1 = {-4713,  1,  1,  0,  0,  0,         0};
  SQLTimeStamp::sql_timestamp_t ts2 = { 1970, 12, 25, 12, 59, 59, 999999999};
  SQLTimeStamp::sql_timestamp_t ts3 = { 1970, 12, 25, 13,  0,  0,         0};
  SQLTimeStamp::sql_timestamp_t ts4 = {32767, 12, 31, 23, 59, 59, 999999999};
  
  try {
    SQLTimeStamp sts1(ts1);
    SQLTimeStamp sts2(ts2);
    SQLTimeStamp sts3(ts3);
    SQLTimeStamp sts4(ts4);

    cout << endl
	 << "sts1 < sts2 = " << (sts1 < sts2 ? "true" : "false") << endl
	 << "sts2 < sts3 = " << (sts2 < sts3 ? "true" : "false") << endl
	 << "sts1 < sts3 = " << (sts1 < sts3 ? "true" : "false") << endl
	 << endl
	 << "sts2 < sts1 = " << (sts2 < sts1 ? "true" : "false") << endl
	 << "sts3 < sts2 = " << (sts3 < sts2 ? "true" : "false") << endl
	 << "sts3 < sts1 = " << (sts3 < sts1 ? "true" : "false") << endl
	 << endl
	 << "sts1 < sts1 = " << (sts1 < sts1 ? "true" : "false") << endl
	 << "sts2 < sts2 = " << (sts2 < sts2 ? "true" : "false") << endl
	 << "sts3 < sts3 = " << (sts3 < sts3 ? "true" : "false") << endl
	 << endl;

    Assert(sts3 < sts4);

  }
  catch (LCS::Exception& e) {
    cerr << e << endl;
  }

  return 0;
}
