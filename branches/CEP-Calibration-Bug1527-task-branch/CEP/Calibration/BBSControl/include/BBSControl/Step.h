//# Step.h: Base component class of the Step composite pattern.
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

#ifndef LOFAR_BBSCONTROL_BBSSTEP_H
#define LOFAR_BBSCONTROL_BBSSTEP_H

// \file
// Base component class of the Step composite pattern.

//# Includes
#include <BBSControl/Command.h>
#include <BBSControl/Types.h>
#include <BBSKernel/ModelConfig.h>

#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
  //# Forward Declarations.
  class ParameterSet;

  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // This is the so-called \e component class in the Step composite
    // pattern (see Gamma, 1995). It is the base class for all Step
    // classes, both composite and leaf classes. It has data members that are
    // common to all Step classes.
    //
    // \todo Make the Step class family exception safe. Currently, methods
    // like Step::create and Step::deserialize operate on and return raw
    // pointers. When an exception occurs within these methods, memory will be
    // leaked. The solution is to use, e.g., boost::shared_ptr. We must be
    // careful though, not to create circular dependencies, since that will
    // cause the pointer to never be freed. This can happen since a Step
    // stores a pointer to its parent as backreference. Here, we should
    // probably use a boost::weak_ptr. See Bug #906
    class Step : public Command,
                 public enable_shared_from_this<Step>
    {
    public:
      // Destructor.
      virtual ~Step();

      // Return the full name of this step. The full name consists of the name
      // of this step, preceeded by that of its parent, etc., separated by
      // dots.
      string fullName() const;

      // Create a new step object. The new step can either be a SingleStep
      // or a MultiStep object. This is determined by examining the
      // parameter set \a parSet. If this set contains a key
      // <tt>Step.<em>name</em>.Steps</tt>, then \a aName is a MultiStep,
      // otherwise it is a SingleStep. The third, optional, argument is used
      // to pass a backreference to the parent Step object.
      static shared_ptr<Step> create(const string& name,
                                     const ParameterSet& parSet,
                                     const Step* parent = 0);

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // @name Accessor methods
      // @{

      // Return the name of this step.
      string name() const { return itsName; }

      // Return a pointer to the parent of this step.
      const Step* getParent() const { return itsParent; }

      // Make \a parent the parent of this step.
      void setParent(const Step* parent) { itsParent = parent; }

//      // Return the selection of baselines for this step.
//      Baselines baselines() const { return itsBaselines; }

//      // Return which correlation products should be used for this step.
//      CorrelationFilter correlation() const
//      { return itsCorrelationFilter; }

      // Return the data selection.
      Selection selection() const { return itsSelection; }

      // Return the model configuration.
      ModelConfig modelConfig() const { return itsModelConfig; }

      // @}

      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this,
      // overriding the default values, "inherited" from the parent step
      // object.
      virtual void read(const ParameterSet& ps);

    protected:
      // Default constructor. Construct an empty Step object and make it a
      // child of the Step object \a parent.
      Step(const Step* parent = 0) : itsParent(parent) {}

      // Construct a Step. \a name identifies the step name in the
      // parameter set. It does \e not uniquely identify the step \e
      // object being created. The third argument is used to pass a
      // backreference to the parent Step object.
      Step(const string& name, const Step* parent);

    private:
      // Name of this step.
      string            itsName;

      // Pointer to the parent of \c *this. All Step objects have a parent,
      // except the top-level Step object. The parent reference is used,
      // among other things, to initialize the data members of the child
      // object with those of its parent.
      const Step*       itsParent;

      Selection         itsSelection;

//      // Selection of baselines for this step.
//      Baselines         itsBaselines;

//      // Parameters describing which correlation products for which
//      // polarizations should be used for this step.
//      CorrelationFilter itsCorrelationFilter;

      // Model configuration options as specified in the parameter set file.
      ModelConfig       itsModelConfig;

      // Write the contents of a Step to an output stream.
      friend ostream& operator<<(ostream&, const Step&);
    };

  } // namespace BBS

} // namespace LOFAR

#endif
