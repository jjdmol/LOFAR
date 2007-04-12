//# Command.h: Base class for commands
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_COMMAND_H
#define LOFAR_BBSCONTROL_COMMAND_H

#include <Common/ObjectFactory.h>
#include <Common/Singleton.h>
#include <Common/lofar_string.h>

#include <Blob/BlobStreamable.h>

namespace LOFAR
{
  //# Forward declarations
  namespace ACC { namespace APS { class ParameterSet; } }

  namespace BBS
  {
    class CommandHandler;

    // Base class for commands that can be sent to or retrieved from the
    // CommandQueue. This class implements a double-dispatch mechanism using
    // the Visitor pattern (Gamma, 1995). 
    class Command
    {
    public:
      // Create a new Command object. The key \c Command.type key in \a ps
      // must contain the type of Command to be constructed.
      static Command* create(const ACC::APS::ParameterSet& ps);

      // Destructor.
      virtual ~Command() {}

      virtual void accept(CommandHandler &handler) const = 0;

      // Write the contents of a Command to a ParameterSet.
      friend ACC::APS::ParameterSet& 
      operator<<(ACC::APS::ParameterSet&, const Command&);

      // Read the contents of a ParameterSet into a Command.
      friend ACC::APS::ParameterSet& 
      operator>>(ACC::APS::ParameterSet&, Command&);

    protected:
      // Return the command type of \c *this as a string.
      virtual const string& type() const = 0;

    private:
      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ACC::APS::ParameterSet& ps) const = 0;

      // Read the contents from the ParameterSet \a ps into \c *this.
      virtual void read(const ACC::APS::ParameterSet& ps) = 0;

    };

    // Factory that can be used to generate new Command objects.
    // The factory is defined as a singleton.
    typedef Singleton< ObjectFactory< Command(), string > > CommandFactory;

  } //# namespace BBS

} //# namespace LOFAR

#endif
