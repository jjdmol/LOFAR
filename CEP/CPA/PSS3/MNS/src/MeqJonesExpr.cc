//# MeqJonesExpr.cc: The base class of a Jones matrix expression.
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

#include <Common/Profiling/PerfProfile.h>

#include <MNS/MeqJonesExpr.h>
#include <Common/Debug.h>

MeqJonesExpr::~MeqJonesExpr()
{}


void MeqJonesExpr::multiply (const MeqJonesExpr& left,
			     const MeqJonesExpr& right)
{
  PERFPROFILE(__PRETTY_FUNCTION__);
  const MeqResult& l11 = left.getResult11();
  const MeqResult& l12 = left.getResult12();
  const MeqResult& l21 = left.getResult21();
  const MeqResult& l22 = left.getResult22();
  const MeqResult& r11 = right.getResult11();
  const MeqResult& r12 = right.getResult12();
  const MeqResult& r21 = right.getResult21();
  const MeqResult& r22 = right.getResult22();
  bool isDouble = l11.getValue().isDouble()  &&  l12.getValue().isDouble()
              &&  l21.getValue().isDouble()  &&  l22.getValue().isDouble()
              &&  r11.getValue().isDouble()  &&  r22.getValue().isDouble()
              &&  r21.getValue().isDouble()  &&  r22.getValue().isDouble();
  int nx = std::max(l11.getValue().nx(), r11.getValue().nx());
  int ny = std::max(l11.getValue().ny(), r11.getValue().ny());
  int nl = l11.getValue().nelements();
  int nr = r11.getValue().nelements();
  Assert(nl==nr);

  if (isDouble) {
    double* v11 = its11.setDouble (nx, ny);
    double* v12 = its12.setDouble (nx, ny);
    double* v21 = its21.setDouble (nx, ny);
    double* v22 = its22.setDouble (nx, ny);
    const double* dl11 = l11.getValue().doubleStorage();
    const double* dl12 = l12.getValue().doubleStorage();
    const double* dl21 = l21.getValue().doubleStorage();
    const double* dl22 = l22.getValue().doubleStorage();
    const double* dr11 = r11.getValue().doubleStorage();
    const double* dr12 = r12.getValue().doubleStorage();
    const double* dr21 = r21.getValue().doubleStorage();
    const double* dr22 = r22.getValue().doubleStorage();
    for (int i=0; i<nl; i++) {
      v11[i] = dl11[i] * dr11[i] + dl12[i] * dr21[i];
      v12[i] = dl11[i] * dr12[i] + dl12[i] * dr22[i];
      v21[i] = dl21[i] * dr11[i] + dl22[i] * dr21[i];
      v22[i] = dl21[i] * dr12[i] + dl22[i] * dr22[i];
    }
  } else {
    its11.setDComplex (std::max(l11.getValue().nx(), r11.getValue().nx()),
		       std::max(l11.getValue().ny(), r11.getValue().ny()));
    dcomplex* v11 = its11.setDComplex (nx, ny);
    dcomplex* v12 = its12.setDComplex (nx, ny);
    dcomplex* v21 = its21.setDComplex (nx, ny);
    dcomplex* v22 = its22.setDComplex (nx, ny);
    nl = l11.getValue().nelements();
    nr = r11.getValue().nelements();
    Assert(nl==nr);
    const dcomplex* dl11 = l11.getValue().dcomplexStorage();
    const dcomplex* dl12 = l12.getValue().dcomplexStorage();
    const dcomplex* dl21 = l21.getValue().dcomplexStorage();
    const dcomplex* dl22 = l22.getValue().dcomplexStorage();
    const dcomplex* dr11 = r11.getValue().dcomplexStorage();
    const dcomplex* dr12 = r12.getValue().dcomplexStorage();
    const dcomplex* dr21 = r21.getValue().dcomplexStorage();
    const dcomplex* dr22 = r22.getValue().dcomplexStorage();
    for (int i=0; i<nl; i++) {
      v11[i] = dl11[i] * dr11[i] + dl12[i] * dr21[i];
      v12[i] = dl11[i] * dr12[i] + dl12[i] * dr22[i];
      v21[i] = dl21[i] * dr11[i] + dl22[i] * dr21[i];
      v22[i] = dl21[i] * dr12[i] + dl22[i] * dr22[i];
    }
  }

  int np11 = std::max(l11.nperturbed(), r11.nperturbed());
  int np12 = std::max(l12.nperturbed(), r12.nperturbed());
  int np21 = std::max(l21.nperturbed(), r21.nperturbed());
  int np22 = std::max(l22.nperturbed(), r22.nperturbed());
  int npert = std::max(np11, std::max(np12, std::max(np21, np22)));
  for (int j=0; j<npert; j++) {
    MeqMatrix perturbation;
    bool eval = false;
    if (l11.isDefined(j)) {
      eval = true;
      perturbation = l11.getPerturbation(j);
    } else if (l12.isDefined(j)) {
      eval = true;
      perturbation = l12.getPerturbation(j);
    } else if (l21.isDefined(j)) {
      eval = true;
      perturbation = l21.getPerturbation(j);
    } else if (l22.isDefined(j)) {
      eval = true;
      perturbation = l22.getPerturbation(j);
    } else if (r11.isDefined(j)) {
      eval = true;
      perturbation = r11.getPerturbation(j);
    } else if (r12.isDefined(j)) {
      eval = true;
      perturbation = r12.getPerturbation(j);
    } else if (r21.isDefined(j)) {
      eval = true;
      perturbation = r21.getPerturbation(j);
    } else if (r22.isDefined(j)) {
      eval = true;
      perturbation = r22.getPerturbation(j);
    }
    if (eval) {
      its11.setPerturbation (j, perturbation);
      its12.setPerturbation (j, perturbation);
      its21.setPerturbation (j, perturbation);
      its22.setPerturbation (j, perturbation);
      if (isDouble) {
	double* v11 = its11.setPerturbedDouble (j, nx, ny);
	double* v12 = its12.setPerturbedDouble (j, nx, ny);
	double* v21 = its21.setPerturbedDouble (j, nx, ny);
	double* v22 = its22.setPerturbedDouble (j, nx, ny);
	const double* pl11 = l11.getPerturbedValue(j).doubleStorage();
	const double* pl12 = l12.getPerturbedValue(j).doubleStorage();
	const double* pl21 = l21.getPerturbedValue(j).doubleStorage();
	const double* pl22 = l22.getPerturbedValue(j).doubleStorage();
	const double* pr11 = r11.getPerturbedValue(j).doubleStorage();
	const double* pr12 = r12.getPerturbedValue(j).doubleStorage();
	const double* pr21 = r21.getPerturbedValue(j).doubleStorage();
	const double* pr22 = r22.getPerturbedValue(j).doubleStorage();
	for (int i=0; i<nl; i++) {
	  v11[i] = pl11[i] * pr11[i] + pl12[i] * pr21[i];
	  v12[i] = pl11[i] * pr12[i] + pl12[i] * pr22[i];
	  v21[i] = pl21[i] * pr11[i] + pl22[i] * pr21[i];
	  v22[i] = pl21[i] * pr12[i] + pl22[i] * pr22[i];
	}
      } else {
	dcomplex* v11 = its11.setPerturbedDComplex (j, nx, ny);
	dcomplex* v12 = its12.setPerturbedDComplex (j, nx, ny);
	dcomplex* v21 = its21.setPerturbedDComplex (j, nx, ny);
	dcomplex* v22 = its22.setPerturbedDComplex (j, nx, ny);
	const dcomplex* pl11 = l11.getPerturbedValue(j).dcomplexStorage();
	const dcomplex* pl12 = l12.getPerturbedValue(j).dcomplexStorage();
	const dcomplex* pl21 = l21.getPerturbedValue(j).dcomplexStorage();
	const dcomplex* pl22 = l22.getPerturbedValue(j).dcomplexStorage();
	const dcomplex* pr11 = r11.getPerturbedValue(j).dcomplexStorage();
	const dcomplex* pr12 = r12.getPerturbedValue(j).dcomplexStorage();
	const dcomplex* pr21 = r21.getPerturbedValue(j).dcomplexStorage();
	const dcomplex* pr22 = r22.getPerturbedValue(j).dcomplexStorage();
	for (int i=0; i<nl; i++) {
	  v11[i] = pl11[i] * pr11[i] + pl12[i] * pr21[i];
	  v12[i] = pl11[i] * pr12[i] + pl12[i] * pr22[i];
	  v21[i] = pl21[i] * pr11[i] + pl22[i] * pr21[i];
	  v22[i] = pl21[i] * pr12[i] + pl22[i] * pr22[i];
	}
      }
    }
  }
}

