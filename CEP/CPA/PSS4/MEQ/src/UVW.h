//# UVW.h: Calculate station UVW from station position and phase center
//#
//# Copyright (C) 2003
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

#ifndef MEQ_UVW_H
#define MEQ_UVW_H
    
#include <MEQ/Function.h>

#pragma aidgroup Meq
#pragma types #Meq::UVW #Meq::U

namespace Meq {    


class UVW : public Function
{
public:
  UVW();

  virtual ~UVW();

  // Get the result for the given request.
  virtual int getResultImpl (Result::Ref &resref, const Request&, bool newReq);

  // Check and convert the children.
  void checkChildren();

  Result::Ref& getU()
    { return itsRefU; }
  Result::Ref& getV()
    { return itsRefV; }
  Result::Ref& getW()
    { return itsRefW; }

private:
  Result itsU;
  Result itsV;
  Result itsW;
  Result::Ref itsRefU;
  Result::Ref itsRefV;
  Result::Ref itsRefW;
};


class UVWFunc : public Function
{
public:
  UVWFunc();

  virtual ~UVWFunc();

  // Get the result for the given request.
  void makeResult (Result::Ref &resref, const Request&, const Result& res);

  // Check and convert the children.
  void checkChildren();

protected:
  UVW* itsUVW;
};


class U : public UVWFunc
{
public:
  U();

  virtual ~U();

  // Get the result for the given request.
  virtual int getResultImpl (Result::Ref &resref, const Request&, bool newReq);
};


class V : public UVWFunc
{
public:
  V();

  virtual ~V();

  // Get the result for the given request.
  virtual int getResultImpl (Result::Ref &resref, const Request&, bool newReq);
};


class W : public UVWFunc
{
public:
  W();

  virtual ~W();

  // Get the result for the given request.
  virtual int getResultImpl (Result::Ref &resref, const Request&, bool newReq);
};


} // namespace Meq

#endif
