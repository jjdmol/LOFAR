//# meqCompare.cc: 
//# MeqExpr node class, automatically generated from Glish
//#
//# Copyright  (C)  2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2,  7990 AA Dwingeloo, The Netherlands,  seg@astron.nl
//# 
//# $Id$: 

#include "meqCompare.h"
#include <MEQ/Vells.h>
#include <Common/Debug.h>

#define   meqCopy           *(values[0])
#define   meqFlagger        *(values[1])



using namespace Meq::VellsMath;
namespace meq {

  Compare::Compare()
  {}

  Compare::~Compare()
  {}

  void Compare::evaluateVells (Vells& result, const Request&, const vector<Vells*>& values)
  {
    result = ( meqCopy - meqFlagger );
  }


} // namespace meq

