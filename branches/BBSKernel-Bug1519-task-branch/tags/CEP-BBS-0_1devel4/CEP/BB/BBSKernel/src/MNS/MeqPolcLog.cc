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

#include <lofar_config.h>

#include <BBSKernel/MNS/MeqPolcLog.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MeqPolcLog::MeqPolcLog(const LOFAR::ParmDB::ParmValue& pvalue)
  : MeqFunklet(pvalue)
{
  const LOFAR::ParmDB::ParmValueRep& pval = pvalue.rep();
  
  ASSERTSTR(pval.itsType == "polclog",
    "Funklet in ParmValue is not of type 'polclog'.");
       
//  ASSERTSTR(pval.itsConstants.size() >= 2,
//    "Not enough f0 constants found in the parmdb for this polclog.");
}

MeqPolcLog::~MeqPolcLog()
{
}

MeqPolcLog* MeqPolcLog::clone() const
{
  return new MeqPolcLog(*this);
}

MeqResult MeqPolcLog::getResult(const MeqRequest& request, int nrpert, int pertInx)
{
  // Assume the output domain is contained within the input domain, i.e.:
  // assume the left boundary of the first output cell lies on or to the
  // right of the left boundary of the first intput cell and the right boundary
  // of the last output cell lies on or to the left of the right boundary of the
  // last intput cell.
  //ASSERTSTR(outDomain.startX() >= inDomain.startX() && outDomain.endX() <= inDomain.endX() && 
  //          outDomain.startY() >= inDomain.startY() && outDomain.endY() <= inDomain.endY(),
  //          "The output domain should be completely contained within the input domain.");
  
  // Get the f0 constants from the parmdb.
  const LOFAR::ParmDB::ParmValueRep &parameters = itsParmValue.rep();
  ASSERTSTR(parameters.itsConstants.size() >= 2, "No constants found in the parmdb for this polclog.");
  const double f0[2] = {parameters.itsConstants[0], parameters.itsConstants[1]};
  ASSERTSTR(f0[0] > 0.0 && f0[1] == 0.0, "A polclog should always be logarithmic in frequency and linear in time.");
  ASSERTSTR(itsCoeff.nelements() > 0, "No coefficients found in the parmdb for this polclog.");
  ASSERTSTR(itsCoeff.nelements() == 1 || itsCoeff.nx() > 1, "A polclog should always depend on frequency. Otherwise, use a polc node instead.");
  
  // Check if any perturbed values need to be computed.
  const bool computePerturbedValues = nrpert > 0 && request.nspid() > 0;
  
  // Create the result object containing as many spids as needed for
  // this polynomial (but no more).
  MeqResult result(request.nspid());
  
  // If there is only one coefficient, the polynomial is independent
  // of x and y. So, fill the result with the value of the coefficient
  // and fill the perturbed value if required.
  if(itsCoeff.nelements() == 1)
  {
    result.setValue(MeqMatrix(itsCoeff.getDouble()));
    
    if(computePerturbedValues)
    {
      result.setPerturbedValue(itsScidInx,
        MeqMatrix(itsCoeff.getDouble() + itsCoeffPert.getDouble()));
    }
  }
  else
  {  
    // Allocate space for the value itself and keep a pointer.
    result.setValue(MeqMatrix(double(0), request.nx(), request.ny()));
    double* outValues = result.getValueRW().doubleStorage();

    // Allocate space for the perturbed values (if any).
    double** const outPerturbedValues = computePerturbedValues ? new double*[itsCoeff.nelements()] : NULL;
    if(computePerturbedValues)
    {
      // Create a matrix for each perturbed value and keep a pointer.
      int idx = pertInx;
      for(int i = 0; i < itsCoeff.nelements(); ++i)
      {
        if(isCoeffSolvable(i))
        {
          result.setPerturbedValue(idx, MeqMatrix(double(0), request.nx(), request.ny()));
          outPerturbedValues[i] = result.getPerturbedValueRW(idx).doubleStorage();
          idx++;
        }
        else
        {
          outPerturbedValues[i] = NULL;
        }
      }
    }

    
    // Call the appropriate function to evaluate the polynomial.
    if(itsCoeff.ny() == 1)
    {
      evalUnivariatePolynomial(request, outValues, outPerturbedValues, computePerturbedValues, log10(f0[0]));
    }
    else
    {
      evalBivariatePolynomial(request, outValues, outPerturbedValues, computePerturbedValues, log10(f0[0]));
    }    
    
    delete [] outPerturbedValues;
  }
    
  return result;
}


