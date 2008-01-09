//# BBSStep.h: Base component class of the BBSStep composite pattern.
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

#ifndef LOFAR_BBSCONTROL_BBSSTEP_H
#define LOFAR_BBSCONTROL_BBSSTEP_H

// \file
// Base component class of the BBSStep composite pattern.

//# Includes
#include <BBSControl/Command.h>
#include <BBSControl/BBSStructs.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
  //# Forward Declarations.
  namespace ACC { namespace APS { class ParameterSet; } }

  namespace BBS
  {
    //# Forward Declarations.
    class BBSMultiStep;
    class StrategyController;

    // \addtogroup BBSControl
    // @{

    // This is the so-called \e component class in the BBSStep composite
    // pattern (see Gamma, 1995). It is the base class for all BBSStep
    // classes, both composite and leaf classes. It has data members that are
    // common to all BBSStep classes.
    //
    // \todo Make the BBSStep class family exception safe. Currently, methods
    // like BBSStep::create and BBSStep::deserialize operate on and return raw
    // pointers. When an exception occurs within these methods, memory will be
    // leaked. The solution is to use, e.g., boost::shared_ptr. We must be
    // careful though, not to create circular dependencies, since that will
    // cause the pointer to never be freed. This can happen since a BBSStep
    // stores a pointer to its parent as backreference. Here, we should
    // probably use a boost::weak_ptr. See Bug #906
    class BBSStep : public Command,
                    public enable_shared_from_this<BBSStep>
    {
    public:
      // Destructor.
      virtual ~BBSStep();

      // Return the full name of this step. The full name consists of the name
      // of this step, preceeded by that of its parent, etc., separated by
      // dots.
      string fullName() const;

      // Get all steps that this step consists of. The result will be a vector
      // containing pointers to all steps, sorted pre-order depth-first.
      //
      // \todo Instead of making getAllSteps() a member function, returning a
      // vector of BBSStep*, it would be better to have a BBSStepIterator
      // class that can be used to iterate over the all steps. I had some
      // trouble getting that thingy working, so, due to time constraints, I
      // implemented things the ugly way.
      vector< shared_ptr<const BBSStep> > getAllSteps() const;

      // Create a new step object. The new step can either be a BBSSingleStep
      // or a BBSMultiStep object. This is determined by examining the
      // parameter set \a parSet. If this set contains a key
      // <tt>Step.<em>name</em>.Steps</tt>, then \a aName is a BBSMultiStep,
      // otherwise it is a SingleStep. The third, optional, argument is used
      // to pass a backreference to the parent BBSStep object.
      static shared_ptr<BBSStep> create(const string& name,
                                        const ACC::APS::ParameterSet& parSet,
                                        const BBSStep* parent = 0);

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // @name Accessor methods
      // @{

      // Return the name of this step.
      string name() const { return itsName; }

      // Return a pointer to the parent of this step.
      const BBSStep* getParent() const { return itsParent; }

      // Make \a parent the parent of this step.
      void setParent(const BBSStep* parent) { itsParent = parent; }

      // Return the selection of baselines for this step.
      Baselines baselines() const { return itsBaselines; }

      // Return which correlation products should be used for this step.
      Correlation correlation() const { return itsCorrelation; }

      // Return the amount of integration that must be applied to the data.
      Integration integration() const { return itsIntegration; }

      // Return the sources in the source model for the current patch.
      vector<string> sources() const { return itsSources; }

      // Return the extra sources outside the current patch.
      vector<string> extraSources() const { return itsExtraSources; }

      // Return a list of instrument models to be used for this step.
      vector<string> instrumentModels() const { return itsInstrumentModels; }

      // @}

      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ACC::APS::ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this,
      // overriding the default values, "inherited" from the parent step
      // object.
      virtual void read(const ACC::APS::ParameterSet& ps);

    protected:
      // Default constructor. Construct an empty BBSStep object and make it a
      // child of the BBSStep object \a parent.
      BBSStep(const BBSStep* parent = 0) : itsParent(parent) {}

      // Construct a BBSStep. \a name identifies the step name in the
      // parameter set file. It does \e not uniquely identify the step \e
      // object being created. The third argument is used to pass a
      // backreference to the parent BBSStep object.
      BBSStep(const string& name, 
	      const ACC::APS::ParameterSet& parSet,
	      const BBSStep* parent);

    private:
      // Implementation of getAllSteps(). The default implementation adds \c
      // this to the vector \a steps.
      // \note This method must be overridden by BBSMultiStep.
      virtual void
      doGetAllSteps(vector< shared_ptr<const BBSStep> >& steps) const;

      // Name of this step.
      string                 itsName;

      // Pointer to the parent of \c *this. All BBSStep objects have a parent,
      // except the top-level BBSStep object. The parent reference is used,
      // among other things, to initialize the data members of the child
      // object with those of its parent.
      const BBSStep*         itsParent;

      // Selection of baselines for this step.
      Baselines              itsBaselines;

      // Parameters describing which correlation products for which
      // polarizations should be used for this step.
      Correlation            itsCorrelation;

      // Parameters describing the amount of integration that must be applied
      // to the data. Integration can be useful to decrease the amount of
      // data.
      Integration            itsIntegration;

      // The sources in the source model for the current patch.
      vector<string>         itsSources;

      // Extra sources outside the current patch that may contribute to the
      // current patch. They should be taken into account in order to improve
      // the predictions of source parameters for the current patch.
      vector<string>         itsExtraSources;

      // A list of instrument models to be used for this step.
      vector<string>         itsInstrumentModels;

      // Write the contents of a BBSStep to an output stream.
      friend ostream& operator<<(ostream&, const BBSStep&);

    };

   
  } // namespace BBS

} // namespace LOFAR

#endif
