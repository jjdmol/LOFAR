// ParamBlock.cc: Class to hold a collection of parameter name/value pairs.
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
//  Revision 1.5  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.4  2002/03/15 13:28:08  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.3  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.2  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.1  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//
//////////////////////////////////////////////////////////////////////


#include "BaseSim/ParamBlock.h"
#include <Common/lofar_iostream.h>

ParamBlock::ParamBlock()
{}

ParamBlock::ParamBlock (const ParamBlock& that)
: map<string, ParamValue> (that)
{}

ParamBlock::~ParamBlock()
{}

ParamBlock& ParamBlock::operator= (const ParamBlock& that)
{
  if (this != &that) {
    map<string, ParamValue>::operator= (that);
  }
  return *this;
}

bool ParamBlock::getBool (const string& name, bool defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getBool();
}
int ParamBlock::getInt (const string& name, int defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getInt();
}
float ParamBlock::getFloat (const string& name, float defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getFloat();
}
double ParamBlock::getDouble (const string& name, double defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getDouble();
}
complex<float> ParamBlock::getComplex (const string& name,
				       const complex<float>& defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getComplex();
}
complex<double> ParamBlock::getDComplex (const string& name,
					 const complex<double>& defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getDComplex();
}
const string& ParamBlock::getString (const string& name,
				     const string& defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getString();
}

void ParamBlock::show (ostream& os) const
{
  for (const_iterator iter=begin(); iter!=end(); ++iter) {
    os << iter->first << "\t= " << iter->second << endl;
  }
}

ostream& operator<< (ostream& os, const ParamBlock& param)
{
  for (ParamBlock::const_iterator iter=param.begin();
       iter!=param.end();
       ++iter) {
    if (iter != param.begin()) {
      os << ", ";
    }
    os << iter->first << '=' << iter->second;
  }
  return os;
}
