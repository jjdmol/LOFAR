//#  VHECRsettings.h: class/struct that holds the Observation information
//#
//#  Copyright (C) 2006
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
//#  $Id: VHECRsettings.h 10634 2007-11-01 11:32:07Z overeem $

#ifndef VHECRSETTINGS_H
#define VHECRSETTINGS_H

// \file
// class/struct that holds the Observation information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/ParameterSet.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_bitset.h>
#include <Common/LofarConstants.h>
#include <APL/APLCommon/AntennaField.h>

namespace LOFAR {
  namespace VHECR {
//# Forward Declarations
//class forward;

// The VHECRsettings class is a class
// for holding VHECR related information.
// It can be instantiated with a parset containing Observation information.
class VHECRsettings
{
public:
	VHECRsettings();
	~VHECRsettings() { };
	
	explicit VHECRsettings (ParameterSet* aParSet);
	
	typedef bitset<MAX_RCUS> RCUset_t;
	
	/*
	\brief Put the parameters from the parset file into the VHECRTast object
	
	\param AntennaSet - E.g. "LBA_INNER"
	\param AntennaPositionsFile - Absolute path to the file with the antenna positions.
	\param Clock - Sampling clock speed in MHz (i.e.160 or 200) 
	\param NoCoincChann - The number of channels needed to detect a coincidence.
	\param CoincidenceTime - The time-range in seconds during which triggers are considered part of a coincidence.
	\param DoDirectionFit - Do a direction fit: none [0], simple [1], more fancy [2] 
	\param MinElevation - Minimum elevation to accept a trigger in degrees.
	\param MaxFitVariance - Maximum variance (``badness of fit'') of the direction fit to still accept a trigger. 
	\param ParamExtension - String with "keyword=value;" pairs for additional parameters during development.
    */
    string        antennaSet;
	AntennaField*  antennaField;
	int           clockFreq;
	
	int           noCoincChann;
	double        coincidenceTime;
	int           doDirectionFit;
	double        minElevation;
	double        maxFitVariance;
	
	//vector<string> ParamExtension=""  //TODO
	
	// RCUsetSelected are the selected RCUs in TBB observation
	RCUset_t RCUsetSelected;
	// RCUsetActive are the selected RCUs who can generate triggers
	RCUset_t RCUsetActive;
	
	bool isLoaded();
	//# print function for operator<<
	ostream&	print(ostream&	os) const;

private:
	bool itsSettingsLoaded;
	uint16 _windowNr(const string&		wdwName);

};

inline bool VHECRsettings::isLoaded() { return(itsSettingsLoaded); }
//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const VHECRsettings& settings)
{	
	return (settings.print(os));
}

  } // namespace StationCU
} // namespace LOFAR

#endif
