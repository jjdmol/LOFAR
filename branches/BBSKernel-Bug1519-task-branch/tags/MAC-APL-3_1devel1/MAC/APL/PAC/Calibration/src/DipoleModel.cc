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
//#  $Id$


#include "DipoleModel.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <fstream>
#include <blitz/array.h>

using namespace CAL;
using namespace blitz;
using namespace std;

DipoleModel::DipoleModel(string name) : m_name(name)
{
}

DipoleModel::~DipoleModel()
{
}

/*const*/ DipoleModel* DipoleModelLoader::loadFromFile(string filename)
{
  DipoleModel* model = new DipoleModel(filename);

  if (!model) return 0;

  ifstream modelfile(filename.c_str());

  if (!modelfile)
    {
      LOG_FATAL_STR("Failed to open dipole model: " << filename);
      exit(EXIT_FAILURE);
    }

  modelfile >> model->m_sens;

  modelfile.close();

  return model;
}

