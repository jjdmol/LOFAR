//  DH_Empty.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////


#include <DH_Empty.h>

namespace LOFAR
{

DH_Empty::DH_Empty (const string& name)
: DataHolder (name, "DH_Empty")
{
}

DH_Empty::DH_Empty(const DH_Empty& that)
  : DataHolder(that)
{
}

DH_Empty::~DH_Empty()
{
}

DataHolder* DH_Empty::clone() const
{
  return new DH_Empty(*this);
}


}
