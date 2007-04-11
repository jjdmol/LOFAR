//#  BBSRefitStep.cc: 
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
#include <BBSControl/BBSRefitStep.h>
#include <BBSControl/CommandHandler.h>

namespace LOFAR
{
  namespace BBS
  {
//     // Register BBSRefitStep with the BBSStepFactory. Use an anonymous
//     // namespace. This ensures that the variable `dummy' gets its own private
//     // storage area and is only visible in this compilation unit.
//     namespace
//     {
//       bool dummy = BBSStepFactory::instance().
// 	registerClass<BBSRefitStep>("BBSRefitStep");
//     }

    void BBSRefitStep::accept(CommandHandler &handler) const
    {
        handler.handle(*this);
    }

    const string& BBSRefitStep::operation() const 
    {
      static string theOperation("Refit");
      return theOperation;
    }
  
//     const string& BBSRefitStep::classType() const 
//     {
//       static string theType("BBSRefitStep");
//       return theType;
//     }
  
    const string& BBSRefitStep::type() const 
    {
      static const string theType("RefitStep");
      return theType;
    }

  } // namespace BBS

} // namespace LOFAR
    
    
