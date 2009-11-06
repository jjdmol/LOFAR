//#  tAPLUtilities.cc
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
#include <APL/APLCommon/APLUtilities.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;

int main (int	argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	
	cout << "16.0GB : " << byteSize(16.0*1024*1024*1024) << endl;
	cout << " 8.0GB : " << byteSize(8.0*1024*1024*1024) << endl;
	cout << " 4.0GB : " << byteSize(4.0*1024*1024*1024) << endl;
	cout << " 2.5GB : " << byteSize(2.5*1024*1024*1024) << endl;
	cout << " 4.0MB : " << byteSize(4*1024*1024) << endl;
	cout << " 2.5MB : " << byteSize(2560*1024) << endl;
	cout << " 4.0KB : " << byteSize(4*1024) << endl;
	cout << " 2.5KB : " << byteSize(2560) << endl;
	cout << "  512B : " << byteSize(512) << endl;

	return (0);
}

