//# MeqJonesCMul3.cc: Calculate left*mid*conj(right)
//#
//# Copyright (C) 2005
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
#include <Common/Profiling/PerfProfile.h>
#include <Common/Timer.h>

#include <BBS3/MNS/MeqJonesCMul3.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqJonesResult.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

#if defined __SSE2__
#include <emmintrin.h>
#include <xmmintrin.h>
#endif


using namespace casa;

namespace LOFAR {

MeqJonesCMul3::MeqJonesCMul3 (const MeqJonesExpr& left,
			      const MeqJonesExpr& mid,
			      const MeqJonesExpr& right)
: itsLeft  (left),
  itsMid   (mid),
  itsRight (right)
{
  itsLeft.incrNParents();
  itsMid.incrNParents();
  itsRight.incrNParents();
}

MeqJonesCMul3::~MeqJonesCMul3()
{}


#if 0 && defined __SSE2__
// this hack forces gcc to inline
#define _mm_mul_pd(A,B) mul(A,B)
#define _mm_add_pd(A,B) add(A,B)
#define _mm_sub_pd(A,B) sub(A,B)
inline __m128d mul(__m128d a, __m128d b)
{
    asm ("mulpd %2,%0" : "=x" (b) : "0" (b), "x" (a));
    return b;
}
inline __m128d add(__m128d a, __m128d b)
{
    asm ("addpd %2,%0" : "=x" (b) : "0" (b), "x" (a));
    return b;
}
inline __m128d sub(__m128d b, __m128d a)
{
    asm ("subpd %2,%0" : "=x" (b) : "0" (b), "x" (a));
    return b;
}
#endif


#if defined __SSE2__

static void getResultSSE2 (
  MeqResult &result11, MeqResult &result12,
  MeqResult &result21, MeqResult &result22,
  const MeqRequest &request,
  const MeqResult &l11, const MeqResult &l12,
  const MeqResult &l21, const MeqResult &l22,
  const MeqResult &m11, const MeqResult &m12,
  const MeqResult &m21, const MeqResult &m22,
  const MeqResult &r11, const MeqResult &r12,
  const MeqResult &r21, const MeqResult &r22,
  const MeqMatrix &ml11, const MeqMatrix &ml12,
  const MeqMatrix &ml21, const MeqMatrix &ml22,
  const MeqMatrix &mm11, const MeqMatrix &mm12,
  const MeqMatrix &mm21, const MeqMatrix &mm22,
  const MeqMatrix &mr11, const MeqMatrix &mr12,
  const MeqMatrix &mr21, const MeqMatrix &mr22)
{
  __m128d t11_r, t11_i, t12_r, t12_i, t21_r, t21_i, t22_r, t22_i;
  dcomplex v;
  int n, nx = mm11.nx(), ny = mm11.ny();

  v = ml11.getDComplex();
  __m128d l11_r = _mm_set1_pd(real(v)), l11_i = _mm_set1_pd(imag(v));
  v = ml12.getDComplex();
  __m128d l12_r = _mm_set1_pd(real(v)), l12_i = _mm_set1_pd(imag(v));
  v = ml21.getDComplex();
  __m128d l21_r = _mm_set1_pd(real(v)), l21_i = _mm_set1_pd(imag(v));
  v = ml22.getDComplex();
  __m128d l22_r = _mm_set1_pd(real(v)), l22_i = _mm_set1_pd(imag(v));

  __m128d *m11_r, *m11_i;
  mm11.dcomplexStorage((const double *&) m11_r, (const double *&) m11_i);
  __m128d *m12_r, *m12_i;
  mm12.dcomplexStorage((const double *&) m12_r, (const double *&) m12_i);
  __m128d *m21_r, *m21_i;
  mm21.dcomplexStorage((const double *&) m21_r, (const double *&) m21_i);
  __m128d *m22_r, *m22_i;
  mm22.dcomplexStorage((const double *&) m22_r, (const double *&) m22_i);

  v = mr11.getDComplex();
  __m128d r11_r = _mm_set1_pd(real(v)), r11_i = _mm_set1_pd(imag(v));
  v = mr12.getDComplex();
  __m128d r12_r = _mm_set1_pd(real(v)), r12_i = _mm_set1_pd(imag(v));
  v = mr21.getDComplex();
  __m128d r21_r = _mm_set1_pd(real(v)), r21_i = _mm_set1_pd(imag(v));
  v = mr22.getDComplex();
  __m128d r22_r = _mm_set1_pd(real(v)), r22_i = _mm_set1_pd(imag(v));

  MeqMatrix v11(v, nx, ny, false);
  MeqMatrix v12(v, nx, ny, false);
  MeqMatrix v21(v, nx, ny, false);
  MeqMatrix v22(v, nx, ny, false);

  __m128d *v11_r, *v11_i;
  v11.dcomplexStorage((const double *&) v11_r, (const double *&) v11_i);
  __m128d *v12_r, *v12_i;
  v12.dcomplexStorage((const double *&) v12_r, (const double *&) v12_i);
  __m128d *v21_r, *v21_i;
  v21.dcomplexStorage((const double *&) v21_r, (const double *&) v21_i);
  __m128d *v22_r, *v22_i;
  v22.dcomplexStorage((const double *&) v22_r, (const double *&) v22_i);
  n = (mm11.rep()->nelements() + 1) / 2;

  // now this is programming ...

  for (int _i = 0; _i < n; _i ++) {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m11_r[_i]),_mm_mul_pd(l11_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m21_r[_i]),_mm_mul_pd(l12_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m11_i[_i]),_mm_mul_pd(l11_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m21_i[_i]),_mm_mul_pd(l12_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m12_r[_i]),_mm_mul_pd(l11_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m22_r[_i]),_mm_mul_pd(l12_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m12_i[_i]),_mm_mul_pd(l11_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m22_i[_i]),_mm_mul_pd(l12_i,m22_r[_i])));
