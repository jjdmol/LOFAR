//# meqCompare.h: 
//# MeqExpr node class, automatically generated from Glish
//#
//# Copyright  (C)  2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2,  7990 AA Dwingeloo, The Netherlands,  seg@astron.nl
//# 
//# $Id$: 

#ifndef MEQ_meqCompare_H
#define MEQ_meqCompare_H
#include <MEQ/Function.h>
#pragma aidgroup MeqGen
#pragma types #meq::Compare


namespace meq {

  using namespace Meq;

  class Compare : public Function
  {

    public:

      Compare();

      virtual ~Compare();

      virtual void evaluateVells (Vells& result, const Request&, const vector<Vells*>& values);


    private:

  };

} // namespace meq

#endif
