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

using namespace CAL;
using namespace std;
using namespace blitz;

SourceCatalog::SourceCatalog(string name, int numsources) : m_name(name), m_sources(numsources)
{
}

SourceCatalog::~SourceCatalog()
{
}

const Array<double, 2>& SourceCatalog::getSourcePositions() const
{
  Array<double, 2> pos(m_sources.size(), 2);

  vector<Source>::iterator it(m_sources);

  for (int i = 0, vector<Source>::iterator source = it.begin(); source < it.end(); it++, i++)
    {
      pos(i, 0) = (*source).getRA();
      pos(i, 1) = (*source).getDEC();
      i++;
    }

  return pos;
}

static const SourceCatalog* SourceCatalogLoader::loadFromFile(string filename)
{
  SourceCatalog* catalog = new SourceCatalog();
}