t21_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m11_r[_i]),_mm_mul_pd(l21_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m21_r[_i]),_mm_mul_pd(l22_i,m21_i[_i])));
t21_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m11_i[_i]),_mm_mul_pd(l21_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m21_i[_i]),_mm_mul_pd(l22_i,m21_r[_i])));
t22_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m12_r[_i]),_mm_mul_pd(l21_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m22_r[_i]),_mm_mul_pd(l22_i,m22_i[_i])));
t22_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m12_i[_i]),_mm_mul_pd(l21_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m22_i[_i]),_mm_mul_pd(l22_i,m22_r[_i])));
v11_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v11_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
v12_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v12_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
v21_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t21_r,r11_r),_mm_mul_pd(t21_i,r11_i)),_mm_add_pd(_mm_mul_pd(t22_r,r12_r),_mm_mul_pd(t22_i,r12_i)));
v21_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t21_i,r11_r),_mm_mul_pd(t21_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t22_i,r12_r),_mm_mul_pd(t22_r,r12_i)));
v22_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t21_r,r21_r),_mm_mul_pd(t21_i,r21_i)),_mm_add_pd(_mm_mul_pd(t22_r,r22_r),_mm_mul_pd(t22_i,r22_i)));
v22_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t21_i,r21_r),_mm_mul_pd(t21_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t22_i,r22_r),_mm_mul_pd(t22_r,r22_i)));
  }
  
  result11.setValue (v11);
  result12.setValue (v12);
  result21.setValue (v21);
  result22.setValue (v22); 

  // Determine which values are perturbed and determine the perturbation.
  double perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval11 = false;
    bool eval12 = false;
    bool eval21 = false;
    bool eval22 = false;
    if (m11.isDefined(spinx)) {
      eval11 = true;
      perturbation = m11.getPerturbation(spinx);
    } else if (m12.isDefined(spinx)) {
      eval11 = true;
      perturbation = m12.getPerturbation(spinx);
    } else if (m21.isDefined(spinx)) {
      eval11 = true;
      perturbation = m21.getPerturbation(spinx);
    } else if (m22.isDefined(spinx)) {
      eval11 = true;
      perturbation = m22.getPerturbation(spinx);
    }
    if (eval11) {
      eval12 = eval21 = eval22 = true;
    } else {
      if (l11.isDefined(spinx)) {
	perturbation = l11.getPerturbation(spinx);
	eval11 = true;
	eval12 = true;
      } else if (l12.isDefined(spinx)) {
	perturbation = l12.getPerturbation(spinx);
	eval11 = true;
	eval12 = true;
      }
      if (l21.isDefined(spinx)) {
	perturbation = l21.getPerturbation(spinx);
	eval21 = true;
	eval22 = true;
      } else if (l22.isDefined(spinx)) {
	perturbation = l22.getPerturbation(spinx);
	eval21 = true;
	eval22 = true;
      }
      if (r11.isDefined(spinx)) {
	perturbation = r11.getPerturbation(spinx);
	eval11 = true;
	eval21 = true;
      } else if (r12.isDefined(spinx)) {
	perturbation = r12.getPerturbation(spinx);
	eval11 = true;
	eval21 = true;
      }
      if (r21.isDefined(spinx)) {
	perturbation = r21.getPerturbation(spinx);
	eval12 = true;
	eval22 = true;
      } else if (r22.isDefined(spinx)) {
	perturbation = r22.getPerturbation(spinx);
	eval12 = true;
	eval22 = true;
      }
    }
    if (eval11 || eval12 || eval21 || eval22) {
      const MeqMatrix& ml11 = l11.getPerturbedValue(spinx);
      const MeqMatrix& ml12 = l12.getPerturbedValue(spinx);
      const MeqMatrix& ml21 = l21.getPerturbedValue(spinx);
      const MeqMatrix& ml22 = l22.getPerturbedValue(spinx);
      const MeqMatrix& mm11 = m11.getPerturbedValue(spinx);
      const MeqMatrix& mm12 = m12.getPerturbedValue(spinx);
      const MeqMatrix& mm21 = m21.getPerturbedValue(spinx);
      const MeqMatrix& mm22 = m22.getPerturbedValue(spinx);
      const MeqMatrix& mr11 = r11.getPerturbedValue(spinx);
      const MeqMatrix& mr12 = r12.getPerturbedValue(spinx);
      const MeqMatrix& mr21 = r21.getPerturbedValue(spinx);
      const MeqMatrix& mr22 = r22.getPerturbedValue(spinx);

      if (eval11 || eval12) {
	v = ml11.getDComplex();
	__m128d l11_r = _mm_set1_pd(real(v)), l11_i = _mm_set1_pd(imag(v));
	v = ml12.getDComplex();
	__m128d l12_r = _mm_set1_pd(real(v)), l12_i = _mm_set1_pd(imag(v));

	__m128d *m11_r, *m11_i;
	mm11.dcomplexStorage((const double *&) m11_r, (const double *&) m11_i);
	__m128d *m12_r, *m12_i;
	mm12.dcomplexStorage((const double *&) m12_r, (const double *&) m12_i);
	__m128d *m21_r, *m21_i;
	mm21.dcomplexStorage((const double *&) m21_r, (const double *&) m21_i);
	__m128d *m22_r, *m22_i;
	mm22.dcomplexStorage((const double *&) m22_r, (const double *&) m22_i);

	if (eval11 && eval12) {
	  v = mr11.getDComplex();
	  __m128d r11_r = _mm_set1_pd(real(v)), r11_i = _mm_set1_pd(imag(v));
	  v = mr12.getDComplex();
	  __m128d r12_r = _mm_set1_pd(real(v)), r12_i = _mm_set1_pd(imag(v));
	  v = mr21.getDComplex();
	  __m128d r21_r = _mm_set1_pd(real(v)), r21_i = _mm_set1_pd(imag(v));
	  v = mr22.getDComplex();
	  __m128d r22_r = _mm_set1_pd(real(v)), r22_i = _mm_set1_pd(imag(v));

	  MeqMatrix &v11 = result11.getPerturbedValueRW(spinx);
	  MeqMatrix &v12 = result12.getPerturbedValueRW(spinx);
	  v11.setDCMat(nx, ny);
	  v12.setDCMat(nx, ny);

	  __m128d *v11_r, *v11_i;
	  v11.dcomplexStorage((const double *&) v11_r, (const double *&) v11_i);
	  __m128d *v12_r, *v12_i;
	  v12.dcomplexStorage((const double *&) v12_r, (const double *&) v12_i);

	  for (int _i = 0; _i < n; _i ++) {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m11_r[_i]),_mm_mul_pd(l11_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m21_r[_i]),_mm_mul_pd(l12_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m11_i[_i]),_mm_mul_pd(l11_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m21_i[_i]),_mm_mul_pd(l12_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m12_r[_i]),_mm_mul_pd(l11_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m22_r[_i]),_mm_mul_pd(l12_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m12_i[_i]),_mm_mul_pd(l11_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m22_i[_i]),_mm_mul_pd(l12_i,m22_r[_i])));
v11_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v11_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
v12_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v12_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
	  }

	  result11.setPerturbation (spinx, perturbation);
	  result12.setPerturbation (spinx, perturbation);
	} else if (eval11) {
	  v = mr11.getDComplex();
	  __m128d r11_r = _mm_set1_pd(real(v)), r11_i = _mm_set1_pd(imag(v));
	  v = mr12.getDComplex();
	  __m128d r12_r = _mm_set1_pd(real(v)), r12_i = _mm_set1_pd(imag(v));

	  MeqMatrix &v11 = result11.getPerturbedValueRW(spinx);
	  v11.setDCMat(nx, ny);

	  __m128d *v11_r, *v11_i;
	  v11.dcomplexStorage((const double *&) v11_r, (const double *&) v11_i);

	  for (int _i = 0; _i < n; _i ++) {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m11_r[_i]),_mm_mul_pd(l11_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m21_r[_i]),_mm_mul_pd(l12_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m11_i[_i]),_mm_mul_pd(l11_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m21_i[_i]),_mm_mul_pd(l12_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m12_r[_i]),_mm_mul_pd(l11_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m22_r[_i]),_mm_mul_pd(l12_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m12_i[_i]),_mm_mul_pd(l11_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m22_i[_i]),_mm_mul_pd(l12_i,m22_r[_i])));
