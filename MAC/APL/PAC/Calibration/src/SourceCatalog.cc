//#  -*- mode: c++ -*-
//#  SourceCatalog.cc: implementation of the SourceCatalog class.
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

#include "SourceCatalog.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <fstream>

using namespace CAL;
using namespace std;
using namespace blitz;

SourceCatalog::SourceCatalog(string name) : m_name(name)
{
}

SourceCatalog::~SourceCatalog()
{
}

const Array<double, 2> SourceCatalog::getSourcePositions() const
{
  Array<double, 2> pos(m_sources.size(), 2);

  vector<Source>::const_iterator it;

  int i = 0;
  for (it = m_sources.begin(); it < m_sources.end(); it++, i++)
    {
      pos(i, 0) = (*it).getRA();
      pos(i, 1) = (*it).getDEC();
      i++;
    }

  return pos.copy();
}

const SourceCatalog* SourceCatalogLoader::loadFromFile(string filename)
{
  SourceCatalog* catalog = new SourceCatalog(filename);

  ifstream catfile(filename.c_str());

  if (!catfile)
    {
      LOG_FATAL_STR("Failed to open source catalog: " << filename);
      exit(EXIT_FAILURE);
    }

  string name, fluxstring;
  double ra, dec;
  Array<double, 2> flux;

  while (!catfile.eof() && !catfile.fail())
    {
      getline(catfile, name);
      if ("" == name) break;
      catfile >> ra;
      catfile >> dec;
      catfile.ignore(80,'\n');
      getline(catfile, fluxstring);

      istringstream fluxstream(fluxstring);
      fluxstream >> flux;

      catalog->m_sources.push_back(Source(name, ra, dec, flux));
    }

  catfile.close();

  return catalog;
}

