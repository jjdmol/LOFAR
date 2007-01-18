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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_set.h>
#include <Common/lofar_iostream.h>
#include <cmath>
#include <limits>

using namespace LOFAR;
using namespace std;


#define TOSTRING_TEST(type,min,max) \
{ \
  cout << "min(" #type ") = " << toString(min) \
       << "\t max(" #type ") = " << toString(max) << endl; \
}

#define TOSTRING_TEST2(type, val, format) \
{ \
  type x(val); \
  cout << "toString(" #type "(" #val ")," #format ") = " \
       << toString(x, format) << endl; \
}


bool testSplit()
{
  string s(",aa,bb,,dd,");
  vector<string> vs = StringUtil::split(s,',');
  cout << "Splitting string \"" << s << "\" using \',\' as seperator ..." 
       << endl;
  for (string::size_type i = 0; i < vs.size(); i++) {
    cout << "vs[" << i << "] = \"" << vs[i] << "\"" << endl;
  }
  cout << endl;
  return true;
}


bool testCompare(bool nocase)
{
  set<string, StringUtil::Compare> 
    s(nocase ? StringUtil::Compare::NOCASE : StringUtil::Compare::NORMAL);
  s.insert("Alfons");
  s.insert("aap");
  s.insert("Bertus");
  s.insert("bakker");
  s.insert("Chris");
  s.insert("chocolade");
  if (nocase) cout << "=== Strings sorted NOCASE ===" << endl;
  else cout << "=== Strings sorted NORMAL ===" << endl;
  for (set<string>::const_iterator it = s.begin(); it != s.end(); ++it) {
    cout << *it << endl;
  }
  cout << endl;
  return true;
}


bool testTrim()
{
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

  return true;
}


bool testCase()
{
  cout << "Lowercase and uppercase a string:" << endl;
  string s("The zip code of Hondelage in Germany is 38108");
  cout << "orginal: " << s << endl;
  cout << "lowered: " << toLower(s) << endl;
  cout << "uppered: " << toUpper(s) << endl;
  return true;
}


bool testToString()
{
  //## Conversion of fundamental arithmetic types to string ##//

  cout << "\n*** Conversion of fundamental arithmetic types to string ***\n";
  try {
    TOSTRING_TEST(bool,false,true);
    TOSTRING_TEST(char,0,127);
    TOSTRING_TEST(signed char,-128,127);
    TOSTRING_TEST(unsigned char,0,255);
    TOSTRING_TEST(short,-32768,32767);
    TOSTRING_TEST(unsigned short,0,65535);
    TOSTRING_TEST(int,-32768,32767);
    TOSTRING_TEST(unsigned int,0,65536);
    TOSTRING_TEST(long,~0x7FFFFFFFL,0x7FFFFFFFL);
    TOSTRING_TEST(unsigned long,0,0xFFFFFFFFUL);
#ifdef HAVE_LONG_LONG
    TOSTRING_TEST(long long,~0x7FFFFFFFFFFFFFFFLL,0x7FFFFFFFFFFFFFFFLL);
    TOSTRING_TEST(unsigned long long,0,0xFFFFFFFFFFFFFFFFULL);
#endif
    TOSTRING_TEST(float,numeric_limits<float>::min(),numeric_limits<float>::max());
    TOSTRING_TEST(double,numeric_limits<double>::min(), numeric_limits<double>::max());
#ifdef HAVE_LONG_DOUBLE
    TOSTRING_TEST(long double,numeric_limits<long double>::min(), numeric_limits<long double>::max());
#endif
    cout << endl;
    TOSTRING_TEST2(int, 42, "%06i");
    TOSTRING_TEST2(float, M_E, "%e");
    TOSTRING_TEST2(float, M_PI, "%8.4f");
    TOSTRING_TEST2(double, M_E, "%+08.12g");
    TOSTRING_TEST2(double, M_PI*1e12, "%+08.12g");
  } catch (Exception& e) {
    LOG_ERROR_STR(e);
    return false;
  }
  return true;
}


int main()
{
  INIT_LOGGER("tStringUtil");

  bool result = 
    testSplit()        &&
    testCompare(false) &&
    testCompare(true)  &&
    testTrim()         &&
    testCase()         &&
    testToString();

  return (result ? 0 : 1);
}
