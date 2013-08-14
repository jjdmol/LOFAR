//# RunningStatistics.cc
//#
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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
#include "RunningStatistics.h"

namespace LOFAR
{
  namespace Cobalt
  {

      RunningStatistics::RunningStatistics() 
        :
      counter(0) 
      {}

      void RunningStatistics::Clear()
      {
        counter = 0;
      }

      void RunningStatistics::Push(double sample)
      {
        counter++;

        if (counter == 1)
        {
          oldMean = newMean = sample;
          oldVariance = 0.0;
        }
        else
        {
          // The 
          newMean = oldMean + (sample - oldMean) / counter;

          newVariance = oldVariance + (sample - oldMean) * (sample - newMean);

          // set up for next iteration
          oldMean = newMean; 
          oldVariance = newVariance;
        }
      }

      int RunningStatistics::NumDataValues() const
      {
        return counter;
      }

      double RunningStatistics::Mean() const
      {
        return (counter > 0) ? newMean : 0.0;
      }

      double RunningStatistics::Variance() const
      {
        return (counter > 1) ? 
          newVariance/(counter - 1) :
        0.0;
      }

      double RunningStatistics::StandardDeviation() const
      {
        return sqrt( Variance() );
      }


  }
}