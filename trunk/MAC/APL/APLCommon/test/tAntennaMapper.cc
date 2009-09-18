//#  tAntennaMapper.cc
//#
//#  Copyright (C) 2008
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
#include <APL/APLCommon/AntennaMapper.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;
//using LOFAR::APLCommon::AntennaMapper;

void doTest(int	antNr, int antType, AntennaMapper&	AM)
{
	cout << ((antType == AntennaMapper::AT_LBA) ? "LBA" : "HBA") << " antenna " << antNr 
		 << " is connected to Xrcu " << AM.XRCU(antNr) << " and Yrcu " 
		 << AM.YRCU(antNr) << " using input " << AM.RCUinput(antNr, antType) << endl;
}

int main (int	argc, char* argv[]) 
{
	//						rcus, lbas, hbas
	AntennaMapper	AMCore  (96, 96, 48);
	AntennaMapper	AMRemote(96, 96, 0);
	AntennaMapper	AMEurope(192,96, 48);

	cout << endl << "Core station with half HW and full LBA and HBA" << endl;
	doTest(5, 	 AntennaMapper::AT_LBA, AMCore);
	doTest(5+48, AntennaMapper::AT_LBA, AMCore);
	doTest(48,   AntennaMapper::AT_LBA, AMCore);
	doTest(5, 	 AntennaMapper::AT_HBA, AMCore);

	cout << endl << "Remote station with half HW and full LBA and NO HBA" << endl;
	doTest(5, 	 AntennaMapper::AT_LBA, AMRemote);
	doTest(5+48, AntennaMapper::AT_LBA, AMRemote);
	doTest(5, 	 AntennaMapper::AT_HBA, AMRemote);

	cout << endl << "Europa station with full HW and full LBA and HBA" << endl;
	doTest(5, 	 AntennaMapper::AT_LBA, AMEurope);
	doTest(5+48, AntennaMapper::AT_LBA, AMEurope);
	doTest(5, 	 AntennaMapper::AT_HBA, AMEurope);
	
	return (0);
}

