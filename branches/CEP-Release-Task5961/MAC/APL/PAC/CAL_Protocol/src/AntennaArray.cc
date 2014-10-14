//#  -*- mode: c++ -*-
//#  AntennaArray.cc: implementation of the AntennaArray class.
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/CAL_Protocol/AntennaArray.h>
#include <APL/CAL_Protocol/AntennaArrayData.h>

#include <blitz/array.h>
#include <fstream>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;

AntennaArray::AntennaArray()
{
  // dummy arrays
  m_name = "uninitialized";
  m_geoloc.resize(3);
  m_geoloc = 0.0;
  m_pos.resize(1,1,1);
  m_pos = 0.0;
  m_rcuindex.resize(1,1);
  m_rcuindex = 0;
}

AntennaArray::AntennaArray( string                  name,
							const Array<double, 1>& geoloc,
							const Array<double, 3>& pos,
							const Array<int16,  2>* rcuindex) :
	m_name(name), 
	m_geoloc(geoloc), 
	m_pos(pos)
{
	m_rcuindex.resize(m_pos.extent(firstDim), m_pos.extent(secondDim));

	if (rcuindex) {
		m_rcuindex = *rcuindex;
	}
	else {
		firstIndex i;
		m_rcuindex = i;
	}
}

AntennaArray::~AntennaArray()
{
}

AntennaArray& AntennaArray::operator=(const AntennaArray& rhs)
{
  if (this != &rhs) {

    // copy members
    m_name = rhs.m_name;

    m_geoloc.resize(rhs.m_geoloc.shape());
    m_geoloc = rhs.m_geoloc;

    m_pos.resize(rhs.m_pos.shape());
    m_pos = rhs.m_pos;

    m_rcuindex.resize(rhs.m_rcuindex.shape());
    m_rcuindex = rhs.m_rcuindex;

  }
  return *this;
}

//
// ---------------- Class ANTENNAARRAYS ---------------
AntennaArrays::AntennaArrays()
{
}

AntennaArrays::~AntennaArrays()
{
  for (map<string, const AntennaArray*>::const_iterator it = m_arrays.begin();
       it != m_arrays.end(); ++it) {
    if ((*it).second) delete (*it).second;
  }
}

const AntennaArray* AntennaArrays::getByName(std::string name)
{
  // find AntennaArray
  map<string,const AntennaArray*>::iterator it = m_arrays.find(name);

  if (it != m_arrays.end()) {
    return (*it).second;
  }

  return 0;
}

void AntennaArrays::getAll(std::string url)
{
  AntennaArrayData arraydata;

  while (arraydata.getNextFromFile(url)) {
    AntennaArray* newarray = new AntennaArray(arraydata.getName(),
					      arraydata.getGeoLoc(),
					      arraydata.getPositions());

    m_arrays[arraydata.getName()] = newarray;
    LOG_INFO_STR("Read array definition '" << arraydata.getName() << "' from file '" << url << "'");
  }
}

//
// getNameList() : vector<string>
//
vector<string> AntennaArrays::getNameList()
{
	vector<string>	names;
	map<string,const AntennaArray*>::iterator	iter = m_arrays.begin();
	map<string,const AntennaArray*>::iterator	end  = m_arrays.end();

	while (iter != end) {
		names.push_back(iter->first);
		iter++;
	}

	return (names);
}

