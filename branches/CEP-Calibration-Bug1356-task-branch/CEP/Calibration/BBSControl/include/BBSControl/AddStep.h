//# AddStep.h: Derived leaf class of the Step composite pattern.
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
//# $Id: AddStep.h 12371 2008-12-23 13:18:31Z loose $

#ifndef LOFAR_BBSCONTROL_BBSADDSTEP_H
#define LOFAR_BBSCONTROL_BBSADDSTEP_H

// \file
// Derived leaf class of the Step composite pattern.

//# Includes
#include <BBSControl/SingleStep.h>

namespace LOFAR
{
  namespace BBS
  {
    // \ingroup BBSControl
    // @{

    // This is a so-called \e leaf class in the Step composite pattern (see
    // Gamma, 1995).
    // \note Currently, a %AddStep is in fact identical to a
    // SingleStep. Only the classType() method is overridden.
    class AddStep : public SingleStep
    {
    public:
      AddStep(const Step* parent = 0) : SingleStep(parent) {}

      AddStep(const string& name, 
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
