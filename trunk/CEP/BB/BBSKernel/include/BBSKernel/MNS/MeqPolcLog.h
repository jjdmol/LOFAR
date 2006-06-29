//# MeqPolcLog.h: Univariate or bivariate polynomial. Each axis has an
//# associated transformation.
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef MNS_MEQPOLCLOG_H
#define MNS_MEQPOLCLOG_H

// \file
// Univariate or bivariate polynomial. Each axis has an associated transformation.
// The following transformations are currently supported:
//     - linear_axis(a, b)  : x' = (x - a) / b
//     - log_axis_base10(f0): x' = log10(x / f0)
// 
// A polclog always has two constants, one f0 constant for each axis. An f0 value
// of zero implies that the associated axis is linear. Otherwise, the log_axis_base10
// transformation is used.
//
// As an example, suppose we have a polclog of which the first axis is logarithmic, and
// the second is linear. The polynomial is of degree one in both f' and t':
//
// p(f', t') = c00 + c10 * f' + c01 * t' + c11 * f' * t'
//
// Frequency f (in Hz) and time t (in s) are related to f' and t' by:
//
// f' = log10(f / f0)
//    = log10(f) - log10(f0)
//
// t' = (t - t_s) / (t_e - t_s)
//
// Where f_s and f_e are the start and end of the frequency domain on which the polclog
// is valid; similarly for t_s and t_e.
//
// Currently, the coefficients are not transformed, so should be given for the domains
// of f' and t', [log10(f_s / f0), log10(f_e / f0)] and [0.0, 1.0] respectively in this
// example.

//# Includes
#include <BBS/MNS/MeqFunklet.h>
#include <functional>

using std::unary_function;

namespace LOFAR {

// \ingroup BBS
// \addtogroup MNS
// @{

struct linear_axis: public unary_function<double, double>
{
  linear_axis(const double _start, const double _size)
    : start(_start),
      scale(1.0 / _size)
  {
  }
  
  double operator()(double x) const
  {
    return (x - start) * scale;
  }

  const double start;
  const double scale;
};

struct log_axis_base10: public unary_function<double, double>
{
  log_axis_base10(const double _f0)
    : log_f0(log10(_f0))
  {
  }
  
  double operator()(double x) const
  {
    return log10(x) - log_f0;
  }
  
  const double log_f0;
};

class MeqPolcLog: public MeqFunklet
{
public:
  // Create an empty 2-dim polynomial.
  MeqPolcLog()
  {
  }

  // Create a PolcLog from a ParmValue object.
  MeqPolcLog(const ParmDB::ParmValue& pvalue);

  // Convert a PolcLog to a ParmValue object.
  ParmDB::ParmValue toParmValue() const;

  virtual ~MeqPolcLog();

  // Clone the PolcLog.
  virtual MeqPolcLog* clone() const;

  // Calculate the value and possible perturbations.
  virtual MeqResult getResult (const MeqRequest&,
        int nrpert, int pertInx);
  virtual MeqResult getAnResult (const MeqRequest&,
        int nrpert, int pertInx);

private:
  template <class AxisTransform>
  void evaluateUnivariatePolynomial(int axis,
                                    const MeqRequest &request,
                                    double* outValues,
                                    double** const outPerturbedValues,
                                    bool computePerturbedValues,
                                    AxisTransform axisTransform);
  
  template <class AxisTransformX, class AxisTransformY>                                    
  void evaluateBivariatePolynomial(const MeqRequest &request,
                                   double* outValues,
                                   double** const outPerturbedValues,
                                   bool computePerturbedValues,
                                   AxisTransformX axisTransformX,
                                   AxisTransformY axisTransformY);
};

// @}

}

#endif
