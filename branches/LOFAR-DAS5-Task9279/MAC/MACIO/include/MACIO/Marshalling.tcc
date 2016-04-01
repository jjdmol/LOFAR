//#  -*- mode: c++ -*-
//#
//#  Marshalling.h: Macros for packing/unpacking some classes
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: Marshalling.tcc 963 2015-03-18 14:27:57Z overeem $

#ifndef MACIO_MARSHALLING_TCC_
#define MACIO_MARSHALLING_TCC_

#include <Common/LofarTypes.h>
#include <Common/KVpair.h>
#include <boost/dynamic_bitset.hpp>
#include <cstring>
#include <map>
#include <string>
#include <sstream>
#include <vector>

using LOFAR::int32;
using LOFAR::uint32;
using LOFAR::KVpair;

// Basic template
template<typename T> inline uint32 MSH_size(const T	&tVar)
{
	return (sizeof(tVar));
}

template<typename T> inline uint32 MSH_pack(char *bufPtr, uint32 offset, const T &tVar)
{
	uint32	size = MSH_size(tVar);
	memcpy(bufPtr + offset, &tVar, size);
	return (offset + size);
}

template<typename T> inline uint32 MSH_unpack(const char *bufPtr, uint32 offset, T &tVar)
{
	uint32	size = MSH_size(tVar);
	memcpy(&tVar, bufPtr + offset, size);
	return (offset + size);
}

// Specialistion for std::string
template<> inline uint32 MSH_size<std::string>(const std::string &tVar)
{
	return (sizeof(int32) + tVar.size() + sizeof(char));
}

template <> inline uint32 MSH_pack<std::string>(char *bufPtr, uint32 offset, const std::string &tVar)
{
	uint32	nrChars = tVar.size();
	memcpy(bufPtr + offset, &nrChars, sizeof(nrChars));
	offset += sizeof(nrChars);
	memcpy(bufPtr + offset, tVar.data(), nrChars);
	offset += nrChars;
    memset(bufPtr + offset, 0, 1);
	offset++;
	return (offset);
}

template<> inline uint32 MSH_unpack<std::string>(const char *bufPtr, uint32 offset, std::string &tVar)
{
	uint32	nrChars;
	memcpy(&nrChars, bufPtr + offset, sizeof(nrChars));
	offset += sizeof(nrChars);
	tVar= std::string(bufPtr+offset);
	offset += (nrChars + 1);
	return (offset);
}

// Specialistion for KVpair
template<> inline uint32 MSH_size<KVpair>(const KVpair &tVar)
{
	return (sizeof(tVar.valueType) + sizeof(tVar.timestamp) + MSH_size(tVar.first) + MSH_size(tVar.second));
}

template <> inline uint32 MSH_pack<KVpair>(char *bufPtr, uint32 offset, const KVpair &tVar)
{
	offset = MSH_pack(bufPtr, offset, tVar.valueType);
	offset = MSH_pack(bufPtr, offset, tVar.timestamp);
	offset = MSH_pack(bufPtr, offset, tVar.first);
	offset = MSH_pack(bufPtr, offset, tVar.second);
	return (offset);
}

template<> inline uint32 MSH_unpack<KVpair>(const char *bufPtr, uint32 offset, KVpair &tVar)
{
	offset = MSH_unpack(bufPtr, offset, tVar.valueType);
	offset = MSH_unpack(bufPtr, offset, tVar.timestamp);
	offset = MSH_unpack(bufPtr, offset, tVar.first);
	offset = MSH_unpack(bufPtr, offset, tVar.second);
	return (offset);
}


// Specialistion for boost::dynamic_bitset
template<> inline uint32 MSH_size<boost::dynamic_bitset<> >(const boost::dynamic_bitset<> &tVar)
{
	return (sizeof(int32) + tVar.size() + sizeof(char));
}

// Specialisation for boost::dynamic_bitset
template <> inline uint32 MSH_pack<boost::dynamic_bitset<> >(char *bufPtr, uint32 offset, const boost::dynamic_bitset<> &tVar)
{
	int32	nrBits = tVar.size();
	memcpy(bufPtr + offset, &nrBits, sizeof(nrBits));
	offset += sizeof(nrBits);
	std::stringstream  tmpbuf(std::stringstream::in | std::stringstream::out);
	tmpbuf << tVar;
	memcpy(bufPtr + offset, tmpbuf.str().data(), nrBits);
	offset += nrBits;
    memset(bufPtr + offset, 0, 1);
	offset++;
	return (offset);
}

template<> uint32 inline MSH_unpack<boost::dynamic_bitset<> >(const char *bufPtr, uint32 offset, boost::dynamic_bitset<> &tVar)
{
	int32	nrBits;
	memcpy(&nrBits, bufPtr + offset, sizeof(nrBits));
	offset += sizeof(nrBits);
	tVar = boost::dynamic_bitset<>(std::string(bufPtr+offset));
	offset += (nrBits + 1);
	return (offset);
}

