//# MWLocalStep.h: Base class for a step to process a local MW command
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

#ifndef LOFAR_MWCOMMON_MWLOCALSTEP_H
#define LOFAR_MWCOMMON_MWLOCALSTEP_H

// @file
// @brief Base classes for local MW commands
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/MWStep.h>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Base class for a step to process a local MW command.

  // This class defines a class that serves as the base class for a
  // local MW step. A local MW step is a step that can be executed
  // directly by a worker without the need of interaction between workers.
  // An example is a subtract or correct. A solve is not a local step,
  // because it requires interaction between workers.

  class MWLocalStep: public MWStep
  {
  public:
    MWLocalStep()
    {}

    virtual ~MWLocalStep();

    // Visit the object, so the visitor can process it.
    // The default implementation uses the MWStepVisitor::visitlocal
    // function.
    virtual void visit (MWStepVisitor&) const;
  };


}} //# end namespaces

#endif
