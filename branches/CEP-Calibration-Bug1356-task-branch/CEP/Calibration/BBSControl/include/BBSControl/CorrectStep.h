//# CorrectStep.h: Derived leaf class of the Step composite pattern.
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

#ifndef LOFAR_BBSCONTROL_BBSCORRECTSTEP_H
#define LOFAR_BBSCONTROL_BBSCORRECTSTEP_H

// \file
// Derived leaf class of the Step composite pattern.

//# Includes
#include <BBSControl/SingleStep.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // This is a so-called \e leaf class in the Step composite pattern (see
    // Gamma, 1995).
    class CorrectStep : public SingleStep
    {
    public:
      CorrectStep(const Step* parent = 0) :
        SingleStep(parent)
      {
      }

      CorrectStep(const string& name,
                  const ParameterSet& parSet,
                  const Step* parent);

      // Accept a CommandVisitor that wants to process \c *this.
      virtual CommandResult accept(CommandVisitor &visitor) const;

      // Return the operation type of \c *this as a string.
      virtual const string& operation() const;

      // Return the command type of \c *this as a string.
      virtual const string& type() const;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