// basics for std::vector<T>
template<typename T> uint32 MSH_size(const std::vector<T> &tVar)
{
	uint32 sizevar = sizeof(int32);
	typename std::vector<T>::const_iterator	iter = tVar.begin();
	typename std::vector<T>::const_iterator	end  = tVar.end();
	while (iter != end) {
		sizevar += MSH_size(*iter);
		iter++;
	}
    return (sizevar);
}

template<typename T> uint32 MSH_pack(char *bufPtr, uint32	offset, const std::vector<T> &tVar)
{
	int32	nrElem = tVar.size();
	memcpy((char*)bufPtr + offset, &nrElem, sizeof(int32));
	offset += sizeof(int32);

	typename std::vector<T>::const_iterator	iter = tVar.begin();
	typename std::vector<T>::const_iterator	end  = tVar.end();
	while (iter != end) {
		offset = MSH_pack(bufPtr, offset, *iter);
		iter++;
	}
	return (offset);
}

template<typename T> uint32 MSH_unpack(const char* bufPtr, uint32 offset, std::vector<T> &tVar)
{
	int32	nrElem = 0;
	memcpy(&nrElem, (char*)bufPtr + offset, sizeof(nrElem));
	offset += sizeof(nrElem);

	for (int elem = 0; elem < nrElem; elem++) {
		T	elem1;
		offset = MSH_unpack(bufPtr, offset, elem1);
		tVar.push_back(elem1);
	}
	return (offset);
}

// basics for std::map<std::string,T*>
template<typename T> uint32 MSH_size(const std::map<std::string,T*> &tVar)
{
	uint32 sizevar = sizeof(int32);
	typename std::map<std::string,T*>::const_iterator	iter = tVar.begin();
	typename std::map<std::string,T*>::const_iterator	end  = tVar.end();
	while (iter != end) {
		sizevar += (MSH_size(iter->first) + iter->second->getSize());
		iter++;
	}
    return (sizevar);
}

template<typename T> uint32 MSH_pack(char *bufPtr, uint32	offset, const std::map<std::string,T*> &tVar)
{
	int32	nrElem = tVar.size();
	memcpy((char*)bufPtr + offset, &nrElem, sizeof(int32));
	offset += sizeof(int32);

	typename std::map<std::string,T*>::const_iterator	iter = tVar.begin();
	typename std::map<std::string,T*>::const_iterator	end  = tVar.end();
	while (iter != end) {
		offset = MSH_pack(bufPtr, offset, iter->first);
		offset += iter->second->pack(bufPtr + offset);
		iter++;
	}
	return (offset);
}

template<typename T> uint32 MSH_unpack(const char* bufPtr, uint32 offset, std::map<std::string,T*> &tVar)
{
	int32	nrElem = 0;
	memcpy(&nrElem, (char*)bufPtr + offset, sizeof(nrElem));
	offset += sizeof(nrElem);

	for (int elem = 0; elem < nrElem; elem++) {
		std::string	elem1;
		offset = MSH_unpack(bufPtr, offset, elem1);
		tVar[elem1] = new T;
		offset += tVar[elem1]->unpack(bufPtr + offset);
	}
	return (offset);
}

// basics for std::map<std::string,T>
template<typename T> uint32 MSH_size(const std::map<std::string,T> &tVar)
{
	uint32 sizevar = sizeof(int32);
	typename std::map<std::string,T>::const_iterator	iter = tVar.begin();
	typename std::map<std::string,T>::const_iterator	end  = tVar.end();
	while (iter != end) {
		sizevar += (MSH_size(iter->first) + iter->second.getSize());
		iter++;
	}
    return (sizevar);
}

template<typename T> uint32 MSH_pack(char *bufPtr, uint32	offset, const std::map<std::string,T> &tVar)
{
	int32	nrElem = tVar.size();
	memcpy((char*)bufPtr + offset, &nrElem, sizeof(int32));
	offset += sizeof(int32);

	typename std::map<std::string,T>::const_iterator	iter = tVar.begin();
	typename std::map<std::string,T>::const_iterator	end  = tVar.end();
	while (iter != end) {
		offset = MSH_pack(bufPtr, offset, iter->first);
		offset += iter->second.pack(bufPtr + offset);
		iter++;
	}
	return (offset);
}

template<typename T> uint32 MSH_unpack(const char* bufPtr, uint32 offset, std::map<std::string,T> &tVar)
{
	int32	nrElem = 0;
	memcpy(&nrElem, (char*)bufPtr + offset, sizeof(nrElem));
	offset += sizeof(nrElem);

	for (int elem = 0; elem < nrElem; elem++) {
		std::string	elem1;
		offset = MSH_unpack(bufPtr, offset, elem1);
		offset += tVar[elem1].unpack(bufPtr + offset);
	}
	return (offset);
}

#endif /* MACIO_MARSHALLING_TCC_ */
