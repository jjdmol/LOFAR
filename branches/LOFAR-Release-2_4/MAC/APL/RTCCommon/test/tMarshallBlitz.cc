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

int main (int, char*	argv[])
{
	INIT_LOGGER(argv[0]);

	char	buf[4096];
	size_t		offset(0);

	// blitz array <double>
	blitz::Array<double, 2>		ba1(2,4);
	ba1 = 	10,	11,
			20, 21,
			30, 31,
			40, 41;
	cout << "Testing blitz::Array<double, 2>" << ba1 << endl;
	
	cout << "size = " << MSH_size(ba1) << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_pack(buf, offset, ba1);
	cout << "packed:" << endl;
	hexdump(buf, offset);

	blitz::Array<double, 2>		ba2(2,4);
	offset = 0;
	MSH_unpack(buf, offset, ba2);
	cout << "unpacked: " << ba2 << endl;

	// blitz array <int>
	blitz::Array<int, 2>		emptyArr;
	cout << "Testing EMPTY blitz::Array<int, 2>" << emptyArr << endl;
	
	cout << "size = " << MSH_size(emptyArr) << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_pack(buf, offset, emptyArr);
	cout << "packed:" << endl;
	hexdump(buf, offset);

	blitz::Array<int, 2>		empty2;
	offset = 0;
	MSH_unpack(buf, offset, empty2);
	cout << "unpacked: " << empty2 << endl;

	return (0);
}
