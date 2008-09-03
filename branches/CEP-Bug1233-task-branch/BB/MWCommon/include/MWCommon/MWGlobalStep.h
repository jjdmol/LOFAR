//# MWGlobalStep.h: Base classes for global MW commands (like subtract)
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_MWCOMMON_MWGLOBALSTEP_H
#define LOFAR_MWCOMMON_MWGLOBALSTEP_H

// @file
// @brief Base classes for global MW commands (like subtract)
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/MWStep.h>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Base class for a step to process a global MW command.

  // This class defines a class that serves as the base class for a
  // global MW step. A global MW step is a step that cannot be executed
  // directly by a worker without the need of interaction between workers.

  class MWGlobalStep: public MWStep
  {
  public:
    MWGlobalStep()
    {}

    virtual ~MWGlobalStep();

    // Visit the object, so the visitor can process it.
    // The default implementation uses the MWStepVisitor::visitGlobal
    // function.
    virtual void visit (MWStepVisitor&) const;
  };


}} //# end namespaces

#endif
