//# meqDFT_GVD.h: 
//# MeqExpr node class, automatically generated from Glish
//#
//# Copyright  (C)  2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2,  7990 AA Dwingeloo, The Netherlands,  seg@astron.nl
//# 
//# $Id$: 

#ifndef MEQ_meqDFT_GVD_H
#define MEQ_meqDFT_GVD_H
#include <MEQ/Function.h>
#pragma aidgroup MeqGen
#pragma types #meq::DFT_GVD


namespace meq {

  using namespace Meq;

  class DFT_GVD : public Function
  {

    public:

      DFT_GVD();

      virtual ~DFT_GVD();

      virtual void evaluateVells (Vells& result, const Request&, const vector<Vells*>& values);


    private:

  };

} // namespace meq

#endif
