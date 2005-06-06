//  EmptyAppl.h: An empty ApplicationHolder  
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef LOFAR_EMPTYAPPL_EMPTYAPPL_H
#define LOFAR_EMPTYAPPL_EMPTYAPPL_H

// \file EmptyAppl.h
// Empty ApplicationHolder class

#include <CEPFrame/ApplicationHolder.h>
#include <Common/KeyValueMap.h>

/**
   This is an empty ApplicationHolder class   
*/

class EmptyAppl: public LOFAR::ApplicationHolder
{
public:
  EmptyAppl();
  virtual ~EmptyAppl();

  // overloaded methods from the ApplicationHolder base class
  virtual void define(const LOFAR::KeyValueMap& params = LOFAR::KeyValueMap());
  virtual void run(int);
  virtual void dump() const;
  virtual void quit();

  void undefine();

private:
     
};


#endif
