//# KernelCommandControl.h: 
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

#ifndef LOFAR_BBSCONTROL_KERNELCOMMANDCONTROL_H
#define LOFAR_BBSCONTROL_KERNELCOMMANDCONTROL_H

#include <BBSControl/CommandHandler.h>
//#include <BBSControl/BlobStreamableConnection.h>
//#include <BBSControl/BBSStructs.h>

//#include <BBSKernel/Prediffer.h>

namespace LOFAR
{
//# Forward declations
class BlobStreamable;

namespace BBS
{

class KernelCommandControl: public CommandHandler
{
public:
    virtual ~KernelCommandControl()
    {
    }

    // @name Implementation of handle() for different commands.
    // @{
    virtual void handle(const BBSStrategy &command);
    virtual void handle(const BBSStep &command);
    virtual void handle(const BBSMultiStep &command);
    virtual void handle(const BBSSingleStep &command);
    virtual void handle(const BBSPredictStep &command);
    virtual void handle(const BBSSubtractStep &command);
    virtual void handle(const BBSCorrectStep &command);
    virtual void handle(const BBSSolveStep &command);
    virtual void handle(const BBSShiftStep &command);
    virtual void handle(const BBSRefitStep &command);
    // @}

/*
private:
    // Prediffer
    scoped_ptr<Prediffer> itsPrediffer;

    // Connections
    //scoped_ptr<BlobStreamableConnection> itsControllerConnection;
    scoped_ptr<BlobStreamableConnection> itsSolverConnection;

    // Region of interest
    vector<double> itsRegionOfInterest;

    // Work domain size
    DomainSize itsWorkDomainSize;
*/
};

} //# namespace BBS
} //# namespace LOFAR

#endif
