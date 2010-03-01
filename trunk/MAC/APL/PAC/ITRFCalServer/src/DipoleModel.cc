//#  -*- mode: c++ -*-
//#  DipoleModel.cc: implementation of the DipoleModel class
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
//#  $Id: DipoleModel.cc 6818 2005-10-20 09:31:47Z cvs $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include "DipoleModel.h"
#include "DipoleModelData.h"

#include <fstream>
#include <blitz/array.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;

DipoleModel::DipoleModel(string name, const Array<complex<double>, 4>& sens) :
  m_name(name), m_sens(sens)
{
}

DipoleModel::~DipoleModel()
{
}

DipoleModels::DipoleModels()
{
}

DipoleModels::~DipoleModels()
{
  for (map<string, const DipoleModel*>::const_iterator it = m_models.begin();
       it != m_models.end(); ++it)
  {
    if ((*it).second) delete (*it).second;
  }
}

void DipoleModels::getAll(string url)
{
  DipoleModelData data;

  while (data.getNextFromFile(url)) {
    DipoleModel* model = new DipoleModel(data.getName(), data.getSens());
    m_models[data.getName()] = model;
  }
}

