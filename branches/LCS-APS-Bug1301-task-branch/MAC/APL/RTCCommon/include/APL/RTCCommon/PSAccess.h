//#  -*- mode: c++ -*-
//#
//#  PSAccess.h: shortcut macros for ParamterSet access
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

#ifndef PSACCESS_H_
#define PSACCESS_H_

#include <Common/ParameterSet.h>

#define __psaccess__convert_i getInt32
#define __psaccess__convert_f getFloat
#define __psaccess__convert_d getDouble
#define GET_CONFIG(var, type)  (globalParameterSet()->__psaccess__convert_##type(var))
#define GET_CONFIG_STRING(var) (globalParameterSet()->getString(var).c_str())
#define GET_CONFIG_PATH()      (globalParameterSet()->getSearchPath())

#endif /* PSACCESS_H_ */
