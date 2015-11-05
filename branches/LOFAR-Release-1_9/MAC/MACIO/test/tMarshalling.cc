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
#include <Common/lofar_vector.h>
#include <Common/hexdump.h>
#include <MACIO/Marshalling.tcc>
#include "tMarshalling.h"


using namespace LOFAR;
using namespace std;

namespace LOFAR {

SubArray::SubArray(int i, double d, string s) :
	someInt(i), someDouble(d), someString(s)
{
}

size_t SubArray::getSize() const {
	return (sizeof(int) + sizeof(double) + MSH_size(someString));
}

size_t SubArray::pack(char*	buffer) const {
	size_t	offset = 0;
	memcpy(buffer+offset, &someInt, sizeof(int));
	offset += sizeof (int);
	memcpy(buffer+offset, &someDouble, sizeof(double));
	offset += sizeof (double);
	MSH_pack(buffer, offset, someString);

	return (offset);
}

size_t SubArray::unpack(const char*	buffer) {
	size_t offset = 0;
	memcpy(&someInt, buffer+offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&someDouble, buffer+offset, sizeof(double));
	offset += sizeof(double);
	MSH_unpack(buffer, offset, someString);
	return (offset);
}

SubArrayNC::SubArrayNC(int i, double d, string s) :
	someInt(i), someDouble(d), someString(s)
{
}

size_t SubArrayNC::getSize() const {
	return (sizeof(int) + sizeof(double) + MSH_size(someString));
}

size_t SubArrayNC::pack(char*	buffer) const {
	size_t	offset = 0;
	memcpy(buffer+offset, &someInt, sizeof(int));
	offset += sizeof (int);
	memcpy(buffer+offset, &someDouble, sizeof(double));
	offset += sizeof (double);
	MSH_pack(buffer, offset, someString);

	return (offset);
}

size_t SubArrayNC::unpack(const char*	buffer) {
	size_t	offset = 0;
	memcpy(&someInt, buffer+offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&someDouble, buffer+offset, sizeof(double));
	offset += sizeof(double);
	MSH_unpack(buffer, offset, someString);
	return (offset);
}


} // namespace LOFAR

