//#  ConverterCommand.h: class describing commands to be sent to converter.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_AMCBASE_AMCCLIENT_CONVERTERCOMMAND_H
#define LOFAR_AMCBASE_AMCCLIENT_CONVERTERCOMMAND_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_set.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace AMC
  {

    // Class representing the commands that can be sent to the converter. The
    // actual commands are defined in the enumerate type Cmd. This \c enum was
    // defined within a class for a number of reasons:
    // - to be able to define an operator<<(BlobOStream&) and
    //   operator>>(BlobIStream&); this is especially important when we need 
    //   to ensure that the \c enum is streamed with a fixed size
    // - to be able to check whether the \c enum being read using
    //   operator>>(BlobIStream&) represents a valid enumeration value.
    // - to avoid namespace pollution
    class ConverterCommand
    {
    public:
      // All possible commands.
      //
      //# ATTENTION: Make sure that you update the implementation of
      //#            Init::Init() when you add or remove values here!
      //# WARNING:   Do NOT add INVALID to theirCmdSet. It is used to catch
      //#            uninitialized enums.
      enum Cmd {
        INVALID = -1,  ///< Invalid. Used in default constructor.
        J2000toAZEL,   ///< From J2000 to AZEL
        AZELtoJ2000    ///< From AZEL to J2000
      };

      // Default constructor. Initialize itsCmd to \c INVALID.
      ConverterCommand() : itsCmd(INVALID) {}

      // If \a iCmd matches with one of the enumeration values in theirCmdSet,
      // then itsCmd is set to \a iCmd, else itsCmd is set to \c INVALID.
      ConverterCommand(int32 iCmd);

      // Get the current converter command.
      Cmd get() const { return itsCmd; }

      // Check if the current converter command is valid.
      bool isValid() const { return itsCmd != INVALID; }

    private:
      // The type of set containing all valid commands.
      typedef set<Cmd> cmdset_t;

      // The actual set containing all valid commands.
      static cmdset_t theirCmdSet;
      
      // This struct is used to initialize the static data member theirCmdSet, 
      // before it is being used. 
      struct Init
      {
        // Initialize the static data member theirCmdSet. 
        Init();
        // Clear the set static data member theirCmdSet.
        ~Init();
      };

      // We need one, and only one, instance of the initializer in order to
      // trigger the initialization of the static data member theirCmdSet. It
      // will be defined in the <tt>.cc</tt> file.
      static Init theirInit;

      // The current convertor command.
      Cmd itsCmd;

    };

//     // Compare \a lhs and \a rhs for equality.
//     bool operator==(const ConverterCommand& lhs, const ConverterCommand& rhs);

    // Output in ASCII format.
    ostream& operator<<(ostream& os, const ConverterCommand& cc);


    //# We implement the constructor and destructor here, because they are
    //# tightly coupled with the definition of the \c enum Enum::Cmd. Hence,
    //# this improves maintainability.
    inline ConverterCommand::Init::Init()
    {
      ConverterCommand::theirCmdSet.insert(J2000toAZEL);
      ConverterCommand::theirCmdSet.insert(AZELtoJ2000);
    }

    inline ConverterCommand::Init::~Init()
    {
      ConverterCommand::theirCmdSet.clear();
    }

    
  } // namespace AMC

} // namespace LOFAR

#endif
