//# JonesCMul3.cc: Calculate A * B * C^H (the conjugate transpose of C).
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

#include <BBSKernel/Expr/JonesCMul3.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/JonesResult.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/PValueIterator.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>

#if defined __SSE2__
#include <emmintrin.h>
#include <xmmintrin.h>
#endif

using namespace casa;

namespace LOFAR
{
namespace BBS
{

enum PValues
{
    PV_LEFT11, PV_LEFT12, PV_LEFT21, PV_LEFT22,
    PV_MID11, PV_MID12, PV_MID21, PV_MID22,
    PV_RIGHT11, PV_RIGHT12, PV_RIGHT21, PV_RIGHT22,
    N_PValues
};

JonesCMul3::JonesCMul3(const JonesExpr &left, const JonesExpr &mid,
    const JonesExpr &right)
    :   itsLeft(left),
        itsMid(mid),
        itsRight(right)
{
    addChild (itsLeft);
    addChild (itsMid);
    addChild (itsRight);
}

JonesCMul3::~JonesCMul3()
{
}

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
static void getResultSSE2(
    Result &result11, Result &result12,
    Result &result21, Result &result22,
    const Result &l11, const Result &l12,
    const Result &l21, const Result &l22,
    const Result &m11, const Result &m12,
    const Result &m21, const Result &m22,
    const Result &r11, const Result &r12,
    const Result &r21, const Result &r22,
    const Matrix &ml11, const Matrix &ml12,
    const Matrix &ml21, const Matrix &ml22,
    const Matrix &mm11, const Matrix &mm12,
    const Matrix &mm21, const Matrix &mm22,
    const Matrix &mr11, const Matrix &mr12,
    const Matrix &mr21, const Matrix &mr22)
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

    Matrix v11(v, nx, ny, false);
    Matrix v12(v, nx, ny, false);
    Matrix v21(v, nx, ny, false);
    Matrix v22(v, nx, ny, false);

    __m128d *v11_r, *v11_i;
    v11.dcomplexStorage((const double *&) v11_r, (const double *&) v11_i);
    __m128d *v12_r, *v12_i;
    v12.dcomplexStorage((const double *&) v12_r, (const double *&) v12_i);
    __m128d *v21_r, *v21_i;
    v21.dcomplexStorage((const double *&) v21_r, (const double *&) v21_i);
    __m128d *v22_r, *v22_i;
    v22.dcomplexStorage((const double *&) v22_r, (const double *&) v22_i);

    // Compute main value.
    n = (mm11.rep()->nelements() + 1) / 2;

