//#  -*- mode: c++ -*-
//#  SourceData.cc: implementation of the data class for sources
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
#include "SourceData.h"

using namespace std;
using namespace LOFAR;
using namespace CAL;

SourceData::SourceData()
{
}

SourceData::~SourceData()
{
  m_file.close();
}

bool SourceData::getNextFromFile(std::string filename)
{
  string fluxstring;

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

  getline(m_file, m_name); // read name
  if ("" == m_name) {
    m_file.close();
    return false;
  }

  m_file >> m_ra;              // read RA
  m_file >> m_dec;             // read DEC
  m_file.ignore(80,'\n');
  getline(m_file, fluxstring); // read flux array
  istringstream fluxstream(fluxstring);
  fluxstream >> m_flux;

  return true;
}

