//#  -*- mode: c++ -*-
//#  Source.cc: definition of a Source class
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
//#  $Id: Source.cc 10637 2007-11-05 10:37:26Z overeem $

#include <lofar_config.h>
#include "Source.h"
#include "SourceData.h"

using namespace blitz;
using namespace LOFAR;
using namespace CAL;

Source::~Source()
{
}

void Source::getPos(double& ra, double& dec) const
{
  ra  = m_ra;
  dec = m_dec;
}

bool Source::getFlux(int n, double& freq, double& flux) const
{
  if (n < 0 || n >= (int)m_flux.extent(firstDim)) {

    freq = 0; 
    flux = 0;

    return false;
  }

  freq = m_flux(n, 0);
  flux = m_flux(n, 1);
  
  return true;
}

blitz::Array<double, 2> const & Source::getFluxes() const
{
  return m_flux;
}

Sources::Sources()
{
}

Sources::~Sources()
{
}

const Array<double, 2> Sources::getSourcePositions() const
{
  Array<double, 2> pos(m_sources.size(), 2);

  vector<Source>::const_iterator it;

  int i = 0;
  for (it = m_sources.begin(); it < m_sources.end(); ++it, i++)
    {
      pos(i, 0) = (*it).getRA();
      pos(i, 1) = (*it).getDEC();
      i++;
    }

  return pos.copy();
}

void Sources::getAll(std::string url)
{
  SourceData data;

  m_sources.clear(); // clear old sources
  while (data.getNextFromFile(url)) {
    m_sources.push_back(Source(data.getName(),
			       data.getRA(),
			       data.getDEC(),
			       data.getFlux()));
  }
}
