//# Strategy.h: Strategy (sequence of Steps) to calibrate the visibility data.
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_BBSCONTROL_BBSSTRATEGY_H
#define LOFAR_BBSCONTROL_BBSSTRATEGY_H

// \file
// Strategy (sequence of Steps) to calibrate the visibility data.

#include <BBSControl/Types.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_stack.h>

namespace LOFAR
{
  //# Forward declarations
  class ParameterSet;

  namespace BBS
  {
    //# Forward declarations
    class Step;
    class StrategyIterator;

    // \addtogroup BBSControl
    // @{

    class Strategy
    {
    public:
      // Default constructor. Create an empty strategy, which is useful when
      // deserializing a Strategy object.
      Strategy() {}

      // Create a solve strategy for the given work domain.
      Strategy(const ParameterSet& in);

      // Destructor.
      ~Strategy();

      string inputColumn() const
      { return itsInputColumn; }

      Selection selection() const
      { return itsSelection; }

      vector<string> getTimeWindow() const
      { return itsTimeWindow; }

      size_t getChunkSize() const
      { return itsChunkSize; }

      bool useSolver() const
      { return itsUseSolver; }

      // Print the contents of \c this into the output stream \a os.
      void print(ostream& os) const;

    private:
      // Name of the input column.
      string                            itsInputColumn;

      Selection                         itsSelection;

      // Time window.
      vector<string>                    itsTimeWindow;

      // Size in timeslots of the block of data that will be processed as a
      // single chunk.
      size_t                            itsChunkSize;

      // Connect to the global solver?
      bool                              itsUseSolver;

      // Root step of the strategy tree.
      vector<shared_ptr<const Step> >   itsSteps;

      friend class StrategyIterator;
    };

    // Write the contents of a Strategy to an output stream.
    ostream& operator<<(ostream&, const Strategy&);

    // Iterate over all the SingleSteps (leaf nodes) of a Strategy.
    class StrategyIterator
    {
    public:
      // Default constructor. Creates an StrategyIterator that iterates over an
      // empty Strategy. Allows easy incorporation of a StrategyIterator as a
      // class member.
      StrategyIterator() {}

      // Create an StrategyIterator for the given Strategy.
      StrategyIterator(const Strategy &strategy);

      // Is the iterator pointing at the end of the Strategy?
      bool atEnd() const;

      // Return the current Step.
      shared_ptr<const Step> operator*() const;

      // Advance the iterator (prefix).
      void operator++();

    private:
      shared_ptr<const Step>          itsCurrent;
      stack<shared_ptr<const Step> >  itsStack;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
