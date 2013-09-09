//# tNumeric.cc: test program for the Numeric class
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/Numeric.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_iostream.h>
#include <limits>
#include <cmath>

using namespace LOFAR;
using namespace std;

#define initNumbers(T)                                               \
  LOG_INFO("initNumbers("#T")");                                     \
  typedef Numeric::T##Mask_t mask_t;                                 \
  typedef Numeric::T##Union_t union_t;                               \
  mask_t negmask = mask_t(1) << (8*sizeof(T)-1);                     \
  ASSERT(sizeof(T) == sizeof(mask_t));				     \
  T zero(0), one(1), two(2);                                         \
  /* Create a negative zero                              */          \
  union_t negativeZero_u;                                            \
  negativeZero_u.mask = negmask;                                     \
  T negativeZero(negativeZero_u.value);                              \
  /* Create an infinity */                                           \
  T inf(one/zero);                                                   \
  /* Create a NaN */                                                 \
  T nan(zero/zero);                                                  \
  /* Create a number of different NANs - they should    */           \
  /* all be the same on Intel chips.                    */           \
  T nan1(inf-inf);                                                   \
  T nan2(inf/inf);                                                   \
  T nan3(inf*zero);                                                  \
  T nan4(sqrt(-one));                                                \
  /* Copy one of the NANs and modify its representation. */          \
  /* This will still give a NAN, just a different one.   */          \
  union_t nan5_u = { nan1 };                                         \
  nan5_u.mask += 1;                                                  \
  T nan5(nan5_u.value);                                              \
  /* Create number nearest to 2 (1 ULP below)            */          \
  union_t nearestTwo_u = { two };                                    \
  nearestTwo_u.mask -= 1;                                            \
  T nearestTwo(nearestTwo_u.value);                                  \
  /* Create a number near to 2 (sizeof(T) ULPs below)     */         \
  union_t nearTwo_u = { two };                                       \
  nearTwo_u.mask -= sizeof(T);                                       \
  T nearTwo(nearTwo_u.value);                                        \
  /* Create a number not so near to 2 (sizeof(T)^2 ULPs below) */    \
  union_t notNearTwo_u = { two };                                    \
  notNearTwo_u.mask -= sizeof(T)*sizeof(T);                          \
  T notNearTwo(notNearTwo_u.value);                                  \
  /* Create a denormal by starting with zero and         */          \
  /* incrementing the integer representation.            */          \
  union_t smallestDenormal_u = { zero };                             \
  smallestDenormal_u.mask += 1;                                      \
  T smallestDenormal(smallestDenormal_u.value);

#define printNumber(x)                                               \
{ int p(2*sizeof(x)+1);                                              \
  union_t u = { x };                                                 \
  LOG_DEBUG_STR(setprecision(p) << left << setw(17) << #x << " = "   \
                << setw(p+6) << x << " (" << hex << showbase         \
                << setw(p+1) << u.mask << dec << ")");               \
}

#define showNumbers(T)                                               \
{ LOG_INFO("showNumbers("#T")");                                     \
  printNumber(zero);                                                 \
  printNumber(one);                                                  \
  printNumber(two);                                                  \
  printNumber(negativeZero);                                         \
  printNumber(nan);                                                  \
  printNumber(nan1);                                                 \
  printNumber(nan2);                                                 \
  printNumber(nan3);                                                 \
  printNumber(nan4);                                                 \
  printNumber(nan5);                                                 \
  printNumber(nearestTwo);                                           \
  printNumber(nearTwo);                                              \
  printNumber(notNearTwo);                                           \
  printNumber(smallestDenormal);                                     \
}

#define testNegative(T)                                              \
{ LOG_INFO("testNegative("#T")");                                    \
  ASSERT(!Numeric::isNegative(zero));                                \
  ASSERT(Numeric::isNegative(negativeZero));                         \
  ASSERT(!Numeric::isNegative(one));                                 \
  ASSERT(Numeric::isNegative(-one));                                 \
  ASSERT(!Numeric::isNegative(inf));                                 \
  ASSERT(Numeric::isNegative(-inf));                                 \
}

#define testFinite(T)                                                \
{ LOG_INFO("testFinite("#T")");                                      \
  ASSERT(Numeric::isFinite(zero));                                   \
  ASSERT(Numeric::isFinite(negativeZero));                           \
  ASSERT(Numeric::isFinite(one));                                    \
  ASSERT(Numeric::isFinite(-one));                                   \
  ASSERT(!Numeric::isFinite(inf));                                   \
  ASSERT(!Numeric::isFinite(-inf));                                  \
  ASSERT(!Numeric::isFinite(nan));                                   \
  ASSERT(!Numeric::isFinite(nan1));                                  \
  ASSERT(!Numeric::isFinite(nan2));                                  \
  ASSERT(!Numeric::isFinite(nan3));                                  \
  ASSERT(!Numeric::isFinite(nan4));                                  \
  ASSERT(!Numeric::isFinite(nan5));                                  \
}

#define testInf(T)                                                   \
{ LOG_INFO("testInf("#T")");                                         \
  ASSERT(!Numeric::isInf(zero));                                     \
  ASSERT(!Numeric::isInf(one));                                      \
  ASSERT(!Numeric::isInf(-one));                                     \
  ASSERT(Numeric::isInf(inf));                                       \
  ASSERT(Numeric::isInf(-inf));                                      \
  ASSERT(!Numeric::isInf(nan));                                      \
  ASSERT(!Numeric::isInf(nan1));                                     \
  ASSERT(!Numeric::isInf(nan2));                                     \
  ASSERT(!Numeric::isInf(nan3));                                     \
  ASSERT(!Numeric::isInf(nan4));                                     \
  ASSERT(!Numeric::isInf(nan5));                                     \
  ASSERT(one/inf == 0);                                              \
  ASSERT(one/(-inf) == 0);                                           \
  ASSERT((-one)/inf == 0);                                           \
  ASSERT((-one)/(-inf) == 0);                                        \
  ASSERT(Numeric::isInf(inf*inf));                                   \
  ASSERT(Numeric::isInf(inf*(-inf)));                                \
  ASSERT(Numeric::isInf((-inf)*inf));                                \
  ASSERT(Numeric::isInf((-inf)*(-inf)));                             \
  ASSERT(Numeric::isInf(inf+inf));                                   \
  ASSERT(Numeric::isInf((-inf)+(-inf)));                             \
}

#define testNan(T)                                                   \
{ LOG_INFO("testNan("#T")");                                         \
  ASSERT(!Numeric::isNan(zero));                                     \
  ASSERT(!Numeric::isNan(one));                                      \
  ASSERT(!Numeric::isNan(-one));                                     \
  ASSERT(!Numeric::isNan(inf));                                      \
  ASSERT(!Numeric::isNan(-inf));                                     \
  ASSERT(Numeric::isNan(zero/zero));                                 \
  ASSERT(Numeric::isNan(zero/negativeZero));                         \
  ASSERT(Numeric::isNan(negativeZero/zero));                         \
  ASSERT(Numeric::isNan(negativeZero/negativeZero));                 \
  ASSERT(Numeric::isNan(inf-inf));                                   \
  ASSERT(Numeric::isNan(inf/inf));                                   \
  ASSERT(Numeric::isNan(inf/(-inf)));                                \
  ASSERT(Numeric::isNan((-inf)/inf));                                \
  ASSERT(Numeric::isNan((-inf)/(-inf)));                             \
  ASSERT(Numeric::isNan(inf*zero));                                  \
  ASSERT(Numeric::isNan((-inf)*zero));                               \
  ASSERT(Numeric::isNan(inf*negativeZero));                          \
  ASSERT(Numeric::isNan((-inf)*negativeZero));                       \
}

#define testCompare(T)                                                     \
{ LOG_INFO("testCompare("#T")");                                           \
                                                                           \
  /* The first set of tests check things that any self-respecting       */ \
  /* comparison function should agree upon.                             */ \
                                                                           \
  /* Make sure that zero and negativeZero compare as equal.             */ \
  ASSERT(Numeric::compare(zero, negativeZero, mask_t(0)));                 \
                                                                           \
  /* Make sure that nearby numbers compare as equal.                    */ \
  ASSERT(Numeric::compare(two, nearestTwo));                               \
                                                                           \
  /* Make sure that slightly more distant numbers compare as equal.     */ \
  ASSERT(Numeric::compare(two, nearTwo));                                  \
                                                                           \
  /* Make sure the results are the same with parameters reversed.       */ \
  ASSERT(Numeric::compare(nearTwo, two));                                  \
                                                                           \
  /* Make sure that even more distant numbers don't compare as equal.   */ \
  ASSERT(!Numeric::compare(two, notNearTwo));                              \
                                                                           \
  /* The next set of tests check things where the correct answer isn't  */ \
  /* as obvious or important. Some of these tests check for cases that  */ \
  /* are rare or can easily be avoided. Some of them check for cases    */ \
  /* where the behavior of the 2sComplement function is arguably better */ \
  /* than the behavior of the fussier Final function.                   */ \
                                                                           \
  /* Test wrapping from inf to -inf; maxUlps is larger than mantissa.   */ \
  /* I.e. maxUlps = 1 << 23 for floats, and 1 << 52 for doubles;        */ \
  /* or as formula: maxUlps = 1 << 8*(sizeof(T)-1)-(2*sizeof(T)/3)+1    */ \
  mask_t maxUlps = (mask_t)1 << (8*(sizeof(T)-1)-(2*sizeof(T)/3)+1);       \
  LOG_DEBUG_STR("maxUlps = " << hex << showbase << maxUlps);               \
  ASSERT(!Numeric::compare(inf, -inf, maxUlps));                           \
                                                                           \
  /* Test whether max floating point value and infinity                 */ \
  /* (representationally adjacent) compare as unequal.                  */ \
  ASSERT(!Numeric::compare(numeric_limits<T>::max(), inf));                \
                                                                           \
  /* Test whether a NAN compares as unequal to itself.                  */ \
  ASSERT(!Numeric::compare(nan, nan));                                     \
                                                                           \
  /* Test whether a NAN compares as unequal to a different NAN.         */ \
  ASSERT(!Numeric::compare(nan2, nan3));                                   \
                                                                           \
  /* Test whether tiny numbers of opposite signs compare as equal.      */ \
  ASSERT(Numeric::compare(smallestDenormal, -smallestDenormal));           \
}

#define testAll(T)                                                   \
{ initNumbers(T);                                                    \
  showNumbers(T);                                                    \
  testNegative(T);                                                   \
  testFinite(T);                                                     \
  testInf(T);                                                        \
  testNan(T);                                                        \
  testCompare(T);                                                    \
}

void testFloat()
{
  float zero(0.0);
  float one(1.0);
  float two(2.0);
  float none(zero-one);
  float pinf(one/zero);
  float ninf(none/zero);
  float nan(pinf/pinf);
  union {
    long ione;
    float fone;
  } u = { 1 };  
  float pdmin(u.fone); // smallest denormalized float
  float ndmin(-pdmin);
  
  ASSERT(!Numeric::isNegative(zero));
  ASSERT(Numeric::isFinite(zero));
  ASSERT(!Numeric::isInf(zero));
  ASSERT(!Numeric::isNan(zero));

  ASSERT(!Numeric::isNegative(one));
  ASSERT(Numeric::isFinite(one));
  ASSERT(!Numeric::isInf(one));
  ASSERT(!Numeric::isNan(one));

  ASSERT(Numeric::isNegative(none));
  ASSERT(Numeric::isFinite(none));
  ASSERT(!Numeric::isInf(none));
  ASSERT(!Numeric::isNan(none));

  ASSERT(!Numeric::isNegative(pinf));
  ASSERT(!Numeric::isFinite(pinf));
  ASSERT(Numeric::isInf(pinf));
  ASSERT(!Numeric::isNan(pinf));

  ASSERT(Numeric::isNegative(ninf));
  ASSERT(!Numeric::isFinite(ninf));
  ASSERT(Numeric::isInf(ninf));
  ASSERT(!Numeric::isNan(ninf));

  ASSERT(!Numeric::isFinite(nan));
  ASSERT(!Numeric::isInf(nan));
  ASSERT(Numeric::isNan(nan));

  ASSERT(one/pinf == 0);
  ASSERT(one/ninf == 0);
  ASSERT(none/pinf == 0);
  ASSERT(none/ninf == 0);

  ASSERT(Numeric::isInf(pinf*pinf));
  ASSERT(Numeric::isInf(pinf*ninf));
  ASSERT(Numeric::isInf(ninf*pinf));
  ASSERT(Numeric::isInf(ninf*ninf));

  ASSERT(Numeric::isInf(one/zero));
  ASSERT(Numeric::isInf(none/zero));

  ASSERT(Numeric::isInf(pinf+pinf));
  ASSERT(Numeric::isInf(ninf+ninf));

  ASSERT(Numeric::isNan(zero/zero));
  ASSERT(Numeric::isNan(zero/(-zero)));
  ASSERT(Numeric::isNan((-zero)/zero));
  ASSERT(Numeric::isNan((-zero)/(-zero)));

  ASSERT(Numeric::isNan(pinf-pinf));
  ASSERT(Numeric::isNan(ninf-ninf));

  ASSERT(Numeric::isNan(pinf/pinf));
  ASSERT(Numeric::isNan(pinf/ninf));
  ASSERT(Numeric::isNan(ninf/pinf));
  ASSERT(Numeric::isNan(ninf/ninf));

  ASSERT(Numeric::isNan(pinf*zero));
  ASSERT(Numeric::isNan(ninf*zero));

  ASSERT(!Numeric::compare(pdmin, ndmin));
  ASSERT(Numeric::compare(pdmin, ndmin, 2));

  ASSERT(!Numeric::compare(one, two));
  // distance between one and two in ULPs is 0x800000L
  ASSERT(Numeric::compare(one, two, 0x800000L)); 
  ASSERT(!Numeric::compare(one, two, 0x7FFFFFL));
}

void testDouble()
{
  double zero(0.0);
  double one(1.0);
  double none(zero-one);
  double pinf(one/zero);
  double ninf(none/zero);
  double nan(pinf/pinf);

  ASSERT(!Numeric::isNegative(zero));
  ASSERT(Numeric::isFinite(zero));
  ASSERT(!Numeric::isInf(zero));
  ASSERT(!Numeric::isNan(zero));

  ASSERT(!Numeric::isNegative(one));
  ASSERT(Numeric::isFinite(one));
  ASSERT(!Numeric::isInf(one));
  ASSERT(!Numeric::isNan(one));

  ASSERT(Numeric::isNegative(none));
  ASSERT(Numeric::isFinite(none));
  ASSERT(!Numeric::isInf(none));
  ASSERT(!Numeric::isNan(none));

  ASSERT(!Numeric::isNegative(pinf));
  ASSERT(!Numeric::isFinite(pinf));
  ASSERT(Numeric::isInf(pinf));
  ASSERT(!Numeric::isNan(pinf));

  ASSERT(Numeric::isNegative(ninf));
  ASSERT(!Numeric::isFinite(ninf));
  ASSERT(Numeric::isInf(ninf));
  ASSERT(!Numeric::isNan(ninf));

  ASSERT(!Numeric::isFinite(nan));
  ASSERT(!Numeric::isInf(nan));
  ASSERT(Numeric::isNan(nan));

  ASSERT(one/pinf == 0);
  ASSERT(one/ninf == 0);
  ASSERT(none/pinf == 0);
  ASSERT(none/ninf == 0);

  ASSERT(Numeric::isInf(pinf*pinf));
  ASSERT(Numeric::isInf(pinf*ninf));
  ASSERT(Numeric::isInf(ninf*pinf));
  ASSERT(Numeric::isInf(ninf*ninf));

  ASSERT(Numeric::isInf(one/zero));
  ASSERT(Numeric::isInf(none/zero));

  ASSERT(Numeric::isInf(pinf+pinf));
  ASSERT(Numeric::isInf(ninf+ninf));

  ASSERT(Numeric::isNan(zero/zero));
  ASSERT(Numeric::isNan(zero/(-zero)));
  ASSERT(Numeric::isNan((-zero)/zero));
  ASSERT(Numeric::isNan((-zero)/(-zero)));

  ASSERT(Numeric::isNan(pinf-pinf));
  ASSERT(Numeric::isNan(ninf-ninf));

  ASSERT(Numeric::isNan(pinf/pinf));
  ASSERT(Numeric::isNan(pinf/ninf));
  ASSERT(Numeric::isNan(ninf/pinf));
  ASSERT(Numeric::isNan(ninf/ninf));

  ASSERT(Numeric::isNan(pinf*zero));
  ASSERT(Numeric::isNan(ninf*zero));
}


int main(int /*argc*/, const char* argv[])
{
  INIT_LOGGER(argv[0]);
  LOG_INFO("Starting up ...");
  try {
    testAll(float);
    testAll(double);
  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  LOG_INFO("Program terminated successfully");
  return 0;
}
