//# Function1.h: Abstract class for a function with 1 child
//#
//# Copyright (C) 2003
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

#ifndef MEQ_FUNCTION1_H
#define MEQ_FUNCTION1_H
    
#include <MEQ/Function.h>


namespace Meq {    


//##ModelId=400E530302D5
class Function1 : public Function
{
public:
    //##ModelId=400E530902E8
  Function1();

    //##ModelId=400E53090359
  virtual ~Function1();

  // Check if 1 child has been given.
    //##ModelId=400E530903C7
  virtual void checkChildren();
};


} // namespace Meq

#endif
