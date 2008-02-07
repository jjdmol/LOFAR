//# MeqTabular.cc:  A tabular parameter value
//#
//# Copyright (C) 2006
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
#include <utility>

#include <BBSKernel/MNS/MeqTabular.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

#define MIN(__a, __b)   __a <= __b ? __a : __b;
 
using namespace std;
using namespace casa;

namespace LOFAR
{
namespace BBS
{

  MeqTabular::MeqTabular (const LOFAR::ParmDB::ParmValue& pvalue)
  : MeqFunklet (pvalue)
  {
    const LOFAR::ParmDB::ParmValueRep& pval = pvalue.rep();
    ASSERTSTR (pval.itsType == "tabular",
        "Funklet in ParmValue is not of type 'tabular'");
  }

  
  MeqTabular::~MeqTabular()
  {}

  
  MeqTabular* MeqTabular::clone() const
  {
    return new MeqTabular(*this);
  }

  // getResult(): resamples the tabular on the requested grid. NOTE: currently, linear interpolation
  // is used both for upsampling and downsampling. No low pass filter is used prior to downsampling.
  // Also, when samples outside the input domain are required, they are assumed to have a value of 0.0.
  // This may or may not be adequate for your specific application.
  // TODO: split off a 1-D resample method and use seperable interpolation kernels to be able
  // to easily generalize the resample method to n-D.
  MeqResult MeqTabular::getResult (const MeqRequest& request,
          int nrpert, int pertInx)
  {
    ASSERTSTR (nrpert == 0,
        "A tabular parameter value cannot be solvable.");

    ASSERTSTR (itsCoeff.nelements() != 0,
        "Empty tabular not allowed.");
    
    // Get input and output domains.
    const MeqDomain &inDomain = this->domain();
    const MeqDomain &outDomain = request.domain();

    // Assume the output domain is contained within the input domain, i.e.:
    // assume the left boundary of the first output cell lies on or to the
    // right of the left boundary of the first intput cell and the right boundary
    // of the last output cell lies on or to the left of the right boundary of the
    // last intput cell.
    ASSERTSTR(outDomain.startX() >= inDomain.startX() && outDomain.endX() <= inDomain.endX() && outDomain.startY() >= inDomain.startY() && outDomain.endY() <= inDomain.endY(),
      "The output domain should be completely contained within the input domain.");
    
    // Create result.
    MeqResult result(request.nspid());
    
    // Get the number of cells in the input and output domains.
    const pair<int, int> inCount(itsCoeff.nx(), itsCoeff.ny());
    const pair<int, int> outCount(request.nx(), request.ny());

    if (itsCoeff.nelements() == 1)
    {
      // If the input is scalar, it represents a constant -> fill the matrix with the input value.
      result.setValue(MeqMatrix(itsCoeff.getDouble(), outCount.first, outCount.second, true));
    }
    else if (itsCoeff.ny() == 1)
    {
      // The input is 1-D and is defined along the first (frequency) axis.
      
      // Get the scales of input and output domains.
      const pair<double, double> inScale((inDomain.endX() - inDomain.startX()) / inCount.first, (inDomain.endY() - inDomain.startY()) / inCount.second);
      const pair<double, double> outScale((outDomain.endX() - outDomain.startX()) / outCount.first, (outDomain.endY() - outDomain.startY()) / outCount.second);
      const pair<double, double> scaleRatio(outScale.first / inScale.first, outScale.second / inScale.second);
      
      // Get pointers to the input and output values.
      const double* const inValues = itsCoeff.doubleStorage();
      result.setValue(MeqMatrix(0.0, outCount.first, outCount.second));
      double* const outValues = result.getValueRW().doubleStorage();
      
      // Compute the centers of the first cells of the input and output domains.
      double inCenter = inDomain.startX() + 0.5 * inScale.first;
      double outCenter = outDomain.startX() + 0.5 * outScale.first;
      
      // If the centers of all input and output cells are perfectly aligned,
      // just copy the relevant piece of data.
      if(inScale.first == outScale.first && fmod(outDomain.startX() - inDomain.startX(), inScale.first) == 0.0)
      {
        int inStart = (int) ((outCenter - inCenter) / inScale.first);
        
        for(int i = 0; i < outCount.first; ++i)
        {
          outValues[i] = inValues[i + inStart];
        }
      }
      else
      {
        int leftBoundaryCells = 0;
        int rightBoundaryCells = 0;
        int outIndex;
        double offset;

        // Compute the number of boundary cells on the left. This is the number of
        // output cells of which the center lies to the left of the center of the
        // first input cell.
        if(outCenter < inCenter)
        {
          leftBoundaryCells = MIN((int) ceil((inCenter - outCenter) / outScale.first), outCount.first);
        }

        // Compute the number of boundary cells on the right. This is the number of
        // output cells of which the center lies to the right of the center of the
        // last input cell.
        if((outDomain.endX() - 0.5 * outScale.first) > (inDomain.endX() - 0.5 * inScale.first))
        {
          rightBoundaryCells = MIN((int) ceil(((outDomain.endX() - 0.5 * outScale.first) - (inDomain.endX() - 0.5 * inScale.first)) / outScale.first), outCount.first);
        }

        // Compute the values of the left boundary cells. A choice has to be made
        // regarding the boundary input value. For now, this value is fixed at 0.0.
        // However, if necessary this can/should be made configurable.
        for(outIndex = 0; outIndex < leftBoundaryCells; ++outIndex)
        {
          offset = 1.0 - (inCenter - outCenter) / inScale.first;

//          ASSERTSTR(offset >= 0.0 && offset <= 1.0, 
//            "Offset error on left boundary!" << offset << " " << outIndex << " " << leftBoundaryCells << " " << outCenter << " " << inCenter);

          // Compute output value by linear interpolation and store it.
          outValues[outIndex] = offset * inValues[0];
          outCenter += outScale.first;
        }

//        ASSERTSTR(outCenter >= inCenter,
//          "Center of the first output cell should lie to the right of the center of the first input cell.");

        // Get the index of the first relevant input cell.
        double inIndex = (outCenter - inCenter) / inScale.first;

        // Compute output values of all non-boundary cells.
        int index;
        for(outIndex = leftBoundaryCells; outIndex < outCount.first - rightBoundaryCells; ++outIndex)
        {
          // Cell localization.
          index = (int) inIndex;

          // Recompute cell center.
          inCenter = inDomain.startX() + index * inScale.first + 0.5 * inScale.first;

          // Compute offset of output center from input cell center.
          offset = 1.0 - (outCenter - inCenter) / inScale.first;
//          ASSERTSTR(offset >= 0.0 && offset <= 1.0, "Offset error!");

          // Compute output value by linear interpolation and store it.
          outValues[outIndex] = offset * inValues[index] + (1.0 - offset) * inValues[index + 1];

          // Update input index.
          inIndex += scaleRatio.first;

          // Update output cell center.
          outCenter += outScale.first;
        }

        // Compute the values of the right boundary cells. A choice has to be made
        // regarding the boundary input value. For now, this value is fixed at 0.0.
        // However, if necessary this can/should be made configurable.
        //index = (int) inIndex;
        //inCenter = inDomain.startX() + index * inScale.first + 0.5 * inScale.first;
        inCenter = inDomain.endX() - 0.5 * inScale.first;
        
        for(outIndex = outCount.first - rightBoundaryCells; outIndex < outCount.first; ++outIndex)
        {
          offset = 1.0 - (outCenter - inCenter) / inScale.first;

//          ASSERTSTR(offset >= 0.0 && offset <= 1.0, 
//            "Offset error on right boundary!" << outIndex << "," << rightBoundaryCells << ", " << outCenter << ", " << inCenter);

          // Compute output value by linear interpolation and store it.
          outValues[outIndex] = offset * inValues[inCount.first - 1];
          outCenter += outScale.first;
        }
      }
      
      // Distribute the computed values (first row) over the other axis if necessary.
      // TODO: split this off to a more general method in MeqMatrix?
      if(outCount.second > 1)
      {
        int index = outCount.first;
        
        for(int y = 1; y < outCount.second; ++y)
        {
          for(int x = 0; x < outCount.first; ++x)
          {
            outValues[index] = outValues[x];
            index++;
          }
        }
      }
    }

    return result;
  }

  
  MeqResult MeqTabular::getAnResult (const MeqRequest& request,
            int nrpert, int pertInx)
  {
    return getResult (request, nrpert, pertInx);
  }

} // namespace BBS
} // namespace LOFAR
