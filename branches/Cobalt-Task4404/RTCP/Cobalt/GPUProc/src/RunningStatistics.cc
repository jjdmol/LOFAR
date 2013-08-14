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
#include <math.h> 

namespace LOFAR
{
  namespace Cobalt
  {

    RunningStatistics::RunningStatistics() 
    {
      reset();
    }

    void RunningStatistics::reset()
    {
      counter = 0;
      _mean = var_base =  0.0;
    }

    void RunningStatistics::push(double x)
    {
      double delta_mean, delta_weighted;

      long long old_counter = counter;
      counter++;
      delta_mean = x - _mean;
      delta_weighted = delta_mean / counter;
      _mean += delta_weighted;
      var_base += delta_mean * delta_weighted * old_counter;
    }

    size_t RunningStatistics::count() const
    {
      return counter;
    }

    double RunningStatistics::mean() const
    {
      return _mean;
    }

    double RunningStatistics::variance() const
    {
      return  counter != 1? var_base/(counter-1.0): 0.0;
    }

    double RunningStatistics::stDev() const
    {
      return sqrt( variance() );
    }

    RunningStatistics& RunningStatistics::operator+=(const RunningStatistics& rhs)
    { 
      RunningStatistics combined = *this + rhs;
      *this = combined;

      return *this;
    }

    RunningStatistics operator+(const RunningStatistics a,
         const RunningStatistics b)
    {
      RunningStatistics combined;
      combined.counter += a.counter + b.counter;

      double delta_means = b._mean - a._mean;
      double delta_means_sqrt = delta_means*delta_means;

      combined._mean = (a.counter * a._mean + b.counter * b._mean) / combined.counter;

      combined.var_base = a.var_base + b.var_base + 
        delta_means_sqrt * a.counter * b.counter / combined.counter;
      return combined;
    }
  }
}