/// @file
/// @brief Basic exception for master/worker related errors.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCOMMON_MWERROR_H
#define LOFAR_MWCOMMON_MWERROR_H

#include <Common/Exception.h>

namespace LOFAR { namespace CEP {

  /// @ingroup mwcommon
  /// @brief Basic exception for master/worker related errors.

  /// This class defines the basic MW exception.
  /// Only this basic exception is defined so far. In the future, some more 
  /// fine-grained exceptions might be derived from it.

  class MWError: public Exception
  {
  public:
    /// Create the exception object with the given message.
    explicit MWError (const std::string& text, const std::string& file="",
		      int line=0, const std::string& func="");

    virtual ~MWError() throw();
  };


}} /// end namespaces

#endif
