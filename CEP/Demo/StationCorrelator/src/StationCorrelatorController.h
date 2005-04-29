//#  StationCorrelatorController.h: interpretes commands from ACC and executes them on the StationCorrelator object
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#ifndef STATIONCORRELATORCONTROLLER_H
#define STATIONCORRELATORCONTROLLER_H

#include <Common/LofarLogger.h>

#include <tinyCEP/Profiler.h>
#include <ACC/ProcControlServer.h>
#include <ACC/ParameterSet.h>
#include <ACC/ProcessControl.h>

#include <StationCorrelator.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

class StationCorrelatorController : public ProcessControl
{
public:
  // Default constructable
  StationCorrelatorController(StationCorrelator& stationCorrelator, int noRuns = 100);

  // Destructor
  virtual ~StationCorrelatorController();

  // Command to control the application processes.
  virtual bool	define   () const;
  virtual bool	init     () const;
  virtual bool	run      () const;
  virtual bool	pause    (const	string&	condition) const;
  virtual bool	quit  	 () const;
  virtual bool	snapshot (const string&	destination) const;
  virtual bool	recover  (const string&	source) const;
  virtual bool	reinit	 (const string&	configID) const;
  virtual string askInfo   (const string& 	keylist) const;
  
protected:
  StationCorrelator& itsStationCorrelator;
  int itsNoRuns;

  // Copying is not allowed
  StationCorrelatorController(const StationCorrelatorController& that);
  StationCorrelatorController& 	operator=(const StationCorrelatorController& that);
};

#endif
