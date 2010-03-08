//#  tBlitz.cc: test blitz functions
//#
//#  Copyright (C) 2007
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
#include <Common/hexdump.h>
#include <blitz/array.h>

using namespace LOFAR;
using namespace blitz;

int main (int	argc, char*	argv[])
{
	// test empty blitz array
	cout << "Testing 2 dimensional empty blitz array..." << endl;
	Array<int,2>	emptyArr;	// two dimensional empty array.
	cout << "#dimensions    : " << emptyArr.dimensions() << endl;
	cout << "extent(first)  : " << emptyArr.extent(firstDim) << endl;
	cout << "extent(second) : " << emptyArr.extent(secondDim) << endl;
	cout << "#elements      : " << emptyArr.numElements() << endl;
	cout << "contiguous     : " << (emptyArr.isStorageContiguous() ? "yes" :  "no") << endl;
	
	// test 3x10 array
	cout << "Testing 2 dimensional blitz array 3x10 ..." << endl;
	Array<int,2>	anArr(2,1);	// two dimensional empty array.
	anArr.resize(3,10);
	cout << "#dimensions    : " << anArr.dimensions() << endl;
	cout << "extent(first)  : " << anArr.extent(firstDim) << endl;
	cout << "extent(second) : " << anArr.extent(secondDim) << endl;
	cout << "#elements      : " << anArr.numElements() << endl;
	cout << "contiguous     : " << (anArr.isStorageContiguous() ? "yes" :  "no") << endl;
	
	return (0);
}
