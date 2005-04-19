//# Selector.h:
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

#ifndef TINYCEP_SELECTOR_H
#define TINYCEP_SELECTOR_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

namespace LOFAR
{

/**
   This class defines the base class for selection mechanisms.
*/

class Selector
{
public:
  Selector(unsigned int noOptions);

  virtual ~Selector();

  virtual unsigned int getNext() = 0;
  
  virtual Selector* clone() const = 0;

  unsigned int getNumberOfOptions();

  int getCurrentSelection();

protected:
  Selector(const Selector&);

  unsigned int itsNOptions;      // Number of options from which to select one
  int          itsCurrentSelection;  // The current selection
};


inline unsigned int Selector::getNumberOfOptions()
{ return itsNOptions; }

inline int Selector::getCurrentSelection()
{ return itsCurrentSelection; }

}

#endif
