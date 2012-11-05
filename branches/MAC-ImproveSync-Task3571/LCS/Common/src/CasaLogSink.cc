//# CasaLogSink.cc: LogSink to convert casacore messages to LOFAR 
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

// @author Ger van Diepen (gvd AT astron DOT nl)

#include <lofar_config.h>
#include <Common/CasaLogSink.h>
#include <Common/LofarLogger.h>


namespace LOFAR {

#ifdef HAVE_AIPSPP

  CasaLogSink::CasaLogSink()
    : casa::LogSinkInterface (casa::LogFilter())
  {}

  CasaLogSink::CasaLogSink (casa::LogMessage::Priority filter)
    : casa::LogSinkInterface (casa::LogFilter(filter))
  {}

  CasaLogSink::CasaLogSink (const casa::LogFilterInterface& filter)
    : casa::LogSinkInterface (filter)
  {}

  CasaLogSink::~CasaLogSink()
  {}

  void CasaLogSink::attach()
  {
    casa::LogSinkInterface* globalSink = new LOFAR::CasaLogSink;
    // Note that the pointer is taken over by LogSink.
    casa::LogSink::globalSink (globalSink);
  }

  casa::Bool CasaLogSink::postLocally (const casa::LogMessage& message)
  {
    casa::Bool posted = casa::False;
    if (filter().pass(message)) {
      std::string msg (message.origin().location() + ": " + message.message());
      posted = casa::True;
      switch (message.priority()) {
      case casa::LogMessage::DEBUGGING:
      case casa::LogMessage::DEBUG2:
      case casa::LogMessage::DEBUG1:
	{
	  LOG_DEBUG (msg);
	  break;
	}
      case casa::LogMessage::NORMAL5:
      case casa::LogMessage::NORMAL4:
      case casa::LogMessage::NORMAL3:
      case casa::LogMessage::NORMAL2:
      case casa::LogMessage::NORMAL1:
      case casa::LogMessage::NORMAL:
	{
	  LOG_INFO (msg);
	  break;
	}
      case casa::LogMessage::WARN:
	{
	  LOG_WARN (msg);
	  break;
	}
      case casa::LogMessage::SEVERE:
	{
	  LOG_ERROR (msg);
	  break;
	}
      }
    }
    return posted;
  }

  void CasaLogSink::clearLocally()
  {}

  casa::String CasaLogSink::localId()
  {
    return casa::String("CasaLogSink");
  }

  casa::String CasaLogSink::id() const
  {
    return casa::String("CasaLogSink");
  }

#else
  void CasaLogSink::attach()
  {}
#endif

} // end namespaces
