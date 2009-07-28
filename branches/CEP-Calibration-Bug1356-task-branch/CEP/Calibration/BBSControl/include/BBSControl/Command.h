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

// \file

#include <Common/ObjectFactory.h>
#include <Common/Singleton.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <BBSControl/CommandResult.h>

namespace LOFAR
{
  //# Forward declarations
  class ParameterSet;

  namespace BBS
  {
    class CommandVisitor;

    // \ingroup BBSControl
    // @{

    // Base class for commands that can be sent to or retrieved from the
    // CommandQueue. This class works in close cooperation with the
    // CommandVisitor class, which implements a double-dispatch mechanism for
    // the Command class, using the Visitor pattern (Gamma, 1995).
    class Command
    {
    public:
      // Destructor.
      virtual ~Command() {}

      // Return the command type of \c *this as a string.
      virtual const string& type() const = 0;

      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ParameterSet& ps) const = 0;

      // Read the contents from the ParameterSet \a ps into \c *this.
      virtual void read(const ParameterSet& ps) = 0;

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // Accept the "visiting" CommandVisitor. Derived classes must implement
      // this method such that it will make a callback to visitor.visit()
      // passing themselves as argument.
      // \code
      // visitor.visit(*this);
      // \endcode
      virtual CommandResult accept(CommandVisitor &visitor) const = 0;

      // Write the contents of a Command to an output stream.
      friend ostream& operator<<(ostream&, const Command&);

      // Write the contents of a Command to a ParameterSet.
      friend ParameterSet& 
      operator<<(ParameterSet&, const Command&);

      // Read the contents of a ParameterSet into a Command.
      friend ParameterSet& 
      operator>>(ParameterSet&, Command&);
    };

    // Factory that can be used to generate new Command objects.
    // The factory is defined as a singleton.
    typedef Singleton< ObjectFactory< Command(), string > > CommandFactory;

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
