//# meqFlagger.cc: 
//# MeqExpr node class, automatically generated from Glish
//#
//# Copyright  (C)  2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2,  7990 AA Dwingeloo, The Netherlands,  seg@astron.nl
//# 
//# $Id$: 

#include "meqFlagger.h"
#include <MEQ/Vells.h>
#include <Common/Debug.h>

#define   MeqSpigot         *(values[0])

using namespace Meq::VellsMath;

namespace meq {

  Flagger::Flagger()
  {}

  Flagger::~Flagger()
  {}

  void Flagger::evaluateVells (Vells& result, const Request&, const vector<Vells*>& values)
  {
    result = ( MeqSpigot );
  }


} // namespace meq

