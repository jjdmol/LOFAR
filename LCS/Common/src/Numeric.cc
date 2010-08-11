//# Numeric.cc: 
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
#include <cstdlib>

namespace LOFAR
{
  bool Numeric::isFinite(float f)
  {
    floatUnion mask = { f };
    return mask.mask & floatExponentMask != floatExponentMask;
  }

  bool Numeric::isFinite(double d)
  {
    doubleUnion mask = { d };
    return mask.mask & doubleExponentMask != doubleExponentMask;
  }

  bool Numeric::isNegative(float f)
  {
    floatUnion mask = { f };
    return mask.mask & floatNegativeMask == floatNegativeMask;
  }

  bool Numeric::isNegative(double d)
  {
    doubleUnion mask = { d };
    return mask.mask & doubleNegativeMask == doubleNegativeMask;
  }

  bool Numeric::isInf(float f)
  {
    floatUnion mask = { f };
    return !isFinite(f) && (mask.mask & floatMantissaMask == 0L);
  }

  bool Numeric::isInf(double d)
  {
    doubleUnion mask = { d };
    return !isFinite(d) && (mask.mask & doubleMantissaMask == 0LL);
  }

  bool Numeric::isNan(float f)
  {
    floatUnion mask = { f };
    return !isFinite(f) && (mask.mask & floatMantissaMask != 0L);
  }

  bool Numeric::isNan(double d)
  {
    doubleUnion mask = { d };
    return !isFinite(d) && (mask.mask & doubleMantissaMask != 0LL);
  }


  bool Numeric::compare(float lhs, float rhs, floatMask_t maxUlps)
  {

#ifdef USE_INFINITYCHECK
    // If either \a lhs or \a rhs is infinite, then return true if they're
    // exactly equal, i.e. they have the same sign.
    if (isInf(lhs) || isInf(rhs)) return lhs == rhs;
#endif

#ifdef USE_NANCHECK
    // Consider \a lhs and \a rhs unequal when either is NaN. 
    if (isNan(lhs) || isNan(rhs)) return false;
#endif

#ifdef USE_SIGNCHECK
    // Consider \a lhs and \a rhs unequal when they have a different
    // sign, even if they both are very close to zero.
    // \note The check for equality is needed because zero and negative zero
    // have different signs but are equal to each other.
    if (isNegative(lhs) != isNegative(rhs)) return lhs == rhs;
#endif

    floatUnion mlhs = { lhs };
    floatUnion mrhs = { rhs };
    floatMask_t ilhs = mlhs.mask;
    floatMask_t irhs = mrhs.mask;

    // Make \a ilhs and \a irhs lexicographically ordered as twos-complement
    // long.
    if (ilhs < 0) ilhs = floatNegativeMask - ilhs;
    if (irhs < 0) irhs = floatNegativeMask - irhs;

    // If \a ilhs and \a irhs are less than \a maxUlps apart, then \a lhs and
    // \a rhs are considered equal.
    return std::abs(ilhs - irhs) <= maxUlps;

  }


  bool Numeric::compare(double lhs, double rhs, doubleMask_t maxUlps)
  {

#ifdef USE_INFINITYCHECK
    // If either \a lhs or \a rhs is infinite, then return true if they're
    // exactly equal, i.e. they have the same sign.
    if (isInf(lhs) || isInf(rhs)) return lhs == rhs;
#endif

#ifdef USE_NANCHECK
    // Consider \a lhs and \a rhs unequal when either is NaN. 
    if (isNan(lhs) || isNan(rhs)) return false;
#endif

#ifdef USE_SIGNCHECK
    // Consider \a lhs and \a rhs unequal when they have a different
    // sign, even if they both are very close to zero.
    // \note The check for equality is needed because zero and negative zero
    // have different signs but are equal to each other.
    if (isNegative(lhs) != isNegative(rhs)) return lhs == rhs;
#endif

    doubleUnion mlhs = { lhs };
    doubleUnion mrhs = { rhs };
    doubleMask_t ilhs = mlhs.mask;
    doubleMask_t irhs = mrhs.mask;

    // Make \a ilhs and \a irhs lexicographically ordered as twos-complement
    // long.
    if (ilhs < 0) ilhs = doubleNegativeMask - ilhs;
    if (irhs < 0) irhs = doubleNegativeMask - irhs;

    // If \a ilhs and \a irhs are less than \a maxUlps apart, then \a lhs and
    // \a rhs are considered equal.
    ilhs -= irhs;
    if (ilhs < 0) ilhs = -ilhs;
    return ilhs <= maxUlps;

  }
  

} // namespace LOFAR
