#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactory.h>

#include "SQLTimeStampTest.h"

int main()
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
  runner.run();
  return 0;
}
