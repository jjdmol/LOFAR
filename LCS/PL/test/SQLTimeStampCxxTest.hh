#include <cxxtest/TestSuite.h>

#include <PL/SQLTimeStamp.h>
#include <Common/Debug.h>

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

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


class SQLTimeStampCxxTest : public CxxTest::TestSuite
{
public:
  void setUp()
  {
    // Julian day: 0
    const SQLTimeStamp::sql_timestamp_t ts1 = 
      {-4713,  1,  1,  0,  0,  0,         0};
    sts1 = SQLTimeStamp(ts1);
    
    // 1 ns before 25 Dec 1970 13h00m00s
    const SQLTimeStamp::sql_timestamp_t ts2 = 
      { 1970, 12, 25, 12, 59, 59, 999999999};
    sts2 = SQLTimeStamp(ts2);
  
    // 25 Dec 1970 13h00m00s
    const SQLTimeStamp::sql_timestamp_t ts3 =
      { 1970, 12, 25, 13,  0,  0,         0};
    sts3 = SQLTimeStamp(ts3);

    // 31 Dec 32768 23h00m00s (year field will overflow!)
    const SQLTimeStamp::sql_timestamp_t ts4 =
      {32768, 12, 31, 23,  0,  0,         0};
    sts4 = SQLTimeStamp(ts4);
  }

  void tearDown()
  {
  }

  void testConstructorThrows()
  {
    // 32 Dec 1970 23h60m00s (invalid day and min field)
    const SQLTimeStamp::sql_timestamp_t tsf = 
      { 1970, 12, 32, 23, 60,  0,         0};
    // must throw LCS::Assert exception
    TS_ASSERT_THROWS( SQLTimeStamp stsf(tsf), LCS::AssertError );
  }

  void testLessThanComparison()
  {
    TS_ASSERT(sts1 < sts2);
    TS_ASSERT(sts2 < sts3);
    TS_ASSERT(sts1 < sts3);
  }


  void testLessThanComparisonFails()
  {
    TS_ASSERT(!(sts1 <sts1));  // should ALWAYS fail!
    TS_ASSERT(!(sts1 <sts4));  // demonstrates overflow of year field
  }

private:
  SQLTimeStamp sts1;
  SQLTimeStamp sts2;
  SQLTimeStamp sts3;
  SQLTimeStamp sts4;
  SQLTimeStamp stsf;
};
