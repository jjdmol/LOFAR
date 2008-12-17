//# NextChunkCommand.h: Concrete "next-chunk" command
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

#ifndef LOFAR_BBSCONTROL_NEXTCHUNKCOMMAND_H
#define LOFAR_BBSCONTROL_NEXTCHUNKCOMMAND_H

// \file 
// Concrete "next-chunk" command

#include <BBSControl/Command.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // Concrete \c next-chunk command. This command is sent by the global
    // controller when all steps in the strategy have been posted to the
    // database. It indicates to the local controller that it can proceed
    // processing with the "next chunk" of data.
    class NextChunkCommand : public Command
    {
    public:
      // Constructors.
      NextChunkCommand()
        :   itsFreqRange(0.0, 0.0),
            itsTimeRange(0.0, 0.0)
      {}
      
      NextChunkCommand(double freqStart, double freqEnd, double timeStart,
        double timeEnd)
        :   itsFreqRange(freqStart, freqEnd),
            itsTimeRange(timeStart, timeEnd)
      {}            
      
      NextChunkCommand(const pair<double, double> &freqRange,
        const pair<double, double> &timeRange)
        :   itsFreqRange(freqRange),
            itsTimeRange(timeRange)
      {}                    

      // Destructor.
      virtual ~NextChunkCommand() {}

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
      virtual void accept(CommandVisitor &visitor) const;
      
      pair<double, double> getFreqRange() const
      { return itsFreqRange; }

      pair<double, double> getTimeRange() const
      { return itsTimeRange; }

    private:
      pair<double, double>      itsFreqRange;
      pair<double, double>      itsTimeRange;      
    };

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
