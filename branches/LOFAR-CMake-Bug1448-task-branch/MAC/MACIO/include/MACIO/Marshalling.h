//#  -*- mode: c++ -*-
//#
//#  Marshalling.h: Macros for packing/unpacking some classes
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

#ifndef MACIO_MARSHALLING_H_
#define MACIO_MARSHALLING_H_

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_map.h>

// SIZE string
#define MSH_STRING_SIZE(stdstring) \
	(sizeof(uint32) + (stdstring).size() * sizeof(char))

// PACK string
#define MSH_PACK_STRING(bufptr, offset, stdstring)	\
do {	\
	/* pack stdstring with length */	\
	uint32 size = (stdstring).size() * sizeof(char);	\
	memcpy(((char*)(bufptr)) + (offset), &size, sizeof(size));	\
	offset += sizeof(size);	\
	memcpy(((char*)(bufptr)) + (offset), (stdstring).c_str(), size * sizeof(char));	\
	offset += size * sizeof(char);	\
} while (0)

// UNPACK string
#define MSH_UNPACK_STRING(bufptr, offset, stdstring)	\
do {	\
	uint32 size = 0;	\
	memcpy(&size, ((char*)(bufptr)) + (offset), sizeof(size));	\
	offset += sizeof(size);	\
	char stringbuf[size + 1];	\
	memcpy(stringbuf, ((char*)(bufptr)) + (offset), size * sizeof(char));	\
	stringbuf[size] = '\0';	\
	(stdstring) = string(stringbuf); /* cast to std::string */	\
	offset += size * sizeof(char);	\
} while (0)

// SIZE bitset<...>
#define MSH_BITSET_SIZE(bset) sizeof(bset)

// PACK bitset<...>
#define MSH_PACK_BITSET(buffer, offset, bset) \
do { \
	memcpy(((char*)(buffer))+(offset), &(bset), sizeof(bset)); \
	offset += sizeof(bset); \
} while(0)

// UNPACK bitset<...>
#define MSH_UNPACK_BITSET(buffer,offset,bset) \
do { \
	memcpy(&bset, ((char*)(buffer))+(offset), sizeof(bset)); \
    offset += sizeof(bset); \
} while(0)

// SIZE map<string, class_with_getSize>
#define MSH_SIZE_MAP_STRING_CLASS(sizevar, themap, datatype)	\
do {	\
	sizevar = sizeof(int32);	\
	map<string, datatype>::iterator	iter = themap.begin();	\
	map<string, datatype>::iterator	end  = themap.end();	\
	while (iter != end) {	\
		sizevar += MSH_STRING_SIZE(iter->first);	\
		sizevar += iter->second.getSize();	\
		iter++;	\
	}	\
} while (0)
	

// PACK map<string, class_with_pack>
#define MSH_PACK_MAP_STRING_CLASS(bufptr, offset, themap, datatype)	\
do {	\
	int32	nrElem = themap.size();	\
	memcpy(((char*)(bufptr)) + (offset), &nrElem, sizeof(int32));	\
	offset += sizeof(int32);	\
	\
	map<string, datatype>::iterator	iter = themap.begin();	\
	map<string, datatype>::iterator	end  = themap.end();	\
	while (iter != end) {	\
		MSH_PACK_STRING(bufptr, offset, iter->first);	\
		offset += iter->second.pack(bufptr + offset);	\
		iter++;	\
	}	\
} while (0)

// UNPACK map<string, class_with_unpack>
#define MSH_UNPACK_MAP_STRING_CLASS(bufptr, offset, themap, datatype)	\
do {	\
	int32	nrElem = 0;	\
	memcpy(&nrElem, ((char*)(bufptr)) + (offset), sizeof(nrElem));	\
	offset += sizeof(nrElem);	\
	\
	for (int elem = 0; elem < nrElem; elem++) {	\
		string	elem1; \
		MSH_UNPACK_STRING(bufptr, offset, elem1);	\
		offset += themap[elem1].unpack(bufptr + offset);	\
	}	\
} while (0)

// SIZE map<string, ptr2class_with_getSize>
#define MSH_SIZE_MAP_STRING_CLASSPTR(sizevar, themap, datatype)	\
do {	\
	sizevar = sizeof(int32);	\
	map<string, datatype*>::iterator	iter = themap.begin();	\
	map<string, datatype*>::iterator	end  = themap.end();	\
	while (iter != end) {	\
		sizevar += MSH_STRING_SIZE(iter->first);	\
		sizevar += iter->second->getSize();	\
		iter++;	\
	}	\
} while (0)
	

// PACK map<string, ptr2class_with_pack>
#define MSH_PACK_MAP_STRING_CLASSPTR(bufptr, offset, themap, datatype)	\
do {	\
	int32	nrElem = themap.size();	\
	memcpy(((char*)(bufptr)) + (offset), &nrElem, sizeof(int32));	\
	offset += sizeof(int32);	\
	\
	map<string, datatype*>::iterator	iter = themap.begin();	\
	map<string, datatype*>::iterator	end  = themap.end();	\
	while (iter != end) {	\
		MSH_PACK_STRING(bufptr, offset, iter->first);	\
		offset += iter->second->pack((char*)bufptr + offset);	\
		iter++;	\
	}	\
} while (0)

// UNPACK map<string, ptr2class_with_unpack>
#define MSH_UNPACK_MAP_STRING_CLASSPTR(bufptr, offset, themap, datatype)	\
do {	\
	int32	nrElem = 0;	\
	memcpy(&nrElem, ((char*)(bufptr)) + (offset), sizeof(nrElem));	\
	offset += sizeof(nrElem);	\
	\
	for (int elem = 0; elem < nrElem; elem++) {	\
		string	elem1; \
		MSH_UNPACK_STRING(bufptr, offset, elem1);	\
		themap[elem1] = new datatype; \
		offset += themap[elem1]->unpack((char*)bufptr + offset);	\
	}	\
} while (0)

// SIZE vector<string>>
#define MSH_SIZE_VECTOR_STRING(sizevar, thevector)	\
do {	\
	sizevar = sizeof(int32);	\
	vector<string>::iterator	iter = thevector.begin();	\
	vector<string>::iterator	end  = thevector.end();	\
	while (iter != end) {	\
		sizevar += MSH_STRING_SIZE(*iter);	\
		iter++;	\
	}	\
} while (0)
	

// PACK vector<string>
#define MSH_PACK_VECTOR_STRING(bufptr, offset, thevector)	\
do {	\
	int32	nrElem = thevector.size();	\
	memcpy(((char*)(bufptr)) + (offset), &nrElem, sizeof(int32));	\
	offset += sizeof(int32);	\
	\
	vector<string>::iterator	iter = thevector.begin();	\
	vector<string>::iterator	end  = thevector.end();	\
	while (iter != end) {	\
		MSH_PACK_STRING(bufptr, offset, *iter);	\
		iter++;	\
	}	\
} while (0)

// UNPACK vector<string>
#define MSH_UNPACK_VECTOR_STRING(bufptr, offset, thevector)	\
do {	\
	int32	nrElem = 0;	\
	memcpy(&nrElem, ((char*)(bufptr)) + (offset), sizeof(nrElem));	\
	offset += sizeof(nrElem);	\
	\
	for (int elem = 0; elem < nrElem; elem++) {	\
		string	elem1; \
		MSH_UNPACK_STRING(bufptr, offset, elem1);	\
		thevector.push_back(elem1); \
	}	\
} while (0)

#endif /* MARSHALLING_H_ */
