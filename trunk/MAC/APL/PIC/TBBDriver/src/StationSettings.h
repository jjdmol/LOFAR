//#  StationSettings.h: Global station settings
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

#ifndef LOFAR_TBB_STATIONSETTINGS_H
#define LOFAR_TBB_STATIONSETTINGS_H

// \file StationSettings.h
// Global station settings

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace TBB {

// forward declaration
class TBBDriver;

// class_description
// ...
class StationSettings
{
public:
	StationSettings ();
	~StationSettings() {};

	static StationSettings* instance();

	int32 maxTbbBoards();	// RS.MAX_TBBBOARDS
	
	int32 nrTbbBoards();	// RS.N_TBBBOARDS
	
	int32 nrMpsPerBoard();	// const 4
	
	friend class TBBDriver;

protected:	// note TBBDriver must be able to set them
	void setMaxTbbBoards(int32 maxBoards);
	void setNrTbbBoards(int32 nrOfBoards);
	
	

private:
	// Copying is not allowed
	StationSettings(const StationSettings&	that);
	StationSettings& operator=(const StationSettings& that);

	//# --- Datamembers ---
	int32	itsMaxTbbBoards;	// max posible boards
	int32	itsNrTbbBoards;		// nr of installed boards
	int32	itsNrMpsPerBoard;	// nr of Memmory Proccessors on one board
	
	
	
	static StationSettings* theirStationSettings;
};

//# --- inline functions ---
inline	int32 StationSettings::maxTbbBoards()	{ return (itsMaxTbbBoards);   }
inline	int32 StationSettings::nrTbbBoards()	{ return (itsNrTbbBoards); }
inline	int32 StationSettings::nrMpsPerBoard() { return (itsNrMpsPerBoard); }

	} // end TBB namespace
} // end LOFAR namespace

#endif
