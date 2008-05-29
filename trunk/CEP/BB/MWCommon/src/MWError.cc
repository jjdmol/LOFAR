//# MWError.cc: Basic exception for mwcontrol related errors
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <MWCommon/MWError.h>

namespace LOFAR { namespace CEP {

  MWError::MWError (const std::string& text, const std::string& file,
		    int line, const std::string& func)
    : Exception (text, file, line, func)
  {}

  MWError::~MWError() throw()
  {}

}} // end namespaces