int main (int	/*argc*/, char**	/*argv[]*/)
{
	// string test
	string	tstString("Dit is een teststring");
	cout << "Testing string: " << tstString << endl;
	cout << "size = " << MSH_size(tstString) << endl;

	char	buf[4096];
	size_t	offset1(0);
	MSH_pack(buf, offset1, tstString);
	cout << "packed: " << endl;
	hexdump(buf, offset1);

	string	newString;
	size_t offset2(0);
	MSH_unpack(buf, offset2, newString);
	cout << "unpacked: " << newString << endl;
	ASSERTSTR (offset1 == offset2 && tstString == newString, "Failure 1 in strings");


	// bitset test1
	LOFAR::bitset<35>	bs1;
	bs1.reset();
	bs1.set(0);
	bs1.set(5);
	bs1.set(20);
	cout << "Testing bitset<35>: " << bs1 << endl;
	cout << "size = " << MSH_size(bs1) << endl;

	bzero(buf, 4096);
	offset1 = 0;
	MSH_pack(buf, offset1, bs1);
	cout << "packed:" << endl;
	hexdump(buf, offset1);

	LOFAR::bitset<35>	bs2;
	offset2 = 0;
	MSH_unpack(buf, offset2, bs2);
	cout << "size = " << offset2 << endl;
	cout << "unpacked: " << bs2 << endl;
	ASSERTSTR (offset1 == offset2, "Failure in offset var of bitsets");
	ASSERTSTR (bs1 == bs2, "Failure in bitsets");


	// dynamic bitmap 
	boost::dynamic_bitset<>	dbs1(25);
	dbs1.set(0);
	dbs1.set(5);
	dbs1.set(6);
	dbs1.set(20);
	cout << "Testing dynamic_bitset<25>" << endl << dbs1 << endl;
	cout << "size = " << MSH_size(dbs1) << endl;

	bzero(buf, 4096);
	offset1 = 0;
	MSH_pack(&buf[0], offset1, dbs1);
	cout << "packed:" << endl;
	hexdump(buf, offset1);

	boost::dynamic_bitset<>	dbs2;
	offset2 = 0;
	MSH_unpack(&buf[0], offset2, dbs2);
	cout << "unpacked: " << dbs2 << endl;
	cout << "size = " << dbs2.size() << endl;
	cout << "offset1 = " << offset1 << endl;
	cout << "offset2 = " << offset2 << endl;
	ASSERTSTR (offset1 == offset2 && dbs1 == dbs2, "Failure in dynamic bitsets");


	// SubArray		
	SubArray		SA1(25, 3.14, "stringetje");
	cout << "Testing SubArray class:" << SA1.someInt << "," << SA1.someDouble 
										<< "," << SA1.someString << endl;
	
	size_t	size(SA1.getSize());
	cout << "size = " << size << endl;

	bzero(buf, 4096);
	size_t offset = 0;
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
	ms1["eerste_item"] = SubArray(25, 3.14, "stringetje");
	ms1["tweede_item"] = SubArray(0, 32, "ejtegnirts");
	cout << "Testing map<string, SubArray>: " << endl;
	map<string, SubArray>::iterator	iter = ms1.begin();
	map<string, SubArray>::iterator	end  = ms1.end();
	while (iter != end) {
		cout << "map[" << iter->first << "]:" << iter->second.someInt << "," << 
					iter->second.someDouble << "," << iter->second.someString << endl; 
		iter++;
	}

	size_t	mapsize;
	mapsize = MSH_size(ms1);
//	MSH_SIZE_MAP_STRING_CLASS(mapsize, ms1, SubArray);
	cout << "size = " << mapsize << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_pack(buf, offset, ms1);
//	MSH_PACK_MAP_STRING_CLASS(buf, offset, ms1, SubArray);
	cout << "packed:" << endl;
	hexdump(buf, mapsize);

	map<string, SubArray>		ms2;
	offset = 0;
	MSH_unpack(buf, offset, ms2);
	cout << "Unpacked map<string, SubArray>: " << endl;
	map<string, SubArray>::iterator	iter2 = ms2.begin();
	map<string, SubArray>::iterator	end2  = ms2.end();
	while (iter2 != end2) {
		cout << "map[" << iter2->first << "]:" << iter2->second.someInt << "," << 
					iter2->second.someDouble << "," << iter2->second.someString << endl; 
		iter2++;
	}


	// map<string, subArrayNC*>
	map<string, SubArrayNC*>		msanc1;
	msanc1["eerste_item"] = new SubArrayNC(25, 3.14, "stringetje");
	msanc1["tweede_item"] = new SubArrayNC(0, 32, "ejtegnirts");
	cout << "Testing map<string, SubArrayNC*>: " << endl;
	map<string, SubArrayNC*>::iterator	iternc = msanc1.begin();
	map<string, SubArrayNC*>::iterator	endnc  = msanc1.end();
	while (iternc != endnc) {
		cout << "map[" << iternc->first << "]:" << iternc->second->someInt << "," << 
					iternc->second->someDouble << "," << iternc->second->someString << endl; 
		iternc++;
	}

	size_t	mapncsize;
	mapncsize = MSH_size(msanc1);
	cout << "size = " << mapncsize << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_pack(buf, offset, msanc1);
	cout << "packed:" << endl;
	hexdump(buf, mapncsize);

	map<string, SubArrayNC*>		msanc2;
	offset = 0;
	MSH_unpack(buf, offset, msanc2);
	cout << "Unpacked map<string, SubArrayNC*>: " << endl;
	iternc = msanc2.begin();
	endnc  = msanc2.end();
	while (iternc != endnc) {
		cout << "map[" << iternc->first << "]:" << iternc->second->someInt << "," << 
					iternc->second->someDouble << "," << iternc->second->someString << endl; 
		iternc++;
	}


	// vector<string>
	vector<string>		sv1;
	sv1.push_back("piet hein");
	sv1.push_back("nelson");
	sv1.push_back("van Swieten");
	cout << "Testing vector<string>" << endl;
	vector<string>::iterator	itersv = sv1.begin();
	vector<string>::iterator	endsv  = sv1.end();
	int	i = 0;
	while (itersv != endsv) {
		cout << "vector[" << i << "]:" << *itersv << endl;
		i++;
		itersv++;
	}

	size_t svsize;
	svsize = MSH_size(sv1);
	cout << "size = " << svsize << endl;

	bzero(buf, 4096);
	offset1 = 0;
	MSH_pack(buf, offset1, sv1);
	cout << "packed:" << endl;
	hexdump(buf, svsize);

	vector<string>	sv2;
	offset2 = 0;
	MSH_unpack(buf, offset2, sv2);
	cout << "Unpacked vector<string>" << endl;
	itersv = sv2.begin();
	endsv  = sv2.end();
	i = 0;
	while (itersv != endsv) {
		cout << "vector[" << i << "]:" << *itersv << endl;
		i++;
		itersv++;
	}
	ASSERTSTR (offset1 == offset2 && sv1 == sv2, "Failure in vector<string>");


	// vector<double>
	vector<double>		dv1;
	dv1.push_back(3.145926);
	dv1.push_back(0.0);
	dv1.push_back(0.00004567);
	cout << "Testing vector<double>" << endl;
	vector<double>::iterator	iterdv = dv1.begin();
	vector<double>::iterator	enddv  = dv1.end();
	i = 0;
	while (iterdv != enddv) {
		cout << "vector[" << i << "]:" << *iterdv << endl;
		i++;
		iterdv++;
	}

	size_t dvsize;
	dvsize = MSH_size(dv1);
	cout << "size = " << dvsize << endl;

	bzero(buf, 4096);
	offset = 0;
	MSH_pack(buf, offset, dv1);
	cout << "packed:" << endl;
	hexdump(buf, dvsize);

	vector<double>	dv2;
	offset = 0;
	MSH_unpack(buf, offset, dv2);
	cout << "Unpacked vector<double>" << endl;
	iterdv = dv2.begin();
	enddv  = dv2.end();
	i = 0;
	while (iterdv != enddv) {
		cout << "vector[" << i << "]:" << *iterdv << endl;
		i++;
		iterdv++;
	}
	ASSERTSTR (offset1 == offset2 && dv1 == dv2, "Failure in vector<double>");
}
