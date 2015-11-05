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
//#  $Id: Marshalling.h 19796 2012-01-17 10:06:03Z overeem $

#ifndef MACIO_MARSHALLING_TCC_
#define MACIO_MARSHALLING_TCC_

#include <Common/LofarTypes.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <boost/dynamic_bitset.hpp>
#include <sstream>

using namespace LOFAR;

// Basic template
template<typename T> inline size_t MSH_size(const T	&tVar)
{
	return (sizeof(tVar));
}

template<typename T> inline void MSH_pack(char *bufPtr, size_t &offset, const T &tVar)
{
	size_t	size = MSH_size(tVar);
	memcpy(bufPtr + offset, &tVar, size);
	offset += size;
}

template<typename T> inline void MSH_unpack(const char *bufPtr, size_t &offset, T &tVar)
{
	size_t	size = MSH_size(tVar);
	memcpy(&tVar, bufPtr + offset, size);
	offset += size;
}

// Specialistion for string
template<> inline size_t MSH_size<string>(const string &tVar)
{
	return (sizeof(int32) + tVar.size() + sizeof(char));
}

template <> inline void MSH_pack<string>(char *bufPtr, size_t &offset, const string &tVar)
{
	int32	nrChars = tVar.size();
	memcpy(bufPtr + offset, &nrChars, sizeof(nrChars));
	offset += sizeof(nrChars);
	memcpy(bufPtr + offset, tVar.data(), nrChars);
	offset += nrChars;
    memset(bufPtr + offset, 0, 1);
	offset++;
}

template<> inline void MSH_unpack<string>(const char *bufPtr, size_t &offset, string &tVar)
{
	int32	nrChars;
	memcpy(&nrChars, bufPtr + offset, sizeof(nrChars));
	offset += sizeof(nrChars);
	tVar= string(bufPtr+offset);
	offset += nrChars + 1;
}

// Specialistion for boost::dynamic_bitset
template<> inline size_t MSH_size<boost::dynamic_bitset<> >(const boost::dynamic_bitset<> &tVar)
{
	return (sizeof(int32) + tVar.size() + sizeof(char));
}

// Specialisation for boost::dynamic_bitset
template <> inline void MSH_pack<boost::dynamic_bitset<> >(char *bufPtr, size_t &offset, const boost::dynamic_bitset<> &tVar)
{
	int32	nrBits = tVar.size();
	memcpy(bufPtr + offset, &nrBits, sizeof(nrBits));
	offset += sizeof(nrBits);
	stringstream  tmpbuf(stringstream::in | stringstream::out);
	tmpbuf << tVar;
	memcpy(bufPtr + offset, tmpbuf.str().data(), nrBits);
	offset += nrBits;
    memset(bufPtr + offset, 0, 1);
	offset++;
}

template<> void inline MSH_unpack<boost::dynamic_bitset<> >(const char *bufPtr, size_t &offset, boost::dynamic_bitset<> &tVar)
{
	int32	nrBits;
	memcpy(&nrBits, bufPtr + offset, sizeof(nrBits));
	offset += sizeof(nrBits);
//	tVar << std::string(bufPtr+offset);
	tVar = boost::dynamic_bitset<>(string(bufPtr+offset));
	offset += nrBits + 1;
}

// basics for vector<T>
template<typename T> size_t MSH_size(const vector<T> &tVar)
{
	size_t sizevar = sizeof(int32);
	typename vector<T>::const_iterator	iter = tVar.begin();
	typename vector<T>::const_iterator	end  = tVar.end();
	while (iter != end) {
		sizevar += MSH_size(*iter);
		iter++;
	}
    return (sizevar);
} 
	
template<typename T> void MSH_pack(char *bufPtr, size_t	&offset, const vector<T> &tVar)
{
	int32	nrElem = tVar.size();
	memcpy((char*)bufPtr + offset, &nrElem, sizeof(int32));
	offset += sizeof(int32);
	
	typename vector<T>::const_iterator	iter = tVar.begin();
	typename vector<T>::const_iterator	end  = tVar.end();
	while (iter != end) {
		MSH_pack(bufPtr, offset, *iter);
		iter++;
	}
}

template<typename T> void MSH_unpack(const char* bufPtr, size_t &offset, vector<T> &tVar)
{
	int32	nrElem = 0;
	memcpy(&nrElem, (char*)bufPtr + offset, sizeof(nrElem));
	offset += sizeof(nrElem);

	for (int elem = 0; elem < nrElem; elem++) {
		T	elem1;
		MSH_unpack(bufPtr, offset, elem1);
		tVar.push_back(elem1);
	}
} 

// basics for map<string,T*>
template<typename T> size_t MSH_size(const map<string,T*> &tVar)
{
	size_t sizevar = sizeof(int32);
	typename map<string,T*>::const_iterator	iter = tVar.begin();
	typename map<string,T*>::const_iterator	end  = tVar.end();
	while (iter != end) {
		sizevar += MSH_size(iter->first) + iter->second->getSize();
		iter++;
	}
    return (sizevar);
} 
	
template<typename T> void MSH_pack(char *bufPtr, size_t	&offset, const map<string,T*> &tVar)
{
	int32	nrElem = tVar.size();
	memcpy((char*)bufPtr + offset, &nrElem, sizeof(int32));
	offset += sizeof(int32);
	
	typename map<string,T*>::const_iterator	iter = tVar.begin();
	typename map<string,T*>::const_iterator	end  = tVar.end();
	while (iter != end) {
		MSH_pack(bufPtr, offset, iter->first);
		offset += iter->second->pack(bufPtr + offset);
		iter++;
	}
}

template<typename T> void MSH_unpack(const char* bufPtr, size_t &offset, map<string,T*> &tVar)
{
	int32	nrElem = 0;
	memcpy(&nrElem, (char*)bufPtr + offset, sizeof(nrElem));
	offset += sizeof(nrElem);

	for (int elem = 0; elem < nrElem; elem++) {
		string	elem1;
		MSH_unpack(bufPtr, offset, elem1);
		tVar[elem1] = new T;
		offset += tVar[elem1]->unpack(bufPtr + offset);
	}
} 

// basics for map<string,T>
template<typename T> size_t MSH_size(const map<string,T> &tVar)
{
	size_t sizevar = sizeof(int32);
	typename map<string,T>::const_iterator	iter = tVar.begin();
	typename map<string,T>::const_iterator	end  = tVar.end();
	while (iter != end) {
		sizevar += MSH_size(iter->first) + iter->second.getSize();
		iter++;
	}
    return (sizevar);
} 
	
template<typename T> void MSH_pack(char *bufPtr, size_t	&offset, const map<string,T> &tVar)
{
	int32	nrElem = tVar.size();
	memcpy((char*)bufPtr + offset, &nrElem, sizeof(int32));
	offset += sizeof(int32);
	
	typename map<string,T>::const_iterator	iter = tVar.begin();
	typename map<string,T>::const_iterator	end  = tVar.end();
	while (iter != end) {
		MSH_pack(bufPtr, offset, iter->first);
		offset += iter->second.pack(bufPtr + offset);
		iter++;
	}
}

template<typename T> void MSH_unpack(const char* bufPtr, size_t &offset, map<string,T> &tVar)
{
	int32	nrElem = 0;
	memcpy(&nrElem, (char*)bufPtr + offset, sizeof(nrElem));
	offset += sizeof(nrElem);

	for (int elem = 0; elem < nrElem; elem++) {
		string	elem1;
		MSH_unpack(bufPtr, offset, elem1);
		offset += tVar[elem1].unpack(bufPtr + offset);
	}
} 

#endif /* MARSHALLING_H_ */
