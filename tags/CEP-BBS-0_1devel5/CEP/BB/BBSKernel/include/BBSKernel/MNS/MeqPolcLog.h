//# MeqPolcLog.h: Univariate or bivariate polynomial with a
//# logarithmic transform on the frequency axis (log10(f/f0)).
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
// Univariate or bivariate polynomial with a logarithmic transform on the
// frequency axis (log10(f/f0)).
// 
// In the parmdb, a polclog always has two constants, one f0 constant for
// each axis. An f0 value of zero implies that the associated axis is linear.
// Otherwise, the axis is logarithmic (log10(f/f0)). Currently, the MeqPolcLog
// node only supports a univariate polynomial in frequency with a logarithmic
// axis, or a bivariate polynomial with a logarithmic frequency axis and a linear
// time axis. All other configurations will raise an exception.
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
#include <BBSKernel/MNS/MeqFunklet.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{


class MeqPolcLog: public MeqFunklet
{
public:
  // Create an empty 2-dim polynomial.
  MeqPolcLog()
  {
  }

  // Create a PolcLog from a ParmValue object.
  MeqPolcLog(const LOFAR::ParmDB::ParmValue& pvalue);

  // Convert a PolcLog to a ParmValue object.
  LOFAR::ParmDB::ParmValue toParmValue() const;

  virtual ~MeqPolcLog();

  // Clone the PolcLog.
  virtual MeqPolcLog* clone() const;

  // Compute the value, and perturbed values if required.
  virtual MeqResult getResult(const MeqRequest&, int nrpert, int pertInx);
  virtual MeqResult getAnResult(const MeqRequest&, int nrpert, int pertInx);

private:
  void evalUnivariatePolynomial(const MeqRequest &request,
                                double* outValues,
                                double** outPerturbedValues,
                                const bool computePerturbedValues,
                                const double log10_f0) const;
  
  void evalBivariatePolynomial(const MeqRequest &request,
                               double* outValues,
                               double** outPerturbedValues,
                               const bool computePerturbedValues,
                               const double log10_f0) const;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
