#include <lofar_config.h>
#include <BBSControl/util.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

int main()
{
  INIT_LOGGER("tutil");

  // \note All backslashes must be doubled in the string initialization,
  // because a backslash should be properly escaped using a backslash.
  try {
    cout << toPSstring("Hello, World") << endl;
    cout << toPSstring("{Hello, World, Array}") << endl;
    cout << toPSstring("Curly braces, '{' and '}', must be quoted,") 
	 << toPSstring(" or escaped like this: \\{ and \\}") << endl;
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

  return 0;
}
