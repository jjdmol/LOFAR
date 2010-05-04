//# MWStepVisitor.h: Base visitor class to visit an MWStep hierarchy
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_MWCOMMON_MWSTEPVISITOR_H
#define LOFAR_MWCOMMON_MWSTEPVISITOR_H

// @file
// @brief Base visitor class to visit an MWStep hierarchy.
// @author Ger van Diepen (diepen AT astron nl)

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

namespace LOFAR { namespace CEP {

  //# Forward Declarations
  class MWStep;
  class MWMultiStep;
  class MWGlobalStep;
  class MWLocalStep;


  // @ingroup MWCommon
  // @brief Base visitor class to visit an MWStep hierarchy.

  // This is a class to traverse a MWStep composite using the visitor
  // pattern (see Design Patterns, Gamma et al, 1995).
  // It is the base class for all visitor classes.
  //
  // For each step in the composite, a visitXXX function is called where
  // XXX is the step type. In this way many different visitors can be
  // used without the need of implementing such functions in the MWStep
  // classes. The downside is that a visitYYY function needs to be added
  // to all visitor classes if an new step type YYY is created.

  class MWStepVisitor
  {
  public:
    // Define the visit function for an arbitrary MWStep object.
    typedef void VisitFunc (MWStepVisitor&, const MWStep&);

    // Destructor.
    virtual ~MWStepVisitor();

    // Visit the different predefined step types.
    // The default implementation throws an exception that the step cannot
    // be handled.
    virtual void visitMulti  (const MWMultiStep&);
    virtual void visitGlobal (const MWGlobalStep&);
    virtual void visitLocal  (const MWLocalStep&);

    // Visit for an arbitrary \a MWStep type.
    // The default implementation calls the \a VisitFunc function which
    // is registered for the type name of the \a MWStep object.
    // If not registered, it calls visitStep.
    virtual void visit (const MWStep&);

    // Visit for an arbitrary \a MWStep type.
    // The default implementation throws an exception that the step cannot
    // be handled.
    virtual void visitStep (const MWStep&);

    // Register a visit function for an MWStep with the given name.
    // This can be used for other types of MWStep objects.
    // The given function will usually be a static function in a derived
    // visitor class calling a class member function. It can look like:
    // <pre>
    //   void MyVisitor::doXXX (MWStepVisitor& visitor, const MWStep& step)
    //     { dynamic_cast<MyVisitor&>(visitor).visitXXX(
    //                     dynamic_cast<const MWSTepXXX&>(step)); }
    // </pre>
    // The casts are kind of ugly, but unavoidable.
    // The doXXX functions can be registered by the constructor.
    void registerVisit (const std::string& name, VisitFunc*);

  private:
    std::map<std::string, VisitFunc*> itsMap;
  };
  
}} //# end namespaces

#endif
