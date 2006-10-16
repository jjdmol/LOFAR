//# BBSStrategy.h: The properties for solvable parameters
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
#include <BBSControl/BlobStreamable.h>
#include <BBSControl/BBSStructs.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  //# Forward declarations
  class BlobIStream;
  class BlobOStream;
  namespace ACC { namespace APS { class ParameterSet; } }

  namespace BBS
  {
    //# Forward declarations
    class BBSStep;

    // \addtogroup BBS
    // @{

    class BBSStrategy : public BlobStreamable
    {
    public:
      // Default constructor. Create an empty strategy, which is useful when
      // deserializing a BBSStrategy object.
      BBSStrategy() {}

      // Create a solve strategy for the given work domain.
      BBSStrategy(const ACC::APS::ParameterSet& aParamSet);

      // Destructor.
      ~BBSStrategy();

      // Print the contents of \c this into the output stream \a os.
      void print(ostream& os) const;

      // Return the steps that this strategy consists of. Multisteps are
      // expanded recursively until only single steps remain. Expansion is
      // done in pre-order, depth-first.
      // \todo Do we really want to implement such "iterator-like behaviour"
      // in this class?
      vector<const BBSStep*> getAllSteps() const;

      // Indicate whether the BBSSteps contained in \c itsSteps should also be
      // written when write(BlobOStream&) is called.
      void shouldWriteSteps(bool doSteps) { itsWriteSteps = doSteps; }

      // @name Accessor methods
      // @{
      string         dataSet()     const { return itsDataSet; }
      BBDB           bbDB()        const { return itsBBDB; }
      ParmDB         parmDB()      const { return itsParmDB; }
      vector<string> stations()    const { return itsStations; }
      string         inputData()   const { return itsInputData; }
      DomainSize     domainSize()  const { return itsDomainSize; }
      Correlation    correlation() const { return itsCorrelation; }
      Integration    integration() const { return itsIntegration; }
      // @}

    private:
      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Return the class type of \c *this as a string.
      virtual const string& classType() const;

      // Read the BBSStep objects from the blob input stream \a bis and store
      // them in \a itsSteps.
      void BBSStrategy::readSteps(BlobIStream& bis);

      // Write the BBSStep objects in \a itsSteps to the blob output stream \a
      // bos.
      void BBSStrategy::writeSteps(BlobOStream& bos) const;

      // Name of the Measurement Set
      string                 itsDataSet;

      // Information about the blackboard database.
      BBDB                   itsBBDB;

      // Information about the parameter database.
      ParmDB                 itsParmDB;

      // Names of the stations to use. Names may contains wildcards, like \c *
      // and \c ?. Expansion of wildcards will be done in the BBS kernel, so
      // they will be passed unaltered by BBS control.
      vector<string>         itsStations;

      // Name of the MS input data column
      string                 itsInputData;

      // The work domain size
      DomainSize             itsDomainSize;
      
      // Selection type of the correlation products.
      Correlation            itsCorrelation;

      // Integration intervals in frequency (Hz) and time (s).
      Integration            itsIntegration;

      // Sequence of steps that comprise this solve strategy.
      vector<const BBSStep*> itsSteps;

      // Flag indicating whether the BBSStep objects in \c itsSteps should
      // also be written when write(BlobOStream&) is called.
      bool                   itsWriteSteps;
    };

    // Write the contents of a BBSStrategy to an output stream.
    ostream& operator<<(ostream&, const BBSStrategy&);

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
