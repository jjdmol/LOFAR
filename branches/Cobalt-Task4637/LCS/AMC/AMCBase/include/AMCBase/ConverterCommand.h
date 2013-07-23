//# ConverterCommand.h: Commands to be sent to the AMC Converter server.
//#
//# Copyright (C) 2002-2004
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

#ifndef LOFAR_AMCBASE_CONVERTERCOMMAND_H
#define LOFAR_AMCBASE_CONVERTERCOMMAND_H

// \file
// Commands to be sent to the AMC Converter server

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_set.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace AMC
  {

    // \addtogroup AMCBase
    // @{

    // Class representing the commands that can be sent to the converter. The
    // actual commands are defined in the enumerate type Command. This \c enum
    // was defined within a class for a number of reasons:
    // - to be able to define an operator<<(BlobOStream&) and
    //   operator>>(BlobIStream&); this is especially important when we need 
    //   to ensure that the \c enum is streamed with a fixed size;
    // - to be able to check whether the \c enum being read using
    //   operator>>(BlobIStream&) represents a valid enumeration value;
    // - to avoid namespace pollution.
    class ConverterCommand
    {
    public:
      // All commands that can be sent to the converter.
      enum Commands {
        INVALID = -1,  ///< Used when specified value is out of range.
        J2000toAZEL,   ///< From J2000 to AZEL
        J2000toITRF,   ///< From J2000 to ITRF
        AZELtoJ2000,   ///< From AZEL to J2000
        ITRFtoJ2000,   ///< From ITRF to J2000
        //# Insert new types HERE !!
        N_Commands     ///< Number of converter commands.
      };

      // If \a cmd matches with one of the enumeration values in \c Commands,
      // then itsCommand is set to \a cmd, else itsCommand is set to \c
      // INVALID.
      ConverterCommand(Commands cmd = INVALID);

      // Get the current converter command.
      Commands get() const
      { return itsCommand; }

      // Return the current converter command as a string.
      const string& showCommand() const;

      // Check if the current converter command is valid.
      bool isValid() const
      { return itsCommand != INVALID; }

    private:
      // The current convertor command.
      Commands itsCommand;

    };

    // Comparison operator
    bool operator==(const ConverterCommand& lhs, const ConverterCommand& rhs);

    // Output in ASCII format.
    ostream& operator<<(ostream& os, const ConverterCommand& cc);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
