#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#include "SQLTimeStampTest.h"

#include <iostream>
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(SQLTimeStampTest);

//
// Initialization of local static data
//

// Julian day: 0
const SQLTimeStamp::sql_timestamp_t ts1 = 
  {-4713,  1,  1,  0,  0,  0,         0};

// 1 ns before 25 Dec 1970 13h00m00s
const SQLTimeStamp::sql_timestamp_t ts2 = 
  { 1970, 12, 25, 12, 59, 59, 999999999};
  
// 25 Dec 1970 13h00m00s
const SQLTimeStamp::sql_timestamp_t ts3 =
  { 1970, 12, 25, 13,  0,  0,         0};

// 31 Dec 32768 23h00m00s (year field will overflow!)
const SQLTimeStamp::sql_timestamp_t ts4 =
  {32768, 12, 31, 23,  0,  0,         0};

// 32 Dec 1970 23h60m00s (invalid day and min field)
const SQLTimeStamp::sql_timestamp_t tsf = 
  { 1970, 12, 32, 23, 60,  0,         0};


void SQLTimeStampTest::setUp()
{
  sts1 = SQLTimeStamp(ts1);
  sts2 = SQLTimeStamp(ts2);
  sts3 = SQLTimeStamp(ts3);
  sts4 = SQLTimeStamp(ts4);
}

void SQLTimeStampTest::tearDown()
{
}

void SQLTimeStampTest::testConstructorThrows() 
{
  stsf = SQLTimeStamp(tsf);  // must throw LCS::AssertError
}

void SQLTimeStampTest::testLessThanComparison()
{
  CPPUNIT_ASSERT(sts1 < sts2);
  CPPUNIT_ASSERT(sts2 < sts3);
  CPPUNIT_ASSERT(sts2 < sts3);
}

void SQLTimeStampTest::testLessThanIsNotEqual()
{
  CPPUNIT_ASSERT(sts1 < sts1);  // should ALWAYS fail!
}

void SQLTimeStampTest::testShouldOverflow()
{
  CPPUNIT_ASSERT(sts1 < sts4);  // demonstrates overflow of year field
}