v11_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v11_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
	  }

	  result11.setPerturbation (spinx, perturbation);
	} else /*if (eval12)*/ {
	  v = mr21.getDComplex();
	  __m128d r21_r = _mm_set1_pd(real(v)), r21_i = _mm_set1_pd(imag(v));
	  v = mr22.getDComplex();
	  __m128d r22_r = _mm_set1_pd(real(v)), r22_i = _mm_set1_pd(imag(v));

	  MeqMatrix &v12 = result12.getPerturbedValueRW(spinx);
	  v12.setDCMat(nx, ny);

	  __m128d *v12_r, *v12_i;
	  v12.dcomplexStorage((const double *&) v12_r, (const double *&) v12_i);

	  for (int _i = 0; _i < n; _i ++) {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m11_r[_i]),_mm_mul_pd(l11_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m21_r[_i]),_mm_mul_pd(l12_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m11_i[_i]),_mm_mul_pd(l11_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m21_i[_i]),_mm_mul_pd(l12_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m12_r[_i]),_mm_mul_pd(l11_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m22_r[_i]),_mm_mul_pd(l12_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m12_i[_i]),_mm_mul_pd(l11_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m22_i[_i]),_mm_mul_pd(l12_i,m22_r[_i])));
v12_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v12_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
      }

	  result12.setPerturbation (spinx, perturbation);
	}
      }

      if (eval21 || eval22) {
	v = ml21.getDComplex();
	__m128d l21_r = _mm_set1_pd(real(v)), l21_i = _mm_set1_pd(imag(v));
	v = ml22.getDComplex();
	__m128d l22_r = _mm_set1_pd(real(v)), l22_i = _mm_set1_pd(imag(v));

	__m128d *m11_r, *m11_i;
	mm11.dcomplexStorage((const double *&) m11_r, (const double *&) m11_i);
	__m128d *m12_r, *m12_i;
	mm12.dcomplexStorage((const double *&) m12_r, (const double *&) m12_i);
	__m128d *m21_r, *m21_i;
	mm21.dcomplexStorage((const double *&) m21_r, (const double *&) m21_i);
	__m128d *m22_r, *m22_i;
	mm22.dcomplexStorage((const double *&) m22_r, (const double *&) m22_i);

	if (eval21 && eval22) {
	  v = mr11.getDComplex();
	  __m128d r11_r = _mm_set1_pd(real(v)), r11_i = _mm_set1_pd(imag(v));
	  v = mr12.getDComplex();
	  __m128d r12_r = _mm_set1_pd(real(v)), r12_i = _mm_set1_pd(imag(v));
	  v = mr21.getDComplex();
	  __m128d r21_r = _mm_set1_pd(real(v)), r21_i = _mm_set1_pd(imag(v));
	  v = mr22.getDComplex();
	  __m128d r22_r = _mm_set1_pd(real(v)), r22_i = _mm_set1_pd(imag(v));

	  MeqMatrix &v21 = result21.getPerturbedValueRW(spinx);
	  MeqMatrix &v22 = result22.getPerturbedValueRW(spinx);
	  v21.setDCMat(nx, ny);
	  v22.setDCMat(nx, ny);

	  __m128d *v21_r, *v21_i;
	  v21.dcomplexStorage((const double *&) v21_r, (const double *&) v21_i);
	  __m128d *v22_r, *v22_i;
	  v22.dcomplexStorage((const double *&) v22_r, (const double *&) v22_i);

	  for (int _i = 0; _i < n; _i ++) {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m11_r[_i]),_mm_mul_pd(l21_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m21_r[_i]),_mm_mul_pd(l22_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m11_i[_i]),_mm_mul_pd(l21_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m21_i[_i]),_mm_mul_pd(l22_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m12_r[_i]),_mm_mul_pd(l21_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m22_r[_i]),_mm_mul_pd(l22_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m12_i[_i]),_mm_mul_pd(l21_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m22_i[_i]),_mm_mul_pd(l22_i,m22_r[_i])));
