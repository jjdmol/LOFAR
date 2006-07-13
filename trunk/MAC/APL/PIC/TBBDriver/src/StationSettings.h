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

// \addtogroup RSP
// @{

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

	int32 maxTbbBoards();	// RS.N_RSPBOARDS
	
	int32 nrTbbBoards();	// 1 | RS.N_RSPBOARDS depending on OPERATION_MODE
	
	ostream& print (ostream& os) const;

	friend class TBBDriver;

protected:	// note TBBDriver must be able to set them
	void setNrTbbBoards    (int32 nrBlps);
	void setMaxTbbBoards   (int32 nrBlps);

private:
	// Copying is not allowed
	StationSettings(const StationSettings&	that);
	StationSettings& operator=(const StationSettings& that);

	//# --- Datamembers ---
	int32	itsMaxTbbBoards;	// constants
	int32	itsNrMpsPerBoard;
	int32	itsNrTbbBoards;		// values depend on OPERATION_MODE
	int32	itsNrMp;					// values depend on OPERATION_MODE
	
	static StationSettings* theirStationSettings;
};

//# --- inline functions ---
inline	int32 StationSettings::maxTbbBoards()	{ return (itsMaxTbbBoards);   }
inline	int32 StationSettings::nrMpsPerBoard() { return (itsNrMpsPerBoard); }

inline	int32 StationSettings::nrTbbBoards()	{ return (itsNrTbbBoards); }
inline	int32 StationSettings::nrMp()		{ return (itsNrMp); }


//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const StationSettings& aSS)
{	
	return (aSS.print(os));
}

// @}
  } // namespace RSP
} // namespace LOFAR

#endif
