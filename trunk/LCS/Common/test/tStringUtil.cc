//#  tStringUtil.cc: test program for the string utilities class
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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

#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <cmath>
#include <iostream>
#include <limits>

using namespace LOFAR;
using namespace std;

#define TOSTRING_TEST(type) \
{ \
  type min(numeric_limits<type>::min()); \
  type max(numeric_limits<type>::max()); \
  ASSERT(min == numeric_limits<type>::min() && \
         max == numeric_limits<type>::max()); \
  cout << "min(" #type ") = " << toString(min) \
       << "\t max(" #type ") = " << toString(max) << endl; \
}

#define TOSTRING_TEST2(type, val, format) \
{ \
  type x(val); \
  cout << "toString(" #type "(" #val ")," #format ") = " \
       << toString(x, format) << endl; \
}

int main()
{
  INIT_LOGGER("tStringUtil");
  string s(",aa,bb,,dd,");
  vector<string> vs = StringUtil::split(s,',');
  cout << "Splitting string \"" << s << "\" using \',\' as seperator ..." 
       << endl;
  for (string::size_type i = 0; i < vs.size(); i++) {
    cout << "vs[" << i << "] = \"" << vs[i] << "\"" << endl;
  }

  string r1(" 	 	 a String with leading and trailing whitespace	 ");
  string r2("a String without leading and trailing whitespace");
  string r3("1) String with(out) itemnr and arrayindex[123]");

  rtrim(r1);
  ltrim(r1);
  rtrim(r2);
  ltrim(r2);
  rtrim(r3, "0123456789[] ");
  ltrim(r3, "0123456789() ");

  cout << "Trimmed C++ strings:" << endl;
  cout << ">" << r1 << "<" << endl;
  cout << ">" << r2 << "<" << endl;
  cout << ">" << r3 << "<" << endl;

  char c1[] = { " 	 	 a String with leading and trailing whitespace	 " };
  char c2[] = { "a String without leading and trailing whitespace" };
  char c3[] = { "1) String with(out) itemnr and arrayindex[123]" };
  char* p1 = c1;
  char* p2 = c2;
  char* p3 = c3;
  int32 len1, len2, len3;

  len1 = rtrim(p1 = ltrim(p1));
  len2 = rtrim(p2 = ltrim(p2));
  p3 = ltrim(p3, "0123456789() ");
  len3 = rtrim(p3 = ltrim(p3, "0123456789() ") , 0, "0123456789[] ");

  cout << "Trimmed C strings:" << endl;
  cout << ">" << p1 << "< , len=" << len1 << endl;
  cout << ">" << p2 << "< , len=" << len2 << endl;
  cout << ">" << p3 << "< , len=" << len3 << endl;


  //## Conversion of fundamental arithmetic types to string ##//

  cout << "\n*** Conversion of fundamental arithmetic types to string ***\n";
  try {
    TOSTRING_TEST(bool);
    TOSTRING_TEST(char);
    TOSTRING_TEST(unsigned char);
    TOSTRING_TEST(short);
    TOSTRING_TEST(unsigned short);
    TOSTRING_TEST(int);
    TOSTRING_TEST(unsigned int);
    TOSTRING_TEST(long);
    TOSTRING_TEST(unsigned long);
#ifdef HAVE_LONG_LONG
    TOSTRING_TEST(long long);
    TOSTRING_TEST(unsigned long long);
#endif
    TOSTRING_TEST(float);
    TOSTRING_TEST(double);
#ifdef HAVE_LONG_DOUBLE
    TOSTRING_TEST(long double);
#endif
    cout << endl;
    TOSTRING_TEST2(int, 42, "%06i");
    TOSTRING_TEST2(float, M_E, "%e");
    TOSTRING_TEST2(float, M_PI, "%8.4f");
    TOSTRING_TEST2(double, M_E, "%+08.12g");
    TOSTRING_TEST2(double, M_PI*1e12, "%+08.12g");
  } catch (Exception& e) {
    LOG_ERROR_STR(e);
  }

  return 0;
}
