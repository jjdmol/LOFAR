// ParamHolder.h: Abstract base class to hold WorkHolder parameters
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
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_PARAMHOLDER_H
#define CEPFRAME_PARAMHOLDER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/BaseSim.h"

/**
   The ParamHolder class is the interface to a set of parameters
   defined in its sub-class DataType.
*/

class ParamHolder
{
public:
  /// Standard Parameter type class
  class DataType
    { public:
        int aValue;
    };

  ParamHolder();

  virtual ~ParamHolder();

  /** Show the contents of the object on cout.
      The default implementation outputs the string "ParamHolder".
  */
  virtual void dump() const;

  /** @name Comparison functions
      @memo Compare the serial number of this and other
      @doc Compare this and the other ParameterHolder using their
      serial number.
  */
  //@{
  bool operator== (const ParamHolder& other) const;
  bool operator!= (const ParamHolder& other) const;
  bool operator< (const ParamHolder& other) const;
  //@}

protected:
  ParamHolder& operator= (const ParamHolder& that);

private:
  /// Forbid copy constructor.
  ParamHolder (const ParamHolder&);


  DataType itsDataPacket;
  static int theirSerial;
  int itsSerial;
};


#endif 
