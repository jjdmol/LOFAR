//# Correlation.h: Definition of supported correlation products.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBSKERNEL_CORRELATION_H
#define LOFAR_BBSKERNEL_CORRELATION_H

// \file
// Definition of supported correlation products.

#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

enum Correlation
{
    // Linear correlations.
    XX,
    XY,
    YX,
    YY,
    // Circular correlations.
    RR,
    RL,
    LR,
    LL,
    N_Correlation
};

// Check if the correlation is valid (i.e. < N_Correlation).
bool isDefined(Correlation in);

// Check if the correlation is linear (i.e. one of XX, XY, YX, YY).
bool isLinear(Correlation in);

// Check if the correlation is circular (i.e. one of RR, RL, LR, LL).
bool isCircular(Correlation in);

// Convert the input argument to the corresponding Correlation. If the input is
// out of bounds, N_Correlation is returned.
Correlation asCorrelation(unsigned int in);

// Convert the input argument to the corresponding Correlation. If the input
// does not match any defined Correlation, N_Correlation is returned.
Correlation asCorrelation(const string &in);

// Convert the input Correlation to its string representation. N_Correlation
// converts to "<UNDEFINED>".
const string &asString(Correlation in);

// Write a Correlation value to an output stream in human readable form.
ostream &operator<<(ostream &out, Correlation obj);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
