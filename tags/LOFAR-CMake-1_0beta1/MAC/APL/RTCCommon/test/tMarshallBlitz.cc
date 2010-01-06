//#  tMarshallBlitz.cc: test pack and unpack macros
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
#include <Common/lofar_bitset.h>
#include <Common/lofar_map.h>
#include <Common/hexdump.h>
#include <APL/RTCCommon/MarshallBlitz.h>

using namespace LOFAR;

int main (int	argc, char*	argv[])
{
	char	buf[4096];
	int		offset(0);

	// blitz array <double>
	blitz::Array<double, 2>		ba1(2,4);
	ba1 = 	10,	11,
			20, 21,
			30, 31,
			40, 41;
	cout << "Testing blitz::Array<double, 2>" << ba1 << endl;
	
	cout << "size = " << MSH_ARRAY_SIZE(ba1, double) << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_PACK_ARRAY(buf, offset, ba1, double);
	cout << "packed:" << endl;
	hexdump(buf, offset);

	blitz::Array<double, 2>		ba2(2,4);
	offset = 0;
	MSH_UNPACK_ARRAY(buf, offset, ba2, double, 2);
	cout << "unpacked: " << ba2 << endl;

	// blitz array <int>
	blitz::Array<int, 2>		emptyArr;
	cout << "Testing EMPTY blitz::Array<int, 2>" << emptyArr << endl;
	
	cout << "size = " << MSH_ARRAY_SIZE(emptyArr, int) << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_PACK_ARRAY(buf, offset, emptyArr, int);
	cout << "packed:" << endl;
	hexdump(buf, offset);

	blitz::Array<int, 2>		empty2;
	offset = 0;
	MSH_UNPACK_ARRAY(buf, offset, empty2, int, 2);
	cout << "unpacked: " << empty2 << endl;

	return (0);
}
