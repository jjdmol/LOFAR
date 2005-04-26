//#  NMUtilities.cc: Utility functions
//#
//#  Copyright (C) 2002-2004
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

#include <Common/LofarLogger.h>
#include <APL/NMUtilities.h>
#include <GCF/PAL/GCF_PVSSInfo.h>

namespace LOFAR
{
using namespace GCF::PAL;  
  namespace ANM
  {
    
INIT_TRACER_CONTEXT(NMUtilities, LOFARLOGGER_PACKAGE);

string NMUtilities::extractNodeName(string propName)
{
  string resName(propName);

  resName.erase(0, GCFPVSSInfo::getLocalSystemName().length() + strlen(":PIC_CEP_"));
  resName.erase(resName.rfind("."));

  return resName;
}
  } // namespace ANM
} // namespace LOFAR
