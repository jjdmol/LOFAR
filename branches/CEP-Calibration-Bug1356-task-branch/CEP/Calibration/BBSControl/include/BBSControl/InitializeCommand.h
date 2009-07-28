//# InitializeCommand.h: Concrete "initialize" command
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

#ifndef LOFAR_BBSCONTROL_INITIALIZECOMMAND_H
#define LOFAR_BBSCONTROL_INITIALIZECOMMAND_H

// \file
// Concrete "initialize" command

#include <BBSControl/Command.h>
#include <BBSControl/Types.h>
#include <Common/LofarTypes.h>
#include <Common/ParameterSet.h>

namespace LOFAR
{
  class ParameterSet;

  namespace BBS
  {
    class Strategy;
    
    // \ingroup BBSControl
    // @{

    // Concrete \c initialize command. This command is sent by the global
    // controller to indicate that the required information for the local
    // controllers is available on the blackboard. This command is sent when a
    // new run is started, immediately after the strategy has been posted to
    // the blackboard.
    class InitializeCommand : public Command
    {
    public:
      // Default constructor.
      InitializeCommand();

      // Create an InitializeCommand from the given Strategy.
      InitializeCommand(const Strategy& strategy);

      // Destructor.
      virtual ~InitializeCommand() {}

      // Return the command type of \c *this as a string.
      virtual const string& type() const;

      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this.
      virtual void read(const ParameterSet& ps);

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // Accept a CommandVisitor that wants to process \c *this.
      virtual CommandResult accept(CommandVisitor &visitor) const;

      string getInputColumn() const
      { return itsInputColumn; }

      vector<string> getStations() const
      { return itsStations; }
      
      Correlation getCorrelation() const
      { return itsCorrelation; }
      
      bool useSolver() const
      { return itsUseSolver; }

    private:
      // Name of the input column.
      string                 itsInputColumn;

      // Names of the stations to use. Names may contains wildcards, like \c *
      // and \c ?.
      vector<string>         itsStations;

      // Correlation product selection.
      Correlation            itsCorrelation;

      // Connect to the global solver?
      bool                   itsUseSolver;
    };

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
