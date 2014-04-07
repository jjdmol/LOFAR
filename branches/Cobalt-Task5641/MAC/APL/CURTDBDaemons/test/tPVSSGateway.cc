//#  tPVSSGateway.cc
//#
//#  Copyright (C) 2014
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
//#  $Id: tProtocol.cc 15266 2010-03-18 08:52:04Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <MACIO/RTmetadata.h>

using namespace LOFAR;
using namespace LOFAR::MACIO;

int main (int	argc, char* argv[]) 
{
	if (argc < 3) {
		cout << "Syntax: tPVSSGateway DPname value";
		return (1);
	}

	INIT_LOGGER(argv[0]);

	RTmetadata	myLogger(3125, "ObservationControl", "localhost");

	myLogger.log(argv[1], strtod(argv[2],NULL));
	return (0);
}

