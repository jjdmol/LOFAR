//  Sel_Linked.h:
//
//  Copyright (C) 2000, 2001
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
// Sel_Linked.h: interface for the Sel_Linked class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_SEL_LINKED_H
#define CEPFRAME_SEL_LINKED_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Common/Debug.h"
#include "CEPFrame/Selector.h"

/**
   This class implements a selection mechanism which follows the selection
   of another selector. Useful when an output must follow an input. Both
   selectors must have the same number of options.
*/

class Sel_Linked : public Selector
{
public:
  Sel_Linked(Selector* selector, unsigned int itsNoptions);

  virtual ~Sel_Linked();

  virtual unsigned int getNext();

private:
  Selector* itsLink;
  
};


#endif
