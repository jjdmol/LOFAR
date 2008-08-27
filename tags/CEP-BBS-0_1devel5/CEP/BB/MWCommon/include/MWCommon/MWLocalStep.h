/// @file
/// @brief Base classes for local MW commands
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCOMMON_MWLOCALSTEP_H
#define LOFAR_MWCOMMON_MWLOCALSTEP_H

#include <MWCommon/MWStep.h>

namespace LOFAR { namespace CEP {

  /// @ingroup mwcommon
  /// @brief Base class for a step to process a local MW command.

  /// This class defines a class that serves as the base class for a
  /// local MW step. A local MW step is a step that can be executed
  /// directly by a worker without the need of interaction between workers.
  /// An example is a subtract or correct. A solve is not a local step,
  /// because it requires interaction between workers.

  class MWLocalStep: public MWStep
  {
  public:
    MWLocalStep()
    {}

    virtual ~MWLocalStep();

    /// Visit the object, so the visitor can process it.
    /// The default implementation uses the MWStepVisitor::visitlocal
    /// function.
    virtual void visit (MWStepVisitor&) const;
  };


}} /// end namespaces

#endif
