//# DH_Empty.h: Dummy DataHolder (doing nothing)
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef CEPFRAME_DH_EMPTY_H
#define CEPFRAME_DH_EMPTY_H

#include <lofar_config.h>

#include <Transport/DataHolder.h>
//#include "CEPFrame/BaseSim.h"

namespace LOFAR
{

/**
   This class represents an empty DataHolder.
   This is a DataHolder that does not do anything.
   It does not generate output nor does it read input.
*/

class DH_Empty: public DataHolder
{
public:
  explicit DH_Empty (const string& name = "");
  DH_Empty(const DH_Empty&);
  virtual ~DH_Empty();
  virtual DataHolder* clone() const;
};

}

#endif 
