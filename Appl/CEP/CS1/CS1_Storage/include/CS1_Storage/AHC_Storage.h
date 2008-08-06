//#  AHC_Storage.h: interpretes commands from ACC and executes
//#                                 them on the TinyApplicationHolder object
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

#ifndef AHC_STORAGE_H
#define AHC_STORAGE_H

// \file
// Interprets commands from ACC and executes them on the TinyApplicationHolder
// object

#include <Common/LofarLogger.h>

#include <tinyCEP/Profiler.h>

#include <tinyCEP/ApplicationHolderController.h>

namespace LOFAR { 
  namespace CS1 {

// \addtogroup tinyCEP
// @{
class AHC_Storage : public ApplicationHolderController
{
public:
  // Default constructable
  AHC_Storage(TinyApplicationHolder& stationCorrelator, int noRuns = 100);

  // Destructor
  ~AHC_Storage();
/* todo: check this code during SAS/MAC tests
  // Command to control the application processes.
  tribool	pause    (const	string&	condition) ;
  tribool	quit  	 ();
*/  
  
protected:
  // Copying is not allowed
  AHC_Storage (const AHC_Storage& that);
  AHC_Storage& 	operator=(const AHC_Storage& that);
};

// @}
  }
}  
#endif
