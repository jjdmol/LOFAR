//#  DH_FixedSize.h: DataHolder with one dimensional byte array of fixed size
//#
//#  Copyright (C) 2000, 2001
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

#ifndef LOFAR_3BLOCKPERF_DH_FIXEDSIZE_H
#define LOFAR_3BLOCKPERF_DH_FIXEDSIZE_H

// \file
// DataHolder with one dimensional byte array of fixed size

#include <string>
#include <Transport/DataHolder.h>

namespace LOFAR {

using std::string;

class DH_FixedSize: public DataHolder
{
public:
  typedef float valueType;

  DH_FixedSize (const string& name, 
		unsigned int size); // size is in bytes

  virtual ~DH_FixedSize();

  DataHolder* clone() const;

  virtual void init();

  void fillDataPointers();

  unsigned int getNoValues();
  valueType* getPtr2Data();
  
protected:
  DH_FixedSize (const DH_FixedSize&);

private:

  /// Forbid assignment.
  DH_FixedSize& operator= (const DH_FixedSize&);
  unsigned int itsNoValues;
  valueType* itsDataPtr;
};

inline unsigned int DH_FixedSize::getNoValues() {
  return itsNoValues; };
inline DH_FixedSize::valueType* DH_FixedSize::getPtr2Data() {
  return itsDataPtr; };
}
#endif 


