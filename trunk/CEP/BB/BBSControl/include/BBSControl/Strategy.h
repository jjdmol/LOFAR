//# Strategy.h: The properties for solvable parameters
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
// The properties for solvable parameters

//# Includes
#include <BBSControl/BBSStructs.h>
#include <BBSControl/Command.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
  //# Forward declarations
  namespace ACC { namespace APS { class ParameterSet; } }

  namespace BBS
  {
    //# Forward declarations
    class Step;

    // \addtogroup BBSControl
    // @{

    class Strategy : public Command
    {
    public:
      // Default constructor. Create an empty strategy, which is useful when
      // deserializing a Strategy object.
      Strategy() {}

      // Create a solve strategy for the given work domain.
      Strategy(const ACC::APS::ParameterSet& aParamSet);

      // Destructor.
      ~Strategy();

      // Return the command type of \c *this as a string.
      virtual const string& type() const;

      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ACC::APS::ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this.
      virtual void read(const ACC::APS::ParameterSet& ps);

      // Accept a CommandVisitor that wants to process \c *this.
      virtual void accept(CommandVisitor &visitor) const;

      // Print the contents of \c this into the output stream \a os.
      void print(ostream& os) const;

      // Return the steps that this strategy consists of. Multisteps are
      // expanded recursively until only single steps remain. Expansion is
      // done in pre-order, depth-first.
      // \todo Do we really want to implement such "iterator-like behaviour"
      // in this class?
      vector< shared_ptr<const Step> > getAllSteps() const;

      // Indicate whether the Steps contained in \c itsSteps should also be
      // written when write(ParameterSet&) is called.
      void shouldWriteSteps(bool doSteps) { itsWriteSteps = doSteps; }

      // @name Accessor methods
      // @{
      string            dataSet()          const { return itsDataSet; }
      ParmDB            parmDB()           const { return itsParmDB; }
      vector<string>    stations()         const { return itsStations; }
      string            inputData()        const { return itsInputData; }
      RegionOfInterest  regionOfInterest() const { return itsRegionOfInterest;}
      DomainSize        domainSize()       const { return itsDomainSize; }
      Correlation       correlation()      const { return itsCorrelation; }
      Integration       integration()      const { return itsIntegration; }
      // @}

    private:
      // Read the Step objects from the parameter set \a ps and store them
      // in \a itsSteps.
      bool readSteps(const ACC::APS::ParameterSet& ps);

      // Write the Step objects in \a itsSteps to parameter set \a ps.
      void writeSteps(ACC::APS::ParameterSet& ps) const;

//       // Write the contents of a Strategy to a ParameterSet.
//       friend ACC::APS::ParameterSet& 
//       operator<<(ACC::APS::ParameterSet&, const Strategy&);

//       // Read the contents of a ParameterSet into a Strategy.
//       friend ACC::APS::ParameterSet& 
//       operator>>(ACC::APS::ParameterSet&, Strategy&);

      // Name of the Measurement Set
      string                 itsDataSet;

      // Information about the parameter database.
      ParmDB                 itsParmDB;

      // Names of the stations to use. Names may contains wildcards, like \c *
      // and \c ?. Expansion of wildcards will be done in the BBS kernel, so
      // they will be passed unaltered by BBS control.
      vector<string>         itsStations;

      // Name of the MS input data column
      string                 itsInputData;

      // Region of interest
      RegionOfInterest       itsRegionOfInterest;

      // The work domain size
      DomainSize             itsDomainSize;

      // Selection type of the correlation products.
      Correlation            itsCorrelation;

      // Integration intervals in frequency (Hz) and time (s).
      Integration            itsIntegration;

      // Sequence of steps that comprise this solve strategy.
      vector< shared_ptr<const Step> > itsSteps;

      // Flag indicating whether the Step objects in \c itsSteps should
      // also be written when write(ParameterSet&) is called.
      bool                   itsWriteSteps;
    };

    // Write the contents of a Strategy to an output stream.
    ostream& operator<<(ostream&, const Strategy&);

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