    for (int _i = 0; _i < n; _i ++)
    {
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

    result11.setValue(v11);
    result12.setValue(v12);
    result21.setValue(v21);
    result22.setValue(v22);

    // Compute perturbed values.
    const Result *pvSet[N_PValues] = {&l11, &l12, &l21, &l22, &m11, &m12, &m21,
        &m22, &r11, &r12, &r21, &r22};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {  
        bool eval11, eval12, eval21, eval22;
        eval11 = eval12 = eval21 = eval22 = false;
      
        if(pvIter.hasPValue(PV_MID11)
          || pvIter.hasPValue(PV_MID12)
          || pvIter.hasPValue(PV_MID21)
          || pvIter.hasPValue(PV_MID22))
        {
            eval11 = eval12 = eval21 = eval22 = true;
        }
        else
        {
            if(pvIter.hasPValue(PV_LEFT11) || pvIter.hasPValue(PV_LEFT12))
            {
                eval11 = eval12 = true;
            }
            
            if(pvIter.hasPValue(PV_LEFT21) || pvIter.hasPValue(PV_LEFT22))
            {
                eval21 = eval22 = true;
            }
            
            if(pvIter.hasPValue(PV_RIGHT11) || pvIter.hasPValue(PV_RIGHT12))
            {
                eval11 = eval21 = true;
            }
            
            if(pvIter.hasPValue(PV_RIGHT21) || pvIter.hasPValue(PV_RIGHT22))
            {
                eval12 = eval22 = true;
            }
        }
   
        const Matrix &ml11 = pvIter.value(PV_LEFT11);
        const Matrix &ml12 = pvIter.value(PV_LEFT12);
        const Matrix &ml21 = pvIter.value(PV_LEFT21);
        const Matrix &ml22 = pvIter.value(PV_LEFT22);
        const Matrix &mm11 = pvIter.value(PV_MID11);
        const Matrix &mm12 = pvIter.value(PV_MID12);
        const Matrix &mm21 = pvIter.value(PV_MID21);
        const Matrix &mm22 = pvIter.value(PV_MID22);
        const Matrix &mr11 = pvIter.value(PV_RIGHT11);
        const Matrix &mr12 = pvIter.value(PV_RIGHT12);
        const Matrix &mr21 = pvIter.value(PV_RIGHT21);
        const Matrix &mr22 = pvIter.value(PV_RIGHT22);

        if(eval11 || eval12)
        {
            v = ml11.getDComplex();
            __m128d l11_r = _mm_set1_pd(real(v)), l11_i = _mm_set1_pd(imag(v));
            v = ml12.getDComplex();
            __m128d l12_r = _mm_set1_pd(real(v)), l12_i = _mm_set1_pd(imag(v));

            __m128d *m11_r, *m11_i;
            mm11.dcomplexStorage((const double *&) m11_r,
                (const double *&) m11_i);
            __m128d *m12_r, *m12_i;
            mm12.dcomplexStorage((const double *&) m12_r,
                (const double *&) m12_i);
            __m128d *m21_r, *m21_i;
            mm21.dcomplexStorage((const double *&) m21_r,
                (const double *&) m21_i);
            __m128d *m22_r, *m22_i;
            mm22.dcomplexStorage((const double *&) m22_r,
                (const double *&) m22_i);

            if(eval11 && eval12)
            {
                v = mr11.getDComplex();
                __m128d r11_r = _mm_set1_pd(real(v));
                __m128d r11_i = _mm_set1_pd(imag(v));
                v = mr12.getDComplex();
                __m128d r12_r = _mm_set1_pd(real(v));
                __m128d r12_i = _mm_set1_pd(imag(v));
                v = mr21.getDComplex();
                __m128d r21_r = _mm_set1_pd(real(v));
                __m128d r21_i = _mm_set1_pd(imag(v));
                v = mr22.getDComplex();
                __m128d r22_r = _mm_set1_pd(real(v));
                __m128d r22_i = _mm_set1_pd(imag(v));

                Matrix &v11 = result11.getPerturbedValueRW(pvIter.key());
                Matrix &v12 = result12.getPerturbedValueRW(pvIter.key());
                v11.setDCMat(nx, ny);
                v12.setDCMat(nx, ny);

                __m128d *v11_r, *v11_i;
                v11.dcomplexStorage((const double *&) v11_r,
                    (const double *&) v11_i);
                __m128d *v12_r, *v12_i;
                v12.dcomplexStorage((const double *&) v12_r,
                    (const double *&) v12_i);

                for(int _i = 0; _i < n; _i ++)
                {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m11_r[_i]),_mm_mul_pd(l11_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m21_r[_i]),_mm_mul_pd(l12_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m11_i[_i]),_mm_mul_pd(l11_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m21_i[_i]),_mm_mul_pd(l12_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m12_r[_i]),_mm_mul_pd(l11_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m22_r[_i]),_mm_mul_pd(l12_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m12_i[_i]),_mm_mul_pd(l11_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m22_i[_i]),_mm_mul_pd(l12_i,m22_r[_i])));
v11_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v11_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
v12_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v12_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
                }
            }
            else if(eval11)
            {
                v = mr11.getDComplex();
                __m128d r11_r = _mm_set1_pd(real(v));
                __m128d r11_i = _mm_set1_pd(imag(v));
                v = mr12.getDComplex();
                __m128d r12_r = _mm_set1_pd(real(v));
                __m128d r12_i = _mm_set1_pd(imag(v));

                Matrix &v11 = result11.getPerturbedValueRW(pvIter.key());
                v11.setDCMat(nx, ny);

                __m128d *v11_r, *v11_i;
                v11.dcomplexStorage((const double *&) v11_r,
                    (const double *&) v11_i);

                for(int _i = 0; _i < n; _i ++)
                {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m11_r[_i]),_mm_mul_pd(l11_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m21_r[_i]),_mm_mul_pd(l12_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m11_i[_i]),_mm_mul_pd(l11_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m21_i[_i]),_mm_mul_pd(l12_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m12_r[_i]),_mm_mul_pd(l11_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m22_r[_i]),_mm_mul_pd(l12_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m12_i[_i]),_mm_mul_pd(l11_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m22_i[_i]),_mm_mul_pd(l12_i,m22_r[_i])));
v11_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v11_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
                }
            }
            else
            {
                v = mr21.getDComplex();
                __m128d r21_r = _mm_set1_pd(real(v));
                __m128d r21_i = _mm_set1_pd(imag(v));
                v = mr22.getDComplex();
                __m128d r22_r = _mm_set1_pd(real(v));
                __m128d r22_i = _mm_set1_pd(imag(v));

                Matrix &v12 = result12.getPerturbedValueRW(pvIter.key());
                v12.setDCMat(nx, ny);

                __m128d *v12_r, *v12_i;
                v12.dcomplexStorage((const double *&) v12_r, (const double *&) v12_i);

                for(int _i = 0; _i < n; _i ++)
                {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m11_r[_i]),_mm_mul_pd(l11_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m21_r[_i]),_mm_mul_pd(l12_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m11_i[_i]),_mm_mul_pd(l11_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m21_i[_i]),_mm_mul_pd(l12_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l11_r,m12_r[_i]),_mm_mul_pd(l11_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l12_r,m22_r[_i]),_mm_mul_pd(l12_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l11_r,m12_i[_i]),_mm_mul_pd(l11_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l12_r,m22_i[_i]),_mm_mul_pd(l12_i,m22_r[_i])));
