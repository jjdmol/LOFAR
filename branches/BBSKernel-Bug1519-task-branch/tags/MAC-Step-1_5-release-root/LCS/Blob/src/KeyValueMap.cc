//# KeyValueMap.cc: Class to hold a collection of parameter name/value pairs.
//#
//#  Copyright (C) 2001
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/KeyValueMap.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/lofar_iostream.h>

namespace LOFAR {

KeyValueMap::KeyValueMap()
{}

KeyValueMap::KeyValueMap (const KeyValueMap& that)
: map<string, KeyValue> (that)
{}

KeyValueMap::~KeyValueMap()
{}

KeyValueMap& KeyValueMap::operator= (const KeyValueMap& that)
{
  if (this != &that) {
    map<string, KeyValue>::operator= (that);
  }
  return *this;
}

bool KeyValueMap::getBool (const string& name, bool defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getBool();
}
int KeyValueMap::getInt (const string& name, int defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getInt();
}
float KeyValueMap::getFloat (const string& name, float defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getFloat();
}
double KeyValueMap::getDouble (const string& name, double defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getDouble();
}
fcomplex KeyValueMap::getFComplex (const string& name,
				   const fcomplex& defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getFComplex();
}
dcomplex KeyValueMap::getDComplex (const string& name,
				   const dcomplex& defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getDComplex();
}
const string& KeyValueMap::getString (const string& name,
				      const string& defVal) const
{
  const_iterator value = find(name);
  if (value == end()) {
    return defVal;
  }
  return value->second.getString();
}

void KeyValueMap::show (ostream& os) const
{
  for (const_iterator iter=begin(); iter!=end(); ++iter) {
    os << iter->first << "\t= " << iter->second << endl;
  }
}

ostream& operator<< (ostream& os, const KeyValueMap& param)
{
  for (KeyValueMap::const_iterator iter=param.begin();
       iter!=param.end();
       ++iter) {
    if (iter != param.begin()) {
      os << ", ";
    }
    os << iter->first << '=' << iter->second;
  }
  return os;
}

BlobOStream& operator<< (BlobOStream& bs, const KeyValueMap& param)
{
  bs.putStart ("KeyValueMap", 1);
  bs << uint32(param.size());
  for (KeyValueMap::const_iterator iter=param.begin();
       iter!=param.end();
       ++iter) {
    bs << iter->first << iter->second;
  }
  bs.putEnd();
  return bs;
}

BlobIStream& operator>> (BlobIStream& bs, KeyValueMap& param)
{
  bs.getStart ("KeyValueMap");
  param.clear();
  uint32 nrp;
  bs >> nrp;
  KeyValue pv;
  string pn;
  for (uint32 i=0; i<nrp; i++) {
    bs >> pn >> pv;
    param[pn] = pv;
  }
  bs.getEnd();
  return bs;
}

} // end namespace
