//# meqDFT_GVD.cc: 
//# MeqExpr node class, automatically generated from Glish
//#
//# Copyright  (C)  2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2,  7990 AA Dwingeloo, The Netherlands,  seg@astron.nl
//# 
//# $Id$: 

#include "meqDFT_GVD.h"
#include <MEQ/Vells.h>
#include <Common/Debug.h>

#define   u                 *(values[0])
#define   DEC               *(values[1])
#define   RA                *(values[2])
#define   RA0               *(values[3])
#define   v                 *(values[4])
#define   DEC0              *(values[5])
#define   w                 *(values[6])



using namespace Meq::VellsMath;
namespace meq {

  DFT_GVD::DFT_GVD()
  {}

  DFT_GVD::~DFT_GVD()
  {}

  void DFT_GVD::evaluateVells (Vells& result, const Request&, const vector<Vells*>& values) 
  { 
    
  result = ( exp(6.283185307 * tocomplex(0.0,u*(cos(DEC) *
  sin(RA-RA0))+v*(sin(DEC) * cos(DEC0) - sin(DEC0) * cos(DEC) *
  cos(RA-RA0))+w*(sqrt(1.0-sqr((cos(DEC) * sin(RA-RA0)))-sqr((sin(DEC) *
  cos(DEC0) - sin(DEC0) * cos(DEC) * cos(RA-RA0))))))) / sqrt(sqrt(
  (sqrt(1.0-sqr((cos(DEC) * sin(RA-RA0)))-sqr((sin(DEC) * cos(DEC0) - sin(DEC0) *
  cos(DEC) * cos(RA-RA0))))) )) ); 

  }


} // namespace meq