v21_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v21_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
v22_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v22_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
	  }

	  result21.setPerturbation (spinx, perturbation);
	  result22.setPerturbation (spinx, perturbation);
	} else if (eval21) {
	  v = mr11.getDComplex();
	  __m128d r11_r = _mm_set1_pd(real(v)), r11_i = _mm_set1_pd(imag(v));
	  v = mr12.getDComplex();
	  __m128d r12_r = _mm_set1_pd(real(v)), r12_i = _mm_set1_pd(imag(v));

	  MeqMatrix &v21 = result21.getPerturbedValueRW(spinx);
	  v21.setDCMat(nx, ny);

	  __m128d *v21_r, *v21_i;
	  v21.dcomplexStorage((const double *&) v21_r, (const double *&) v21_i);

	  for (int _i = 0; _i < n; _i ++) {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m11_r[_i]),_mm_mul_pd(l21_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m21_r[_i]),_mm_mul_pd(l22_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m11_i[_i]),_mm_mul_pd(l21_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m21_i[_i]),_mm_mul_pd(l22_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m12_r[_i]),_mm_mul_pd(l21_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m22_r[_i]),_mm_mul_pd(l22_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m12_i[_i]),_mm_mul_pd(l21_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m22_i[_i]),_mm_mul_pd(l22_i,m22_r[_i])));