void MeqPolcLog::evalUnivariatePolynomial(const MeqRequest &request,
                                          double* outValues,
                                          double** outPerturbedValues,
                                          const bool computePerturbedValues,
                                          const double log10_f0) const
{
  // To avoid confusion, we will not use 'x' and 'y' for the
  // independent and dependent variables. Rather, 'abscissa' is
  // used for the coordinate along the independent axis, and
  // 'ordinate' for the coordinate along the dependent axis (i.e.
  // the value of the polynomial evaluated at the abscissa).
  
  // Get the (number of) coefficients in the polynomial and their
  // associated perturbations.
  const int axis = 0;
  const int order[2] = {itsCoeff.nx(), itsCoeff.ny()};
  const double* const coefficients = itsCoeff.doubleStorage();
  const double* const perturbations = computePerturbedValues ? itsCoeffPert.doubleStorage() : NULL;

  // Get the output (request) domain, the number of cells in the output domain,
  // and the offsets and scales of the axes of the output domain.
  const MeqDomain &outDomain = request.domain();
  const int outCount[2] = {request.nx(), request.ny()};
  const int outStride[2] = {1, outCount[0]};
  const double outScale[2] = {(outDomain.endX() - outDomain.startX()) / outCount[0], (outDomain.endY() - outDomain.startY()) / outCount[1]};
  const double outOffset[2] = {outDomain.startX() + 0.5 * outScale[0], outDomain.startY() + 0.5 * outScale[1]};
  
  // Compute the output values, and perturbed values if required.
  double abscissa = outOffset[axis];
  for(int i = 0; i < outCount[axis]; i++)
  {
    // Transform the abscissa.
    double transformedAbscissa = log10(abscissa) - log10_f0;

    // Evaluate the polynomial at the transformed abscissa using Horner's rule.
    double ordinate = coefficients[order[axis] - 1];
    for(int j = order[axis] - 2; j >= 0; --j)
    {
      ordinate *= transformedAbscissa;
      ordinate += coefficients[j];
    }

    // DEBUG
    //cout << "abscissa, transformedAbscissa, ordinate: " << abscissa << " " << transformedAbscissa << " " << ordinate << endl;

    // Distribute the value over the dependent axis.
    for(int j = 0; j < outCount[1 - axis]; ++j)
    {
      outValues[j * outStride[1 - axis]] = ordinate;
    }

    // Compute perturbed values if required.
    if(computePerturbedValues)
    {
      double powAbscissa = 1.0;

      // Loop over all coefficients.
      for(int j = 0; j < order[axis]; j++)
      {
        if(outPerturbedValues[j])
        {
          // Compute the perturbed value for the current coefficient.
          const double perturbedValue = ordinate + perturbations[j] * powAbscissa;

          // Distribute the perturbed value over the dependent axis.
          for(int k = 0; k < outCount[1 - axis]; ++k)
          {
            outPerturbedValues[j][k * outStride[1 - axis]] = perturbedValue;
          }

          outPerturbedValues[j] += outStride[axis];
        }

        powAbscissa *= transformedAbscissa;
      }
    }

    // Advance to the next cell.
    abscissa += outScale[axis];
    outValues += outStride[axis];
  }
}


void MeqPolcLog::evalBivariatePolynomial(const MeqRequest &request,
                                         double* outValues,
                                         double** outPerturbedValues,
                                         const bool computePerturbedValues,
                                         const double log10_f0) const
{
  // Get the (number of) coefficients in the polynomial and their
  // associated perturbations.
  const int order[2] = {itsCoeff.nx(), itsCoeff.ny()};
  const double* const coefficients = itsCoeff.doubleStorage();
  const double* const perturbations = computePerturbedValues ? itsCoeffPert.doubleStorage() : NULL;

  // Get the offsets and sizes of the axes of the input domain.
  const MeqDomain &inDomain = domain();
  const double axisOffset[2] = {inDomain.startX(), inDomain.startY()};
  const double axisSize[2] = {inDomain.endX() - inDomain.startX(), inDomain.endY() - inDomain.startY()};
  
  // Get the output (request) domain, the number of cells in the output domain, 
  // and the offsets and scales of the axes of the output domain.
  const MeqDomain &outDomain = request.domain();
  const int outCount[2] = {request.nx(), request.ny()};
  const double outScale[2] = {(outDomain.endX() - outDomain.startX()) / outCount[0], (outDomain.endY() - outDomain.startY()) / outCount[1]};
  const double outOffset[2] = {outDomain.startX() + 0.5 * outScale[0], outDomain.startY() + 0.5 * outScale[1]};

  // Allocate an array for the pre-computed powers of x, needed for the
  // computation of the perturbed values.
  double * const powersX = computePerturbedValues ? new double[order[0]] : NULL;

  // Compute the output values, and perturbed values if required.
  double y = outOffset[1];    
  for(int j = 0; j < outCount[1]; j++)
  {
    // Transform the y coordinate.
    double transformedY = (y - axisOffset[1]) / axisSize[1];
    
    double x = outOffset[0];
    for(int i = 0; i < outCount[0]; i++)
    {
      // Transform the x coordinate.
      double transformedX = log10(x) - log10_f0;

      double powY = 1.0;
      double value = 0.0;

      // Compute the value of the polynomial at (transformedX, transformedY)
      // using Horner's rule.
      for(int cy = 0; cy < order[1]; ++cy)
      {
        double valueX = coefficients[cy * order[0] + order[0] - 1];
        for(int cx = order[0] - 2; cx >= 0; --cx)
        {
          valueX *= transformedX;
          valueX += coefficients[cy * order[0] + cx];
        }

        value += valueX * powY;
        powY *= transformedY;
      }

      // Store the value.
      *(outValues) = value;
      outValues++;

      // Compute perturbed values if required.
      if(computePerturbedValues)
      { 
        // Precompute the power of x.
        double powX = 1.0;
        for(int cx = 0; cx < order[0]; ++cx)
        {
          powersX[cx] = powX;
          powX *= transformedX;
        }

        int idx = 0;
        double powY = 1.0;
        for(int cy = 0; cy < order[1]; ++cy)
        {
          for(int cx = 0; cx < order[0]; ++cx)
          {
            if(outPerturbedValues[idx])
            {
              *(outPerturbedValues[idx]) = value + perturbations[idx] * powersX[cx] * powY;
              outPerturbedValues[idx]++;
            }
            idx++;
          }
          
          powY *= transformedY;
        }
      } // If(computePerturbedValues)

      // Advance to the next cell.
      x += outScale[0];
    } // Loop over i
    
    // Advance to the next row of cells.
    y += outScale[1];
  } // Loop over j

  delete [] powersX;
}


MeqResult MeqPolcLog::getAnResult(const MeqRequest& request, int nrpert, int pertInx)
{
  // Not implemented yet.
  return getResult(request, nrpert, pertInx);
}

} // namespace BBS
} // namespace LOFAR
