#include <lofar_config.h>
#include <BBSControl/pqutil.h>
#include <APS/ParameterSet.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace LOFAR::ACC::APS;

#if 0
void toPQ(ostream& os, const ParameterSet& ps)
{
}

void toPS(ostream& os, ParameterSet& ps)
{
}
#endif

int main()
{
  INIT_LOGGER("tutil");
  ParameterSet ps("tpqutil.parset"); 
  string s1(ps.getString("s1"));
  vector<string> s2(ps.getStringVector("s2"));
  vector<string> s3(ps.getStringVector("s3"));
  string s4(ps.getString("s4"));
  vector<string> s5(ps.getStringVector("s5"));
  string s6(ps.getString("s6"));

//   cout << "s1 = " << s1 << endl;
//   cout << "s2 [" << s2.size() << "] = " << endl;
//   for (uint i = 0; i < s2.size(); ++i) cout << "  " << s2[i] << endl;
//   cout << "s3 [" << s3.size() << "] = " << endl;
//   for (uint i = 0; i < s3.size(); ++i) cout << "  " << s3[i] << endl;
//   cout << "s4 = " << s4 << endl;
//   cout << "s5 [" << s5.size() << "] = " << endl;
//   for (uint i = 0; i < s5.size(); ++i) cout << "  " << s5[i] << endl;
//   cout << "s6 = " << s6 << endl;

  string s;
  cout << "\nOriginal:" << endl;
  ps.writeBuffer(s);
  cout << s << endl;

  cout << "\nAs PSQL string:" << endl;
  s = toPQstring(s);
  cout << s << endl;

  cout << "\nAs ParameterSet string:" << endl;
  s = toPSstring(s.c_str());
  cout << s << endl;

  return 0;
#if 1

  // \note All backslashes must be doubled in the string initialization,
  // because a backslash should be properly escaped using a backslash.
  try {
    cout << toPSstring("Hello, World") << endl;
    cout << toPSstring("{Hello, World, Array}") << endl;
    cout << toPSstring("{Curly braces, '{' and '}', must be quoted,") 
	 << toPSstring(" or escaped like this: \\{ and \\}}") << endl;
    cout << toPSstring("{Unbalanced braces do not throw") << endl;
    cout << toPSstring("Properly escaped quote: \\',")
	 << toPSstring(" and another: \\\\\\'") << endl;
    cout << toPSstring("Trailing backslash \\") << endl;
  }
  catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }

  try {
    cout << toPSstring("Missing closing single quote ' will throw") << endl;
    return 1;   // should not get here
  }
  catch (Exception& e) {
    cout << "(Expected): " << e.what() << endl;
  }

  try {
    cout << toPSstring("Missing closing double quote \" will throw") << endl;
    return 1;   // should not get here
  }
  catch (Exception& e) {
    cout << "(Expected): " << e.what() << endl;
  }

  try {
    cout << toPSstring("Improperly escaped quote \\\\' will throw") << endl;
    return 1;   // should not get here
  }
  catch (Exception& e) {
    cout << "(Expected): " << e.what() << endl;
  }

#else

  try {
    cout << toPQstring("Hello, World") << endl;
    cout << toPQstring("[Hello, World, Array]") << endl;
    cout << toPQstring("Curly braces, \"{\" and \"}\", must be quoted,") 
 	 << toPQstring(" or escaped like this: \\{ and \\}") << endl;
    cout << toPQstring("[Unbalanced brackets do not throw") << endl;
    cout << toPQstring("Properly escaped quote: \\',")
 	 << toPQstring(" and another: \\\\\\'") << endl;
    cout << toPQstring("Trailing backslash \\") << endl;
  }
  catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }

#endif

  return 0;
}
