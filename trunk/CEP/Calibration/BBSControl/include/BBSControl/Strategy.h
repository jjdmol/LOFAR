//# Strategy.h: Strategy (sequence of Steps) to calibrate the visibility data.
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

      string getInputColumn() const
      { return itsInputColumn; }

      vector<string> getStations() const
      { return itsStations; }
      
      Correlation getCorrelation() const
      { return itsCorrelation; }
      
      vector<string> getTimeWindow() const
      { return itsTimeWindow; }
      
      size_t getChunkSize() const
      { return itsChunkSize; }
      
      bool useSolver() const
      { return itsUseSolver; }

      // Write the contents of \c *this into the ParameterSet \a ps.
//      void write(ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this.
//      void read(const ParameterSet& ps);

      // Print the contents of \c this into the output stream \a os.
      void print(ostream& os) const;

    private:
//      shared_ptr<const Step> getRoot() const
//      { return itsRoot; }

      // Name of the input column.
      string                 itsInputColumn;

      // Names of the stations to use. Names may contains wildcards, like \c *
      // and \c ?.
      vector<string>         itsStations;

      // Correlation product selection.
      Correlation            itsCorrelation;

      // Time window.
      vector<string>         itsTimeWindow;

      // Size in timeslots of the block of data that will be processed as a
      // single chunk.
      size_t                 itsChunkSize;

      // Connect to the global solver?
      bool                   itsUseSolver;

      // Root step of the strategy tree.
//      shared_ptr<Step>  itsRoot;
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
