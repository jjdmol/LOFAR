//# AH_AccepTest.h: ApplicationHolder for acceptance test 2
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

#ifndef AH_ACCEPTEST_H
#define AH_ACCEPTEST_H

#include <lofar_config.h>

#include <tinyCEP/TinyApplicationHolder.h>

namespace LOFAR
{
  class AH_AccepTest: public TinyApplicationHolder {
  public:
    AH_AccepTest();
    ~AH_AccepTest();

  protected:
    void define(const KeyValueMap& kvm);
    void undefine();
    void init();
    void run(int nsteps);
    void quit();

    // Forbid copy constructor
    AH_AccepTest (const AH_AccepTest&);
    // Forbid assignment
    AH_AccepTest& operator= (const AH_AccepTest&);
  };
}

#endif
