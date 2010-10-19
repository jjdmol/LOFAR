//# MessageHandler.h:
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSKERNEL_MESSAGEHANDLER_H
#define LOFAR_BBSKERNEL_MESSAGEHANDLER_H

namespace LOFAR
{
namespace BBS
{
class CoeffIndexMsg;
class CoeffMsg;
class EquationMsg;
class SolutionMsg;
class ChunkDoneMsg;

// \addtogroup BBSKernel
// @{

// Abstract %Visitor class MessageHandler declares handle() operations
// for each concrete Message class. Concrete handler classes must
// implement a handle() method for each concrete Message class.
//
// \see Gamma e.a., <em>Design Patterns: Elements of Reusable
// Object-Oriented Software</em>, Addison-Wesley, 1995
// \see <a href="http://hillside.net/patterns/papers/type-laundering.html">
// John Vlissides, <em>Type Laundering</em>, C++ Report, February 1997</a>
class MessageHandler
{
public:
  virtual ~MessageHandler() {}

  virtual void handle(const CoeffIndexMsg &message) = 0;
  virtual void handle(const CoeffMsg &message) = 0;
  virtual void handle(const EquationMsg &message) = 0;
  virtual void handle(const SolutionMsg &message) = 0;
  virtual void handle(const ChunkDoneMsg &message) = 0;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