v21_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v21_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
}

	  result21.setPerturbation (spinx, perturbation);
	} else /*if (eval22)*/ {
	  v = mr21.getDComplex();
	  __m128d r21_r = _mm_set1_pd(real(v)), r21_i = _mm_set1_pd(imag(v));
	  v = mr22.getDComplex();
	  __m128d r22_r = _mm_set1_pd(real(v)), r22_i = _mm_set1_pd(imag(v));

	  MeqMatrix &v22 = result22.getPerturbedValueRW(spinx);
	  v22.setDCMat(nx, ny);

	  __m128d *v22_r, *v22_i;
	  v22.dcomplexStorage((const double *&) v22_r, (const double *&) v22_i);

	  for (int _i = 0; _i < n; _i ++) {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m11_r[_i]),_mm_mul_pd(l21_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m21_r[_i]),_mm_mul_pd(l22_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m11_i[_i]),_mm_mul_pd(l21_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m21_i[_i]),_mm_mul_pd(l22_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m12_r[_i]),_mm_mul_pd(l21_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m22_r[_i]),_mm_mul_pd(l22_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m12_i[_i]),_mm_mul_pd(l21_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m22_i[_i]),_mm_mul_pd(l22_i,m22_r[_i])));
v22_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v22_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
	}

	  result22.setPerturbation (spinx, perturbation);
	}
      }
    }
  }
}

