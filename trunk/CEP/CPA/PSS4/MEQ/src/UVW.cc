//# UVW.cc: Calculate station UVW from station position and phase center
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

#include <MEQ/UVW.h>
#include <MEQ/Request.h>
#include <MEQ/Result.h>
#include <MEQ/Cells.h>
#include <aips/Measures/MBaseline.h>
#include <aips/Measures/MPosition.h>
#include <aips/Measures/MEpoch.h>
#include <aips/Measures/MeasConvert.h>
#include <aips/Quanta/MVuvw.h>

namespace Meq {


UVW::UVW()
{
  ///  itsRefU = itsU;
}

UVW::~UVW()
{}

int UVW::getResultImpl (Result::Ref& resref, const Request& request, bool)
{
  const Cells& cells = request.cells();
  // Get RA and DEC of phase center.
  Result::Ref ra, dec;
  int flag = children()[0]->getResult (ra, request);
  flag |= children()[1]->getResult (dec, request);
  ra.persist();
  dec.persist();
  // Get station positions.
  Result::Ref stx, sty, stz;
  flag |= children()[2]->getResult (stx, request);
  flag |= children()[3]->getResult (sty, request);
  flag |= children()[4]->getResult (stz, request);
  stx.persist();
  sty.persist();
  stz.persist();
  if (flag & Node::RES_WAIT) {
    return flag;
  }
  // For the time being we only support scalars.
  const Vells& vra  = ra->getValue();
  const Vells& vdec = dec->getValue();
  const Vells& vstx = stx->getValue();
  const Vells& vsty = sty->getValue();
  const Vells& vstz = stz->getValue();
  Assert (vra.nelements()==1 && vdec.nelements()==1
	  && vstx.nelements()==1 && vsty.nelements()==1
	  && vstz.nelements()==1);
  // Get RA and DEC of phase center.
  MVDirection phaseRef (vra.getRealScalar(), vdec.getRealScalar());
  // Set correct size of values.
  int ntime = cells.ntime();
  LoMat_double& matU = itsU.setReal (1, ntime);
  LoMat_double& matV = itsV.setReal (1, ntime);
  LoMat_double& matW = itsW.setReal (1, ntime);
  double* uptr = matU.data();
  double* vptr = matV.data();
  double* wptr = matW.data();
  // Calculate the UVW coordinates using the AIPS++ code.
  MVPosition mvpos(vstx.getRealScalar(),
		   vsty.getRealScalar(),
		   vstz.getRealScalar());
  MVBaseline mvbl(mvpos);
  MBaseline mbl(mvbl, MBaseline::ITRF);
  Quantum<double> qepoch(0, "s");
  qepoch.setValue (cells.time(0));
  MEpoch mepoch(qepoch, MEpoch::UTC);
  MeasFrame frame;
  frame.set (mepoch);
  mbl.getRefPtr()->set(frame);      // attach frame
  MBaseline::Convert mcvt(mbl, MBaseline::J2000);
  for (Int i=0; i<cells.ntime(); i++) {
    qepoch.setValue (cells.time(i));
    mepoch.set (qepoch);
    frame.set (mepoch);
    const MVBaseline& bas2000 = mcvt().getValue();
    MVuvw uvw2000 (bas2000, phaseRef);
    const Vector<double>& xyz = uvw2000.getValue();
    *uptr++ = xyz(0);
    *vptr++ = xyz(1);
    *wptr++ = xyz(2);
  }
  return 0;
}

void UVW::checkChildren()
{
  Function::convertChildren (5);
}



UVWFunc::UVWFunc()
: itsUVW(0)
{}

UVWFunc::~UVWFunc()
{}

void UVWFunc::makeResult (Result::Ref& resref, const Request& request,
			  const Result& res)
{
  // Get cells.
  const Cells& cells = request.cells();
  int nfreq = cells.nfreq();
  int ntime = cells.ntime();
  // Create result object and attach to the ref that was passed in.
  Result& result = resref <<= new Result();
  // Evaluate the main value.
  LoMat_double& arr = result.setReal (nfreq,ntime);
  const LoMat_double& val = res.getValue().getRealArray();
  for (int i=0; i<ntime; i++) {
    double time = cells.time(i);
    for (int j=0; j<nfreq; j++) {
      arr(j,i) = val(1,i);
    }
  }
  // Evaluate the perturbed values.
  for (int k=0; k<res.nperturbed(); k++) {
    LoMat_double& arr = result.setPerturbedReal (k,nfreq,ntime);
    const LoMat_double& val = res.getPerturbedValue(k).getRealArray();
    for (int i=0; i<ntime; i++) {
      double time = cells.time(i);
      for (int j=0; j<nfreq; j++) {
	arr(j,i) = val(1,i);
      }
    }
  }
  result.setSpids (res.getSpids());
}

void UVWFunc::checkChildren()
{
  if (Function::convertChildren (1)) {
    itsUVW = dynamic_cast<UVW*>(children()[0]);
    AssertMsg (itsUVW, "Child of MeqU,MeqV,MeqW node must be a MeqUVW");
  }
}



U::U()
{}

U::~U()
{}

int U::getResultImpl (Result::Ref& resref, const Request& request, bool)
{
  Result::Ref tmp;
  int flag = itsUVW->getResult (tmp, request);
  if (flag) {
    makeResult (resref, request, itsUVW->getU().deref());
  }
  return flag;
}



V::V()
{}

V::~V()
{}

int V::getResultImpl (Result::Ref& resref, const Request& request, bool)
{
  Result::Ref tmp;
  int flag = itsUVW->getResult (tmp, request);
  if (flag) {
    makeResult (resref, request, itsUVW->getU().deref());
  }
  return flag;
}



W::W()
{}

W::~W()
{}

int W::getResultImpl (Result::Ref& resref, const Request& request, bool)
{
  Result::Ref tmp;
  int flag = itsUVW->getResult (tmp, request);
  if (flag) {
    makeResult (resref, request, itsUVW->getU().deref());
  }
  return flag;
}



} // namespace Meq
