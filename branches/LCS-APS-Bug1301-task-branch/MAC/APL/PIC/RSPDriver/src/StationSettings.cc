//#  StationSettings.cc: Global station settings
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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <StationSettings.h>

namespace LOFAR {
  namespace RSP {

//
// Initialize singleton
//
StationSettings* StationSettings::theirStationSettings = 0;

StationSettings* StationSettings::instance()
{
	if (theirStationSettings == 0) {
		theirStationSettings = new StationSettings();
	}
	return (theirStationSettings);
}

//
// Default constructor
//
StationSettings::StationSettings() :
	itsMaxRspBoards(0),
	itsNrBlpsPerBoard(0),
	itsNrRcusPerBoard(0),
	itsNrRspBoards(0),
	itsNrBlps(0),
	itsNrRcus(0)
{

}

//
// setNrBlpsPerBoard
//
void StationSettings::setNrBlpsPerBoard (int32 nrBlps)
{
	itsNrBlpsPerBoard = nrBlps;
	itsNrRcusPerBoard = itsNrBlpsPerBoard * MEPHeader::N_POL;
	itsNrBlps         = itsNrBlpsPerBoard * itsNrRspBoards;
	itsNrRcus         = itsNrRcusPerBoard * itsNrRspBoards;
}

//
// setNrRspBoards
//
void StationSettings::setNrRspBoards    (int32 nrRspBoards)
{
	itsNrRspBoards = nrRspBoards;
	ASSERTSTR (itsNrRspBoards <= itsMaxRspBoards, 
			formatString("Range conflict in nr of RSP boards (%d<=%d)",
			itsNrRspBoards, itsMaxRspBoards));

	itsNrBlps      = itsNrBlpsPerBoard * itsNrRspBoards;
	itsNrRcus      = itsNrRcusPerBoard * itsNrRspBoards;
}

//
// setMaxRspBoards
//
void StationSettings::setMaxRspBoards   (int32 nrRspBoards)
{
	itsMaxRspBoards = nrRspBoards;
	ASSERTSTR (itsNrRspBoards <= itsMaxRspBoards, 
			formatString("Rangeconflict in nr of RSP boards (%d<=%d)",
			itsNrRspBoards, itsMaxRspBoards));
}

//
// print
//
ostream& StationSettings::print (ostream& os) const
{
	os << "Max RSPboards: " << itsMaxRspBoards   << endl;
	os << "Nr RSPboards : " << itsNrRspBoards    << endl;
	os << "Nr BLPs/board: " << itsNrBlpsPerBoard << endl;
	os << "Nr RCUs/board: " << itsNrRcusPerBoard << endl;
	os << "Nr BLPs      : " << itsNrBlps         << endl;
	os << "Nr RCUs      : " << itsNrRcus         << endl;

	return (os);
}

  } // namespace PACKAGE
} // namespace LOFAR