#endif


MeqJonesResult MeqJonesCMul3::getResult (const MeqRequest& request)
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

// static NSTimer timer("MeqJonesCMul3::getResult", true);
  // Create the result object.
  MeqJonesResult result(request.nspid());
  MeqResult& result11 = result.result11();
  MeqResult& result12 = result.result12();
  MeqResult& result21 = result.result21();
  MeqResult& result22 = result.result22();
  // Calculate the children.
  MeqJonesResult leftBuf, midBuf, rightBuf;
  const MeqJonesResult& left  = itsLeft.getResultSynced (request, leftBuf);
  const MeqJonesResult& mid   = itsMid.getResultSynced (request, midBuf);
  const MeqJonesResult& right = itsRight.getResultSynced (request, rightBuf);
  const MeqResult& l11 = left.getResult11();
  const MeqResult& l12 = left.getResult12();
  const MeqResult& l21 = left.getResult21();
  const MeqResult& l22 = left.getResult22();
  const MeqResult& m11 = mid.getResult11();
  const MeqResult& m12 = mid.getResult12();
  const MeqResult& m21 = mid.getResult21();
  const MeqResult& m22 = mid.getResult22();
  const MeqResult& r11 = right.getResult11();
  const MeqResult& r12 = right.getResult12();
  const MeqResult& r21 = right.getResult21();
  const MeqResult& r22 = right.getResult22();
  const MeqMatrix& ml11 = l11.getValue();
  const MeqMatrix& ml12 = l12.getValue();
  const MeqMatrix& ml21 = l21.getValue();
  const MeqMatrix& ml22 = l22.getValue();
  const MeqMatrix& mm11 = m11.getValue();
  const MeqMatrix& mm12 = m12.getValue();
  const MeqMatrix& mm21 = m21.getValue();
  const MeqMatrix& mm22 = m22.getValue();
  const MeqMatrix& mr11 = r11.getValue();
  const MeqMatrix& mr12 = r12.getValue();
  const MeqMatrix& mr21 = r21.getValue();
  const MeqMatrix& mr22 = r22.getValue();
