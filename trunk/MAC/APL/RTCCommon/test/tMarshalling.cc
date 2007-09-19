//#  tMarshalling.cc: test pack and unpack macros
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
#include <APL/RTCCommon/Marshalling.h>
#include "tMarshalling.h"

using namespace LOFAR;

namespace LOFAR {

SubArray::SubArray(int i, double d, string s) :
	someInt(i), someDouble(d), someString(s)
{}

unsigned int SubArray::getSize() {
	return (sizeof(int) + sizeof(double) + MSH_STRING_SIZE(someString));
}

unsigned int SubArray::pack(void*	buffer) {
	unsigned int	offset = 0;
	memcpy(((char*)(buffer)+offset), &someInt, sizeof(int));
	offset += sizeof (int);
	memcpy(((char*)(buffer)+offset), &someDouble, sizeof(double));
	offset += sizeof (double);
	MSH_PACK_STRING(buffer, offset, someString);

	return (offset);
}

unsigned int SubArray::unpack(void*	buffer) {
	unsigned int	offset = 0;
	memcpy(&someInt, ((char*)(buffer))+offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&someDouble, ((char*)(buffer))+offset, sizeof(double));
	offset += sizeof(double);
	MSH_UNPACK_STRING(buffer, offset, someString);
	return (offset);
}


} // namespace LOFAR

int main (int	argc, char*	argv[])
{
	// string test
	string	tstString("Dit is een teststring");
	cout << "Testing string: " << tstString << endl;

	cout << "size = " << MSH_STRING_SIZE(tstString) << endl;

	char	buf[4096];
	int32	offset(0);
	MSH_PACK_STRING(buf, offset, tstString);
	cout << "packed: " << endl;
	hexdump(buf, offset);

	string	newString;
	offset = 0;
	MSH_UNPACK_STRING(buf, offset, newString);
	cout << "unpacked: " << newString << endl;

	// bitmap test1
	bitset<32>	bs1;
	bs1.reset();
	bs1.set(0);
	bs1.set(5);
	bs1.set(20);
	cout << "Testing bitset<32>" << bs1 << endl;
	
	cout << "size = " << MSH_BITSET_SIZE(bs1) << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_PACK_BITSET(buf, offset, bs1);
	cout << "packed:" << endl;
	hexdump(buf, offset);

	bitset<32>	bs2;
	offset = 0;
	MSH_UNPACK_BITSET(buf, offset, bs2);
	cout << "unpacked: " << bs2 << endl;

	// blitz array <double>
	blitz::Array<double, 2>		ba1(2,4);
	ba1 = 	10,	11,
			20, 21,
			30, 31,
			40, 41;
	cout << "Testing blitz::Array<double, 2>>" << ba1 << endl;
	
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

	// SubArray		
	SubArray		SA1(25, 3.14, "stringetje");
	cout << "Testing SubArray class:" << SA1.someInt << "," << SA1.someDouble 
										<< "," << SA1.someString << endl;
	
	unsigned int	size(SA1.getSize());
	cout << "size = " << size << endl;

	bzero(buf, 4096);
	offset = 0;
	SA1.pack(buf);
	cout << "packed:" << endl;
	hexdump(buf, size);

	SubArray	SA2;
	offset = 0;
	SA2.unpack(buf);
	cout << "unpacked: " << SA2.someInt << "," << SA2.someDouble 
										<< "," << SA2.someString << endl;


	// map<string, subArray>
	map<string, SubArray>		ms1;
	ms1["eerste_item"] = SA1;
	SubArray		SA3(0, 32, "ejtegnirts");
	ms1["tweede_item"] = SA3;
	cout << "Testing map<string, SubArray>: " << endl;
	map<string, SubArray>::iterator	iter = ms1.begin();
	map<string, SubArray>::iterator	end  = ms1.end();
	while (iter != end) {
		cout << "map[" << iter->first << "]:" << iter->second.someInt << "," << 
					iter->second.someDouble << "," << iter->second.someString << endl; 
		iter++;
	}

	unsigned int	mapsize;
	MSH_SIZE_MAP_STRING_CLASS(mapsize, ms1, SubArray);
	cout << "size = " << mapsize << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_PACK_MAP_STRING_CLASS(buf, offset, ms1, SubArray);
	cout << "packed:" << endl;
	hexdump(buf, mapsize);

	map<string, SubArray>		ms2;
	offset = 0;
	MSH_UNPACK_MAP_STRING_CLASS(buf, offset, ms2, SubArray);
	cout << "Unpacked map<string, SubArray>: " << endl;
	map<string, SubArray>::iterator	iter2 = ms2.begin();
	map<string, SubArray>::iterator	end2  = ms2.end();
	while (iter2 != end2) {
		cout << "map[" << iter2->first << "]:" << iter2->second.someInt << "," << 
					iter2->second.someDouble << "," << iter2->second.someString << endl; 
		iter2++;
	}

	return (0);
}
