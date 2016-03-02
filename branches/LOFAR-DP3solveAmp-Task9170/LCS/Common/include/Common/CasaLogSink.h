//# CasaLogSink.h: LogSink to convert casacore messages to LOFAR 
//#
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_COMMON_CASALOGSINK_H
#define LOFAR_COMMON_CASALOGSINK_H

// \file LogSink to convert casacore messages to LOFAR 

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes

#ifdef HAVE_AIPSPP

//#Includes
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogFilter.h>

namespace LOFAR {

  // @brief log4cxx LogSink for Casa log messages.
  // @details
  // This class can be used to redirect the casacore log messages to
  // the logging system used by LOFAR.
  // It can be achieved by defining this sink as the global casa LogSink
  // as follows:
  // @code
  //   CasaLogSink::attach();
  // @endcode

  class CasaLogSink : public casa::LogSinkInterface
  {
  public:
    // By default no filtering is done.
    CasaLogSink();

    // Create the sink with the given filter (level).
    // @{
    explicit CasaLogSink (casa::LogMessage::Priority filter);
    explicit CasaLogSink (const casa::LogFilterInterface& filter);
    // @}

    ~CasaLogSink();

    // Make an object of this class the global casacore LogSink.
    static void attach();

    // If the message passes the filter, write it to the log4cxx sink.
    virtual casa::Bool postLocally (const casa::LogMessage& message);

    // Clear the local sink (i.e. remove all messages from it).
    virtual void clearLocally();

    // Returns the id for this class...
    static casa::String localId();
    // Returns the id of the LogSink in use...
    casa::String id() const;

  private:
    // Copying is forbidden.
    // @{
    CasaLogSink (const CasaLogSink& other);
    CasaLogSink& operator= (const CasaLogSink& other);
    // @}
  };

} // end namespace

#else
namespace LOFAR {
  class CasaLogSink {
  public:
    //# This won't do anything.
    static void attach();
  };
}
#endif

#endif
