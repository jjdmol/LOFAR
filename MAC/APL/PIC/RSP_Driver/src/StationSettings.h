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

#ifndef LOFAR_RSP_STATIONSETTINGS_H
#define LOFAR_RSP_STATIONSETTINGS_H

// \file StationSettings.h
// Global station settings

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace RSP {

// \addtogroup RSP
// @{

// forward declaration
class RSPDriver;

// class_description
// ...
class StationSettings
{
public:
	StationSettings ();
	~StationSettings() {};

	static StationSettings* instance();

	int32 maxRspBoards();	// RS.N_RSPBOARDS
	int32 nrBlpsPerBoard();	// MEPHeader::N_BLPS
	int32 nrRcusPerBoard();	// MEPHeader::N_BLPS * N_POL

	int32 nrRspBoards();	// 1 | RS.N_RSPBOARDS depending on OPERATION_MODE
	int32 nrBlps();			// nrRspBoards * nrBlpsPerBoard
	int32 nrRcus();			// nrRspBoards * nrRcusPerBoard

	bool  hasSplitter();

	ostream& print (ostream& os) const;

	friend class RSPDriver;

protected:	// note RSPDriver must be able to set them
	void setNrBlpsPerBoard(int32 nrBlps);
	void setNrRspBoards   (int32 nrBlps);
	void setMaxRspBoards  (int32 nrBlps);
	void setSplitter      (bool hasSplitter) { itsHasSplitter = hasSplitter; }

private:
	// Copying is not allowed
	StationSettings(const StationSettings&	that);
	StationSettings& operator=(const StationSettings& that);

	//# --- Datamembers ---
	int32	itsMaxRspBoards;	// constants
	int32	itsNrBlpsPerBoard;
	int32	itsNrRcusPerBoard;

	int32	itsNrRspBoards;		// values depend on OPERATION_MODE
	int32	itsNrBlps;
	int32	itsNrRcus;

	bool	itsHasSplitter;

	static StationSettings* theirStationSettings;
};

//# --- inline functions ---
inline	int32 StationSettings::maxRspBoards()	{ return (itsMaxRspBoards);   }
inline	int32 StationSettings::nrBlpsPerBoard() { return (itsNrBlpsPerBoard); }
inline	int32 StationSettings::nrRcusPerBoard() { return (itsNrRcusPerBoard); }

inline	int32 StationSettings::nrRspBoards()	{ return (itsNrRspBoards); }
inline	int32 StationSettings::nrBlps()			{ return (itsNrBlps); }
inline	int32 StationSettings::nrRcus()			{ return (itsNrRcus); }

inline	bool  StationSettings::hasSplitter()	{ return(itsHasSplitter); }

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
