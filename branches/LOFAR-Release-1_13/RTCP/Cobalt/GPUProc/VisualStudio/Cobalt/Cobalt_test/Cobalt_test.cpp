// Cobalt_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

#include <UnitTest++.h>


TEST(SelfTest)
  {
    CHECK(true);
  }

int _tmain(int argc, _TCHAR* argv[])
{
    std::cout<<"Hello Test" << std::endl;
    return UnitTest::RunAllTests();
}

