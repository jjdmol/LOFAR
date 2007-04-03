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

namespace LOFAR
{
namespace BBS
{

class KernelCommandControl: public CommandHandler
{
public:
    virtual ~KernelCommandControl()
    {
    }

    virtual void handle(BBSStrategy &command);
    virtual void handle(BBSStep &command);
    virtual void handle(BBSMultiStep &command);
    virtual void handle(BBSSingleStep &command);
    virtual void handle(BBSPredictStep &command);
    virtual void handle(BBSSubtractStep &command);
    virtual void handle(BBSCorrectStep &command);
    virtual void handle(BBSSolveStep &command);
    virtual void handle(BBSShiftStep &command);
    virtual void handle(BBSRefitStep &command);
};

} //# namespace BBS
} //# namespace LOFAR

#endif
