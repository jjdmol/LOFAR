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
//  $Log$
//  Revision 1.6  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.5  2002/03/15 13:28:08  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.4  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.3  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.2  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.1  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_PARAMBLOCK_H
#define BASESIM_PARAMBLOCK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/ParamValue.h"
#include <Common/lofar_map.h>
#include <Common/lofar_iosfwd.h>

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


#endif 
