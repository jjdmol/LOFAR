//# MeqRequest.cc: The request for an evaluation of an expression
//#
//# Copyright (C) 2002
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

#include <lofar_config.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <Common/LofarLogger.h>


namespace LOFAR
{
namespace BBS
{

MeqRequestId MeqRequest::theirRequestId = 0;


MeqRequest::MeqRequest (const MeqDomain& domain, int nx, int ny, int nrSpid)
: itsRequestId (theirRequestId++),
  itsSourceNr  (0),
  itsNspids    (nrSpid),
  itsOffset    (0, 0)
{
  setDomain (domain, nx, ny);
}

MeqRequest::MeqRequest (const MeqDomain& domain, int nx,
			const vector<double>& y,
			int nrSpid)
: itsRequestId (theirRequestId++),
  itsSourceNr  (0),
  itsNspids    (nrSpid),
  itsOffset    (0, 0)
{
  setDomain (domain, nx, y);
}

MeqRequest::MeqRequest (const MeqRequest& req,
			unsigned int stx, unsigned int nrx, unsigned int sty, unsigned int nry)
: itsRequestId (req.itsRequestId),
  itsStepX     (req.itsStepX),
  itsSourceNr  (req.itsSourceNr),
  itsNspids    (req.itsNspids),
  itsOffset    (req.itsOffset)
{
  ASSERT (int(stx+nrx) <= req.itsNx  &&  int(sty+nry) <= req.itsNy);
  itsNx = nrx;
  itsNy = nry;
  itsYP = req.itsYP + sty;
  itsDomain = MeqDomain (req.itsDomain.startX() + stx*itsStepX,
			 req.itsDomain.startX() + (stx+nrx)*itsStepX,
			 itsYP[0], itsYP[nry]);
}

MeqRequest::MeqRequest (const MeqRequest& req)
: itsRequestId (req.itsRequestId),
  itsDomain    (req.itsDomain),
  itsNx        (req.itsNx),
  itsNy        (req.itsNy),
  itsStepX     (req.itsStepX),
  itsY         (req.itsY),
  itsYP        (req.itsYP),
  itsSourceNr  (req.itsSourceNr),
  itsNspids    (req.itsNspids),
  itsOffset    (req.itsOffset)
{
  if (itsY.size() > 0) {
    itsYP = &itsY[0];
  }
}

MeqRequest& MeqRequest::operator= (const MeqRequest& req)
{
  if (this != &req) {
    itsRequestId = req.itsRequestId;
    itsDomain    = req.itsDomain;
    itsNx        = req.itsNx;
    itsNy        = req.itsNy;
    itsStepX     = req.itsStepX;
    itsY         = req.itsY;
    itsYP        = req.itsYP;
    itsSourceNr  = req.itsSourceNr;
    itsNspids    = req.itsNspids;
    itsOffset    = req.itsOffset;
    if (itsY.size() > 0) {
      itsYP = &itsY[0];
    }
  }
  return *this;
}

void MeqRequest::setDomain (const MeqDomain& domain, int nx, int ny)
{
  itsDomain = domain;
  itsNx     = nx;
  itsNy     = ny;
  itsStepX = (domain.endX() - domain.startX()) / nx;
  double stepy = (domain.endY() - domain.startY()) / ny;
  itsY.resize (ny+1);
  for (int i=0; i<=ny; ++i) {
    itsY[i] = domain.startY() + i*stepy;
  }
  itsYP = &itsY[0];
}

void MeqRequest::setDomain (const MeqDomain& domain, int nx,
			    const vector<double>& y)
{
  
  itsDomain = domain;
  itsNx     = nx;
  itsNy     = y.size() - 1;
  itsStepX  = (domain.endX() - domain.startX()) / nx;
  ASSERT (itsNy > 0);
  for (int i=0; i<itsNy; ++i) {
    ASSERT (y[i] < y[i+1]);
  }
  itsY  = y;
  itsYP = &itsY[0];
}

} // namespace BBS
} // namespace LOFAR
