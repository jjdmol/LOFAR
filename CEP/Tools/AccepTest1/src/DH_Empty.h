//# DH_Empty.h: Dummy DataHolder (doing nothing)
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef CEPFRAME_DH_EMPTY_H
#define CEPFRAME_DH_EMPTY_H

#include <lofar_config.h>

#include <Transport/DataHolder.h>

namespace LOFAR
{

/**
   This class represents an empty DataHolder.
   This is a DataHolder that does not do anything.
   It does not generate output nor does it read input.
*/

class DH_Empty: public DataHolder
{
public:
  explicit DH_Empty (const string& name = "");
  DH_Empty(const DH_Empty&);
  virtual ~DH_Empty();
  virtual DataHolder* clone() const;
};

}

#endif 