v12_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v12_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
                }
            }
        }

        if(eval21 || eval22)
        {
            v = ml21.getDComplex();
            __m128d l21_r = _mm_set1_pd(real(v)), l21_i = _mm_set1_pd(imag(v));
            v = ml22.getDComplex();
            __m128d l22_r = _mm_set1_pd(real(v)), l22_i = _mm_set1_pd(imag(v));

            __m128d *m11_r, *m11_i;
            mm11.dcomplexStorage((const double *&) m11_r,
                (const double *&) m11_i);
            __m128d *m12_r, *m12_i;
            mm12.dcomplexStorage((const double *&) m12_r,
                (const double *&) m12_i);
            __m128d *m21_r, *m21_i;
            mm21.dcomplexStorage((const double *&) m21_r,
                (const double *&) m21_i);
            __m128d *m22_r, *m22_i;
            mm22.dcomplexStorage((const double *&) m22_r,
                (const double *&) m22_i);

            if(eval21 && eval22)
            {
                v = mr11.getDComplex();
                __m128d r11_r = _mm_set1_pd(real(v));
                __m128d r11_i = _mm_set1_pd(imag(v));
                v = mr12.getDComplex();
                __m128d r12_r = _mm_set1_pd(real(v));
                __m128d r12_i = _mm_set1_pd(imag(v));
                v = mr21.getDComplex();
                __m128d r21_r = _mm_set1_pd(real(v));
                __m128d r21_i = _mm_set1_pd(imag(v));
                v = mr22.getDComplex();
                __m128d r22_r = _mm_set1_pd(real(v));
                __m128d r22_i = _mm_set1_pd(imag(v));

                Matrix &v21 = result21.getPerturbedValueRW(pvIter.key());
                Matrix &v22 = result22.getPerturbedValueRW(pvIter.key());
                v21.setDCMat(nx, ny);
                v22.setDCMat(nx, ny);

                __m128d *v21_r, *v21_i;
                v21.dcomplexStorage((const double *&) v21_r,
                    (const double *&) v21_i);
                __m128d *v22_r, *v22_i;
                v22.dcomplexStorage((const double *&) v22_r,
                    (const double *&) v22_i);

                for(int _i = 0; _i < n; _i ++)
                {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m11_r[_i]),_mm_mul_pd(l21_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m21_r[_i]),_mm_mul_pd(l22_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m11_i[_i]),_mm_mul_pd(l21_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m21_i[_i]),_mm_mul_pd(l22_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m12_r[_i]),_mm_mul_pd(l21_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m22_r[_i]),_mm_mul_pd(l22_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m12_i[_i]),_mm_mul_pd(l21_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m22_i[_i]),_mm_mul_pd(l22_i,m22_r[_i])));