// timer.start();
#if defined __SSE2__
  if (ml11.rep()->type == MeqMatrixRep::ComplexScalar &&
      mm11.rep()->type == MeqMatrixRep::ComplexArray &&
      mr11.rep()->type == MeqMatrixRep::ComplexScalar)
    getResultSSE2(result11, result12, result21, result22,
		  request,
		  l11, l12, l21, l22,
		  m11, m12, m21, m22,
		  r11, r12, r21, r22,
		  ml11, ml12, ml21, ml22,
		  mm11, mm12, mm21, mm22,
		  mr11, mr12, mr21, mr22);
  else {
#endif
    MeqMatrix t11(ml11*mm11 + ml12*mm21);
    MeqMatrix t12(ml11*mm12 + ml12*mm22);
    MeqMatrix t21(ml21*mm11 + ml22*mm21);
    MeqMatrix t22(ml21*mm12 + ml22*mm22);
    result11.setValue (t11*conj(mr11) + t12*conj(mr12));
    result12.setValue (t11*conj(mr21) + t12*conj(mr22));
    result21.setValue (t21*conj(mr11) + t22*conj(mr12));
    result22.setValue (t21*conj(mr21) + t22*conj(mr22));

    // Determine which values are perturbed and determine the perturbation.
    double perturbation;
    for (int spinx=0; spinx<request.nspid(); spinx++) {
      bool eval11 = false;
      bool eval12 = false;
      bool eval21 = false;
      bool eval22 = false;
      if (m11.isDefined(spinx)) {
	eval11 = true;
	perturbation = m11.getPerturbation(spinx);
      } else if (m12.isDefined(spinx)) {
	eval11 = true;
	perturbation = m12.getPerturbation(spinx);
      } else if (m21.isDefined(spinx)) {
	eval11 = true;
	perturbation = m21.getPerturbation(spinx);
      } else if (m22.isDefined(spinx)) {
	eval11 = true;
	perturbation = m22.getPerturbation(spinx);
      }
      if (eval11) {
	eval12 = eval21 = eval22 = true;
      } else {
	if (l11.isDefined(spinx)) {
	  perturbation = l11.getPerturbation(spinx);
	  eval11 = true;
	  eval12 = true;
	} else if (l12.isDefined(spinx)) {
	  perturbation = l12.getPerturbation(spinx);
	  eval11 = true;
	  eval12 = true;
	}
	if (l21.isDefined(spinx)) {
	  perturbation = l21.getPerturbation(spinx);
	  eval21 = true;
	  eval22 = true;
	} else if (l22.isDefined(spinx)) {
	  perturbation = l22.getPerturbation(spinx);
	  eval21 = true;
	  eval22 = true;
	}
	if (r11.isDefined(spinx)) {
	  perturbation = r11.getPerturbation(spinx);
	  eval11 = true;
	  eval21 = true;
	} else if (r12.isDefined(spinx)) {
	  perturbation = r12.getPerturbation(spinx);
	  eval11 = true;
	  eval21 = true;
	}
	if (r21.isDefined(spinx)) {
	  perturbation = r21.getPerturbation(spinx);
	  eval12 = true;
	  eval22 = true;
	} else if (r22.isDefined(spinx)) {
	  perturbation = r22.getPerturbation(spinx);
	  eval12 = true;
	  eval22 = true;
	}
      }
      if (eval11 || eval12 || eval21 || eval22) {
	const MeqMatrix& ml11 = l11.getPerturbedValue(spinx);
	const MeqMatrix& ml12 = l12.getPerturbedValue(spinx);
	const MeqMatrix& ml21 = l21.getPerturbedValue(spinx);
	const MeqMatrix& ml22 = l22.getPerturbedValue(spinx);
	const MeqMatrix& mm11 = m11.getPerturbedValue(spinx);
	const MeqMatrix& mm12 = m12.getPerturbedValue(spinx);
	const MeqMatrix& mm21 = m21.getPerturbedValue(spinx);
	const MeqMatrix& mm22 = m22.getPerturbedValue(spinx);
	const MeqMatrix& mr11 = r11.getPerturbedValue(spinx);
	const MeqMatrix& mr12 = r12.getPerturbedValue(spinx);
	const MeqMatrix& mr21 = r21.getPerturbedValue(spinx);
	const MeqMatrix& mr22 = r22.getPerturbedValue(spinx);

	if (eval11 || eval12) {
	  MeqMatrix t11(ml11*mm11 + ml12*mm21);
	  MeqMatrix t12(ml11*mm12 + ml12*mm22);
	  if (eval11) { 
	    result11.setPerturbation (spinx, perturbation);
	    result11.setPerturbedValue (spinx, t11*conj(mr11) + t12*conj(mr12));
	  }
	  if (eval12) {
	    result12.setPerturbation (spinx, perturbation);
	    result12.setPerturbedValue (spinx, t11*conj(mr21) + t12*conj(mr22));
	  }
	}
	if (eval21 || eval22) {
	  MeqMatrix t21(ml21*mm11 + ml22*mm21);
	  MeqMatrix t22(ml21*mm12 + ml22*mm22);
	  if (eval21) {
	    result21.setPerturbation (spinx, perturbation);
	    result21.setPerturbedValue (spinx, t21*conj(mr11) + t22*conj(mr12));
	  }
	  if (eval22) {
	    result22.setPerturbation (spinx, perturbation);
	    result22.setPerturbedValue (spinx, t21*conj(mr21) + t22*conj(mr22));
	  }
	}
      }
    }
#if defined __SSE2__
  }
#endif

// timer.stop();
  return result;
}

}
