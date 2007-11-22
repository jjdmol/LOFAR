//#  GSA_Resources.cc: 
//#
//#  Copyright (C) 2002-2003
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

#define LOFARLOGGER_SUBPACKAGE "SAL"

// GCF/SAL includes
#include  <GSA_Resources.h>

// PVSS includes
#include  <ErrHdl.hxx>

namespace LOFAR {
 namespace GCF {
  namespace PAL {
// Wrapper to read config file
void  GSAResources::init(int &argc, char *argv[])
{
  Resources::setManNum(0);
  begin(argc, argv);

  while ( readSection() || generalSection() )
    ;
  end(argc, argv);
}


// Read the config file.
// Our section is [ApiTest] or [ApiTest_<num>], 
PVSSboolean  GSAResources::readSection()
{
  // Is it our section ? 
  // This will test for [PI] and [PI_<num>]
  if (!isSection("GCF"))
    return PVSS_FALSE;

  // Read next entry
  getNextEntry();
  
  // Loop thru section
  while ( (cfgState != CFG_SECT_START) &&  // Not next section
          (cfgState != CFG_EOF) ) {         // End of config file
    if (!readGeneralKeyWords()) {            // keywords handled in Resources
      ErrHdl::error(ErrClass::PRIO_WARNING,     // not that bad
                    ErrClass::ERR_PARAM,        // wrong parametrization
                    ErrClass::ILLEGAL_KEYWORD,  // illegal Keyword in Res.
                    keyWord);

      // Signal error, so we stop later
      cfgError = PVSS_TRUE;
    }

    getNextEntry();
  }

  // So the loop will stop at the end of the file
  return cfgState != CFG_EOF;
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
