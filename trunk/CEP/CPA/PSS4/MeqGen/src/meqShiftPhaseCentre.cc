//# meqShiftPhaseCentre.cc: 
//# MeqExpr node class, automatically generated from Glish
//#
//# Copyright  (C)  2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2,  7990 AA Dwingeloo, The Netherlands,  seg@astron.nl
//# 
//# $Id$: 

#include "meqShiftPhaseCentre.h"
#include <MEQ/Vells.h>
#include <Common/Debug.h>

#define   meqIntoShiftPhaseCentre   *(values[0])
#define   meqDFT_GVD        *(values[1])
#define   meqDFT_GVD_3      *(values[2])



using namespace Meq::VellsMath;
namespace meq {

  ShiftPhaseCentre::ShiftPhaseCentre()
  {}

  ShiftPhaseCentre::~ShiftPhaseCentre()
  {}

  void ShiftPhaseCentre::evaluateVells (Vells& result, const Request&, const vector<Vells*>& values)
  {
    result = ( meqIntoShiftPhaseCentre * meqDFT_GVD * conj( meqDFT_GVD_3 ) );
  }


} // namespace meq