v21_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v21_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
v22_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v22_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
                }
            }
            else if(eval21)
            {
                v = mr11.getDComplex();
                __m128d r11_r = _mm_set1_pd(real(v));
                __m128d r11_i = _mm_set1_pd(imag(v));
                v = mr12.getDComplex();
                __m128d r12_r = _mm_set1_pd(real(v));
                __m128d r12_i = _mm_set1_pd(imag(v));

                Matrix &v21 = result21.getPerturbedValueRW(pvIter.key());
                v21.setDCMat(nx, ny);

                __m128d *v21_r, *v21_i;
                v21.dcomplexStorage((const double *&) v21_r,
                    (const double *&) v21_i);

                for(int _i = 0; _i < n; _i ++)
                {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m11_r[_i]),_mm_mul_pd(l21_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m21_r[_i]),_mm_mul_pd(l22_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m11_i[_i]),_mm_mul_pd(l21_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m21_i[_i]),_mm_mul_pd(l22_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m12_r[_i]),_mm_mul_pd(l21_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m22_r[_i]),_mm_mul_pd(l22_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m12_i[_i]),_mm_mul_pd(l21_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m22_i[_i]),_mm_mul_pd(l22_i,m22_r[_i])));
v21_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r11_r),_mm_mul_pd(t11_i,r11_i)),_mm_add_pd(_mm_mul_pd(t12_r,r12_r),_mm_mul_pd(t12_i,r12_i)));
v21_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r11_r),_mm_mul_pd(t11_r,r11_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r12_r),_mm_mul_pd(t12_r,r12_i)));
                }
            }
            else
            {
                v = mr21.getDComplex();
                __m128d r21_r = _mm_set1_pd(real(v));
                __m128d r21_i = _mm_set1_pd(imag(v));
                v = mr22.getDComplex();
                __m128d r22_r = _mm_set1_pd(real(v));
                __m128d r22_i = _mm_set1_pd(imag(v));

                Matrix &v22 = result22.getPerturbedValueRW(pvIter.key());
                v22.setDCMat(nx, ny);

                __m128d *v22_r, *v22_i;
                v22.dcomplexStorage((const double *&) v22_r,
                    (const double *&) v22_i);

                for (int _i = 0; _i < n; _i ++)
                {
t11_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m11_r[_i]),_mm_mul_pd(l21_i,m11_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m21_r[_i]),_mm_mul_pd(l22_i,m21_i[_i])));
t11_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m11_i[_i]),_mm_mul_pd(l21_i,m11_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m21_i[_i]),_mm_mul_pd(l22_i,m21_r[_i])));
t12_r=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(l21_r,m12_r[_i]),_mm_mul_pd(l21_i,m12_i[_i])),_mm_sub_pd(_mm_mul_pd(l22_r,m22_r[_i]),_mm_mul_pd(l22_i,m22_i[_i])));
t12_i=_mm_add_pd(_mm_add_pd(_mm_mul_pd(l21_r,m12_i[_i]),_mm_mul_pd(l21_i,m12_r[_i])),_mm_add_pd(_mm_mul_pd(l22_r,m22_i[_i]),_mm_mul_pd(l22_i,m22_r[_i])));
v22_r[_i]=_mm_add_pd(_mm_add_pd(_mm_mul_pd(t11_r,r21_r),_mm_mul_pd(t11_i,r21_i)),_mm_add_pd(_mm_mul_pd(t12_r,r22_r),_mm_mul_pd(t12_i,r22_i)));
v22_i[_i]=_mm_add_pd(_mm_sub_pd(_mm_mul_pd(t11_i,r21_r),_mm_mul_pd(t11_r,r21_i)),_mm_sub_pd(_mm_mul_pd(t12_i,r22_r),_mm_mul_pd(t12_r,r22_i)));
                }
            }
        }

        pvIter.next();
    } // while(!atEnd)
}
#endif

