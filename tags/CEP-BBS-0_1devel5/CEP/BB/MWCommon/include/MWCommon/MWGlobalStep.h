/// @file
/// @brief Base classes for global MW commands (like subtract)
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCOMMON_MWGLOBALSTEP_H
#define LOFAR_MWCOMMON_MWGLOBALSTEP_H

#include <MWCommon/MWStep.h>

namespace LOFAR { namespace CEP {

  /// @ingroup mwcommon
  /// @brief Base class for a step to process a global MW command.

  /// This class defines a class that serves as the base class for a
  /// global MW step. A global MW step is a step that cannot be executed
  /// directly by a worker without the need of interaction between workers.

  class MWGlobalStep: public MWStep
  {
  public:
    MWGlobalStep()
    {}

    virtual ~MWGlobalStep();

    /// Visit the object, so the visitor can process it.
    /// The default implementation uses the MWStepVisitor::visitGlobal
    /// function.
    virtual void visit (MWStepVisitor&) const;
  };


}} /// end namespaces

#endif
