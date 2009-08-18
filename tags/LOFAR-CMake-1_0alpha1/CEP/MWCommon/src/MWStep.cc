//# MWStep.cc: Step to process the MW commands
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/MWStep.h>

namespace LOFAR { namespace CEP {

  MWStep::~MWStep()
  {}

  ParameterSet MWStep::getParms() const
  {
    return ParameterSet();
  }

  void MWStep::visit (MWStepVisitor& visitor) const
  {
    visitor.visit (*this);
  }

  void MWStep::print (std::ostream&, const std::string&) const
  {}

}} // end namespaces
