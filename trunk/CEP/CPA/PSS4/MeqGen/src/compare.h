//# compare.h: 
//# MeqExpr node class, automatically generated from Glish
//#
//# Copyright  (C)  2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2,  7990 AA Dwingeloo, The Netherlands,  seg@astron.nl
//# 
//# $Id$: 

#ifndef _MeqGen_compare_H
#define _MeqGen_compare_H

#include <MEQ/Function.h>

#pragma aidgroup MeqGen
#pragma types #Meq::compare

namespace Meq 
{

  class compare : public Function
  {

    public:

      compare();

      virtual ~compare();

      virtual void evaluateVells (Vells& result, const Request&, const vector<Vells*>& values);


    private:

  };

} // namespace MEQ 

#endif
