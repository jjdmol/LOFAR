//# MySocketExample.cc: test program for socket transfer using tinyCEP
//#
//# Copyright (C) 2004
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

#ifndef TINYCEP_SOCKETEXAMPLE_H
#define TINYCEP_SOCKETEXAMPLE_H

#include <lofar_config.h>

// CEP includes
#include <Transport/DataHolder.h>
#include <Transport/TH_Socket.h>
#include <tinyCEP/TinyApplicationHolder.h>
#include <tinyCEP/TinyDataManager.h>

// Application includes
#include <WH_Example.h>
#include <DH_Example.h>

namespace LOFAR
{

  class MySocketExample: public TinyApplicationHolder
  {
  public:
    MySocketExample(int ninput, int noutput, bool sender);
    ~MySocketExample();

    void define(const KeyValueMap& map);
    void init();
    void run (int nsteps);
    void run_once();
    void quit();
    void dump() const;

    WorkHolder* itsWHs[5];

    private :
      int  itsNinput;
      int  itsNoutput;
      bool itsIsSender;
      
      
  };

} // namespace LOFAR

#endif
