// ParamBlock.h: Class to hold a collection of parameter name/value pairs.
//
//  Copyright (C) 2001
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

#ifndef CEPFRAME_PARAMBLOCK_H
#define CEPFRAME_PARAMBLOCK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/ParamValue.h"
#include <Common/lofar_map.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{

/**
   The ParamBlock class holds a collection of parameter name/value pairs.
   It uses the STL map class to hold the parameters.
*/

class ParamBlock: public map<string, ParamValue>
{
public:
  typedef map<string,ParamValue>::const_iterator const_iterator;
  typedef map<string,ParamValue>::iterator iterator;

  ParamBlock();

  /// Copy constructor (copy semantics)
  ParamBlock (const ParamBlock& that);

  ~ParamBlock();

  /// Assignment (copy semantics)
  ParamBlock& operator= (const ParamBlock& that);

  /// Is a parameter defined?
  bool isDefined (const string& name) const
    { return find(name) != end(); }

  /// Get the value of a parameter. Use the default if not existing.
  bool getBool (const string& name, bool defVal) const;
  int getInt (const string& name, int defVal) const;
  float getFloat (const string& name, float defVal) const;
  double getDouble (const string& name, double defVal) const;
  complex<float> getComplex (const string& name,
			     const complex<float>& defVal) const;
  complex<double> getDComplex (const string& name,
			       const complex<double>& defVal) const;
  const string& getString (const string& name,
			   const string& defVal) const;

  /// Show the contents of the object.
  void show (ostream&) const;

  friend ostream& operator<< (ostream&, const ParamBlock&);
};

}

#endif 
