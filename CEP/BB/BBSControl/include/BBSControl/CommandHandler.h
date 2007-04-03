//# CommandHandler.h: 
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

#ifndef LOFAR_BBSCONTROL_COMMANDHANDLER_H
#define LOFAR_BBSCONTROL_COMMANDHANDLER_H

namespace LOFAR
{
namespace BBS
{
class BBSStrategy;
class BBSStep;
class BBSMultiStep;
class BBSSingleStep;
class BBSPredictStep;
class BBSSubtractStep;
class BBSCorrectStep;
class BBSSolveStep;
class BBSShiftStep;
class BBSRefitStep;

class CommandHandler
{
public:
    virtual ~CommandHandler() = 0;

    virtual void handle(BBSStrategy &command) = 0;
    virtual void handle(BBSStep &command) = 0;
    virtual void handle(BBSMultiStep &command) = 0;
    virtual void handle(BBSSingleStep &command) = 0;
    virtual void handle(BBSPredictStep &command) = 0;
    virtual void handle(BBSSubtractStep &command) = 0;
    virtual void handle(BBSCorrectStep &command) = 0;
    virtual void handle(BBSSolveStep &command) = 0;
    virtual void handle(BBSShiftStep &command) = 0;
    virtual void handle(BBSRefitStep &command) = 0;
};

inline CommandHandler::~CommandHandler()
{
}

} //# namespace BBS
} //# namespace LOFAR

#endif
