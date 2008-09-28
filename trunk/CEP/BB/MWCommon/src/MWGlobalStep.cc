//# MWGlobalStep.cc: Base classes for global MW commands
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/MWGlobalStep.h>

namespace LOFAR { namespace CEP {

  MWGlobalStep::~MWGlobalStep()
  {}

  void MWGlobalStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitGlobal (*this);
  }

}} // end namespaces
