//# compare.cc: 
//# MeqExpr node class, automatically generated from Glish
//#
//# Copyright  (C)  2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2,  7990 AA Dwingeloo, The Netherlands,  seg@astron.nl
//# 
//# $Id$: 

#include "compare.h"
#include <MEQ/Vells.h>
#include <Common/Debug.h>

#define   copy              *(values[0])
#define   MeqSpigot         *(values[1])

namespace Meq {

  compare::compare()
  {}

  compare::~compare()
  {}

  void compare::evaluateVells (Vells& result, const Request&, const vector<Vells*>& values)
  {
    result = ( copy - MeqSpigot );
  }


} // namespace MEQ


