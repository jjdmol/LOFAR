//# NoiseStep.h: Options for noise operation.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSCONTROL_BBSNOISESTEP_H
#define LOFAR_BBSCONTROL_BBSNOISESTEP_H

// \file
// Options for noise operation.

#include <BBSControl/SingleStep.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSControl
// @{

class NoiseStep: public SingleStep
{
public:
    // Default constructor. Construct an empty NoiseStep object and make
    // it a child of the Step object \a parent.
    NoiseStep(const Step* parent = 0);

    // Construct a NoiseStep having the name \a name. Configuration
    // information for this step can be retrieved from the parameter set \a
    // parset, by searching for keys <tt>Step.\a name</tt>. \a parent
    // is a pointer to the Step object that is the parent of \c *this.
    NoiseStep(const string& name,
        const ParameterSet& parset,
        const Step* parent);

    virtual ~NoiseStep();

    // Accept a CommandVisitor that wants to process \c *this.
    virtual void accept(CommandVisitor &visitor) const;

    // Return the operation type of \c *this as a string.
    virtual const string& operation() const;

    // Print the contents of \c *this in human readable form into the output
    // stream \a os.
    virtual void print(ostream& os) const;

    // @name Accessor methods
    // @{
    double mean() const
    { return itsMean; }
    double sigma() const
    { return itsSigma; }
//    unsigned int seed() const
//    { return itsSeed; }
    // @}

    // Return the command type of \c *this as a string.
    virtual const string& type() const;
    
private:    
    // Write the contents of \c *this into the ParameterSet \a ps.
    virtual void write(ParameterSet& ps) const;

    // Read the contents from the ParameterSet \a ps into \c *this.
    virtual void read(const ParameterSet& ps);
    
    double          itsMean, itsSigma;
//    uint32          itsSeed;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
