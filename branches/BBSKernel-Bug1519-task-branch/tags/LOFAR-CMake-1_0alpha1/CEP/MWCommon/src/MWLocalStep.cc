//# MWLocalStep.cc: Base classes for local MW commands
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/MWLocalStep.h>

namespace LOFAR { namespace CEP {

  MWLocalStep::~MWLocalStep()
  {}

  void MWLocalStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visitLocal (*this);
  }

}} // end namespaces
