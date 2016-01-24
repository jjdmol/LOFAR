//# tQueue.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <CoInterface/Align.h>

#include <UnitTest++.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

TEST(powerOfTwo)
{
  CHECK_EQUAL(false, powerOfTwo(0));
  CHECK_EQUAL(true,  powerOfTwo(1));
  CHECK_EQUAL(true,  powerOfTwo(2));
  CHECK_EQUAL(false, powerOfTwo(3));
  CHECK_EQUAL(true,  powerOfTwo(4));
  CHECK_EQUAL(false, powerOfTwo(5));
  CHECK_EQUAL(false, powerOfTwo(6));
  CHECK_EQUAL(false, powerOfTwo(7));
  CHECK_EQUAL(true,  powerOfTwo(8));
  CHECK_EQUAL(false, powerOfTwo(9));
}


TEST(roundUpToPowerOfTwo)
{
  CHECK_EQUAL(1,  roundUpToPowerOfTwo(0));
  CHECK_EQUAL(1,  roundUpToPowerOfTwo(1));
  CHECK_EQUAL(2,  roundUpToPowerOfTwo(2));
  CHECK_EQUAL(4,  roundUpToPowerOfTwo(3));
  CHECK_EQUAL(4,  roundUpToPowerOfTwo(4));
  CHECK_EQUAL(8,  roundUpToPowerOfTwo(5));
  CHECK_EQUAL(8,  roundUpToPowerOfTwo(6));
  CHECK_EQUAL(8,  roundUpToPowerOfTwo(7));
  CHECK_EQUAL(8,  roundUpToPowerOfTwo(8));
  CHECK_EQUAL(16, roundUpToPowerOfTwo(9));
}


TEST(log2)
{
  CHECK_EQUAL(0U, log2(1U));
  CHECK_EQUAL(1U, log2(2U));
  CHECK_EQUAL(2U, log2(4U));
  CHECK_EQUAL(3U, log2(8U));
  CHECK_EQUAL(4U, log2(16U));
  CHECK_EQUAL(5U, log2(32U));
  CHECK_EQUAL(6U, log2(64U));
  CHECK_EQUAL(7U, log2(128U));
}


int main(void)
{
  INIT_LOGGER("tAlign");

  return UnitTest::RunAllTests() > 0;
}

