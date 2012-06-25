//# RefitStep.cc: 
//#
//# Copyright (C) 2002-2007
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

#include <lofar_config.h>
#include <BBSControl/RefitStep.h>
#include <BBSControl/CommandVisitor.h>
#include <Common/ParameterSet.h>

namespace LOFAR
{
  namespace BBS
  {

    RefitStep::RefitStep(const string& name, 
                         const ParameterSet& parSet,
                         const Step* parent) :
      SingleStep(name, parent)
    {
      read(parSet.makeSubset("Step." + name + "."));
    }


    CommandResult RefitStep::accept(CommandVisitor &visitor) const
    {
      return visitor.visit(*this);
    }


    const string& RefitStep::type() const 
    {
      static const string theType("Refit");
      return theType;
    }


    const string& RefitStep::operation() const
    {
      static string theOperation("Refit");
      return theOperation;
    }

  } // namespace BBS

} // namespace LOFAR
