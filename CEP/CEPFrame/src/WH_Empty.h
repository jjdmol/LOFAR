//# WH_Empty.h: An empty WorkHolder (doing nothing)
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

#ifndef CEPFRAME_WH_EMPTY_H
#define CEPFRAME_WH_EMPTY_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <tinyCEP/WorkHolder.h>
#include <CEPFrame/DH_Empty.h>

namespace LOFAR
{

/**
   This is an empty WorkHolder class.
   It contains a DH_Empty object for input and output.
*/

class WH_Empty: public WorkHolder
{
 public:
  /// Construct the work holder with its data holders.
  explicit WH_Empty (const string& name = "WH_Empty");

  virtual ~WH_Empty();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput,
				const KeyValueMap&);

  /// Make a fresh copy of the WH object.
  virtual WH_Empty* make (const string& name);

  /// Do a process step.
  void process();

  /// Show the work holder on stdout.
  void dump();

  /// Example Monitoring output
  virtual int getMonitorValue(const char* name);

private:
  /// Forbid copy constructor.
  WH_Empty (const WH_Empty&);

  /// Forbid assignment.
  WH_Empty& operator= (const WH_Empty&);

};

}

#endif
