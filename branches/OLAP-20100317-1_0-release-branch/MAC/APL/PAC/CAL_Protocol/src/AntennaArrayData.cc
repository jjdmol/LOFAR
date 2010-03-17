//#  -*- mode: c++ -*-
//#  AntennaArrayData.cc: implementation of the AntennaArrayData class
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

#include <APL/CAL_Protocol/AntennaArrayData.h>
#include <string>
#include <fstream>

using namespace std;
using namespace LOFAR;
using namespace CAL;
using namespace blitz;

AntennaArrayData::AntennaArrayData()
{
}

AntennaArrayData::~AntennaArrayData()
{
  m_file.close();
}

bool AntennaArrayData::getNextFromFile(string filename)
{
  if (filename != m_filename) {
    // open new file
    if (m_file.is_open()) m_file.close();
    m_file.open(filename.c_str());
    m_filename = filename;
  }
  
  if (!m_file.good()) {
    m_file.close();
    return false;
  }

  // The file may have comment lines at the top. These lines start with '#'
  // and must be skipped, first.
 
  m_name = '#'; 
  while (m_name[0] == '#') { 
    getline(m_file, m_name); // get name
    // No empty lines are allowed.
    if ("" == m_name) {
      m_file.close();
      return false;
    }
  }

  m_file >> m_geoloc; // get geographical location 1-d array with 3 elements
  if ((1 != m_geoloc.dimensions())
      || (3 != m_geoloc.extent(firstDim))) {
    return false;
  }
  m_file >> m_positions; // get positions
  m_file.ignore(80,'\n'); // read away newline

  return true;
}


