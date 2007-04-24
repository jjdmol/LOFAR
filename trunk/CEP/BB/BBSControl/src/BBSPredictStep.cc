//#  BBSPredictStep.cc: 
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>
#include <BBSControl/BBSPredictStep.h>
#include <BBSControl/CommandVisitor.h>

namespace LOFAR
{
  namespace BBS
  {
    void BBSPredictStep::accept(CommandVisitor &visitor) const
    {
        visitor.visit(*this);
    }


    const string& BBSPredictStep::type() const 
    {
      static const string theType("Predict");
      return theType;
    }

    const string& BBSPredictStep::operation() const
    {
      static string theOperation("Predict");
      return theOperation;
    }

  } // namespace BBS

} // namespace LOFAR
    
    