JonesResult JonesCMul3::getJResult(const Request& request)
{
    // Create the result object.
    JonesResult result;
    result.init();

    Result &result11 = result.result11();
    Result &result12 = result.result12();
    Result &result21 = result.result21();
    Result &result22 = result.result22();

    // Evaluate the children.
    JonesResult tmpLeft, tmpMid, tmpRight;
    const JonesResult &left = itsLeft.getResultSynced(request, tmpLeft);
    const JonesResult &mid = itsMid.getResultSynced(request, tmpMid);
    const JonesResult &right = itsRight.getResultSynced(request, tmpRight);
    
    const Result &l11 = left.getResult11();
    const Result &l12 = left.getResult12();
    const Result &l21 = left.getResult21();
    const Result &l22 = left.getResult22();
    const Result &m11 = mid.getResult11();
    const Result &m12 = mid.getResult12();
    const Result &m21 = mid.getResult21();
    const Result &m22 = mid.getResult22();
    const Result &r11 = right.getResult11();
    const Result &r12 = right.getResult12();
    const Result &r21 = right.getResult21();
    const Result &r22 = right.getResult22();
    
    const Matrix &ml11 = l11.getValue();
    const Matrix &ml12 = l12.getValue();
    const Matrix &ml21 = l21.getValue();
    const Matrix &ml22 = l22.getValue();
    const Matrix &mm11 = m11.getValue();
    const Matrix &mm12 = m12.getValue();
    const Matrix &mm21 = m21.getValue();
    const Matrix &mm22 = m22.getValue();
    const Matrix &mr11 = r11.getValue();
    const Matrix &mr12 = r12.getValue();
    const Matrix &mr21 = r21.getValue();
    const Matrix &mr22 = r22.getValue();

#if defined __SSE2__
    if(ml11.rep()->type == MatrixRep::ComplexScalar
        && mm11.rep()->type == MatrixRep::ComplexArray
        && mr11.rep()->type == MatrixRep::ComplexScalar)
    {    
        getResultSSE2(result11, result12, result21, result22,
            l11, l12, l21, l22,
            m11, m12, m21, m22,
            r11, r12, r21, r22,
            ml11, ml12, ml21, ml22,
            mm11, mm12, mm21, mm22,
            mr11, mr12, mr21, mr22);
    }
    else
    {
#endif
        // Compute main value.
        Matrix t11(ml11 * mm11 + ml12 * mm21);
        Matrix t12(ml11 * mm12 + ml12 * mm22);
        Matrix t21(ml21 * mm11 + ml22 * mm21);
        Matrix t22(ml21 * mm12 + ml22 * mm22);

        result11.setValue(t11 * conj(mr11) + t12 * conj(mr12));
        result12.setValue(t11 * conj(mr21) + t12 * conj(mr22));
        result21.setValue(t21 * conj(mr11) + t22 * conj(mr12));
        result22.setValue(t21 * conj(mr21) + t22 * conj(mr22));
        
        // Compute perturbed values.
        const Result *pvSet[N_PValues] = {&l11, &l12, &l21, &l22, &m11, &m12,
            &m21, &m22, &r11, &r12, &r21, &r22};
        PValueSetIterator<N_PValues> pvIter(pvSet);

        while(!pvIter.atEnd())
        {  
            bool eval11, eval12, eval21, eval22;
            eval11 = eval12 = eval21 = eval22 = false;
            
            if(pvIter.hasPValue(PV_MID11)
              || pvIter.hasPValue(PV_MID12)
              || pvIter.hasPValue(PV_MID21)
              || pvIter.hasPValue(PV_MID22))
            {
                eval11 = eval12 = eval21 = eval22 = true;
            }
            else
            {
                if(pvIter.hasPValue(PV_LEFT11) || pvIter.hasPValue(PV_LEFT12))
                {
                    eval11 = eval12 = true;
                }
                
                if(pvIter.hasPValue(PV_LEFT21) || pvIter.hasPValue(PV_LEFT22))
                {
                    eval21 = eval22 = true;
                }
                
                if(pvIter.hasPValue(PV_RIGHT11) || pvIter.hasPValue(PV_RIGHT12))
                {
                    eval11 = eval21 = true;
                }
                
                if(pvIter.hasPValue(PV_RIGHT21) || pvIter.hasPValue(PV_RIGHT22))
                {
                    eval12 = eval22 = true;
                }
            }

            const Matrix &ml11 = pvIter.value(PV_LEFT11);
            const Matrix &ml12 = pvIter.value(PV_LEFT12);
            const Matrix &ml21 = pvIter.value(PV_LEFT21);
            const Matrix &ml22 = pvIter.value(PV_LEFT22);
            const Matrix &mm11 = pvIter.value(PV_MID11);
            const Matrix &mm12 = pvIter.value(PV_MID12);
            const Matrix &mm21 = pvIter.value(PV_MID21);
            const Matrix &mm22 = pvIter.value(PV_MID22);
            const Matrix &mr11 = pvIter.value(PV_RIGHT11);
            const Matrix &mr12 = pvIter.value(PV_RIGHT12);
            const Matrix &mr21 = pvIter.value(PV_RIGHT21);
            const Matrix &mr22 = pvIter.value(PV_RIGHT22);

            if(eval11 || eval12)
            {
                Matrix t11(ml11 * mm11 + ml12 * mm21);
                Matrix t12(ml11 * mm12 + ml12 * mm22);
              
                if(eval11)
                {
                    result11.setPerturbedValue(pvIter.key(), t11 * conj(mr11)
                        + t12 * conj(mr12));
                }
                
                if(eval12)
                {
                    result12.setPerturbedValue(pvIter.key(), t11 * conj(mr21)
                        + t12 * conj(mr22));
                }
            }

            if(eval21 || eval22)
            {
                Matrix t21(ml21 * mm11 + ml22 * mm21);
                Matrix t22(ml21 * mm12 + ml22 * mm22);

                if(eval21)
                {
                    result21.setPerturbedValue(pvIter.key(), t21 * conj(mr11)
                        + t22 * conj(mr12));
                }

                if(eval22)
                {
                    result22.setPerturbedValue(pvIter.key(), t21 * conj(mr21)
                        + t22 * conj(mr22));
                }
            }
          
            pvIter.next();
        } // while(!atEnd)
#if defined __SSE2__
    }
#endif

    return result;
}

#ifdef EXPR_GRAPH
std::string JonesCMul3::getLabel()
{
    return std::string("JonesCMul3\\nJones expression A * B * (C*)");
}
#endif

} // namespace BBS
} // namespace LOFAR
