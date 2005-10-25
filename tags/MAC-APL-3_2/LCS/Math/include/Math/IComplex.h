//# IComplex.h: Complex class for 8, 16, and 32 bit integers
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

#ifndef MATH_ICOMPLEX_H
#define MATH_ICOMPLEX_H

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  //# Forward Declarations
  class IComplex8;
  class IComplex16;
  class IComplex32;


  class IComplex8 {
  public:
    //# Constructors
    // From one or two ints (note for gnu use)
    IComplex8();
    IComplex8 (char re);
    IComplex8 (char re, char im);

    char re() const;
    char im() const;

    bool operator== (const IComplex8& that) const;
    bool operator== (const IComplex16& that) const;
    bool operator== (const IComplex32& that) const;
    bool operator!= (const IComplex8& that) const;
    bool operator!= (const IComplex16& that) const;
    bool operator!= (const IComplex32& that) const;

    void operator+= (const IComplex8&);
    void operator-= (const IComplex8&);
    void operator*= (const IComplex8&);
    void operator/= (const IComplex8&);
    void mulconj    (const IComplex8&);

  private:
    char itsre;
    char itsim;
  };


  class IComplex16 {
  public:
    //# Constructors
    // From one or two ints (note for gnu use)
    IComplex16();
    IComplex16 (int16 re);
    IComplex16 (int16 re, int16 im);

    int16 re() const;
    int16 im() const;

    bool operator== (const IComplex8& that) const;
    bool operator== (const IComplex16& that) const;
    bool operator== (const IComplex32& that) const;
    bool operator!= (const IComplex8& that) const;
    bool operator!= (const IComplex16& that) const;
    bool operator!= (const IComplex32& that) const;

    void operator+= (const IComplex8&);
    void operator+= (const IComplex16&);
    void operator-= (const IComplex8&);
    void operator-= (const IComplex16&);
    void operator*= (const IComplex8&);
    void operator*= (const IComplex16&);
    void operator/= (const IComplex8&);
    void operator/= (const IComplex16&);
    void mulconj    (const IComplex8&);
    void mulconj    (const IComplex16&);

  private:
    int16 itsre;
    int16 itsim;
  };


  class IComplex32 {
  public:
    //# Constructors
    // From one or two ints (note for gnu use)
    IComplex32();
    IComplex32 (int32 re);
    IComplex32 (int32 re, int32 im);

    int32 re() const;
    int32 im() const;

    bool operator== (const IComplex8& that) const;
    bool operator== (const IComplex16& that) const;
    bool operator== (const IComplex32& that) const;
    bool operator!= (const IComplex8& that) const;
    bool operator!= (const IComplex16& that) const;
    bool operator!= (const IComplex32& that) const;

    void operator+= (const IComplex8&);
    void operator+= (const IComplex16&);
    void operator+= (const IComplex32&);
    void operator-= (const IComplex8&);
    void operator-= (const IComplex16&);
    void operator-= (const IComplex32&);
    void operator*= (const IComplex8&);
    void operator*= (const IComplex16&);
    void operator*= (const IComplex32&);
    void operator/= (const IComplex8&);
    void operator/= (const IComplex16&);
    void operator/= (const IComplex32&);
    void mulconj    (const IComplex8&);
    void mulconj    (const IComplex16&);
    void mulconj    (const IComplex32&);

  private:
    int32 itsre;
    int32 itsim;
  };


  // Show on ostream.
  ostream& operator<< (ostream& os, const IComplex8&);
  ostream& operator<< (ostream& os, const IComplex16&);
  ostream& operator<< (ostream& os, const IComplex32&);

  // Take conjugate.
  IComplex8  conj (const IComplex8& x);
  IComplex16 conj (const IComplex16& x);
  IComplex32 conj (const IComplex32& x);

  // Take norm (re^2+im^2).
  int32 norm (const IComplex8& x);
  int32 norm (const IComplex16& x);
  int32 norm (const IComplex32& x);

  // left+right
  IComplex8  operator+ (const IComplex8&  left, const IComplex8&  right);
  IComplex16 operator+ (const IComplex16& left, const IComplex16& right);
  IComplex16 operator+ (const IComplex16& left, const IComplex8&  right);
  IComplex16 operator+ (const IComplex8&  left, const IComplex16& right);
  IComplex32 operator+ (const IComplex32& left, const IComplex32& right);
  IComplex32 operator+ (const IComplex32& left, const IComplex16& right);
  IComplex32 operator+ (const IComplex32& left, const IComplex8&  right);
  IComplex32 operator+ (const IComplex16& left, const IComplex32& right);
  IComplex32 operator+ (const IComplex8&  left, const IComplex32& right);
  IComplex8  add8  (const IComplex8&  left, const IComplex8&  right);
  IComplex16 add16 (const IComplex8&  left, const IComplex8&  right);
  IComplex16 add16 (const IComplex8&  left, const IComplex16& right);
  IComplex16 add16 (const IComplex16& left, const IComplex8&  right);
  IComplex16 add16 (const IComplex16& left, const IComplex16& right);
  IComplex32 add32 (const IComplex8&  left, const IComplex8&  right);
  IComplex32 add32 (const IComplex8&  left, const IComplex16& right);
  IComplex32 add32 (const IComplex16& left, const IComplex8&  right);
  IComplex32 add32 (const IComplex16& left, const IComplex16& right);
  IComplex32 add32 (const IComplex8&  left, const IComplex32& right);
  IComplex32 add32 (const IComplex32& left, const IComplex8&  right);
  IComplex32 add32 (const IComplex16& left, const IComplex32& right);
  IComplex32 add32 (const IComplex32& left, const IComplex16& right);
  IComplex32 add32 (const IComplex32& left, const IComplex32& right);

  // left-right
  IComplex8  operator- (const IComplex8&  left, const IComplex8&  right);
  IComplex16 operator- (const IComplex16& left, const IComplex16& right);
  IComplex16 operator- (const IComplex16& left, const IComplex8&  right);
  IComplex16 operator- (const IComplex8&  left, const IComplex16& right);
  IComplex32 operator- (const IComplex32& left, const IComplex32& right);
  IComplex32 operator- (const IComplex32& left, const IComplex16& right);
  IComplex32 operator- (const IComplex32& left, const IComplex8&  right);
  IComplex32 operator- (const IComplex16& left, const IComplex32& right);
  IComplex32 operator- (const IComplex8&  left, const IComplex32& right);
  IComplex8  sub8  (const IComplex8&  left, const IComplex8&  right);
  IComplex16 sub16 (const IComplex8&  left, const IComplex8&  right);
  IComplex16 sub16 (const IComplex8&  left, const IComplex16& right);
  IComplex16 sub16 (const IComplex16& left, const IComplex8&  right);
  IComplex16 sub16 (const IComplex16& left, const IComplex16& right);
  IComplex32 sub32 (const IComplex8&  left, const IComplex8&  right);
  IComplex32 sub32 (const IComplex8&  left, const IComplex16& right);
  IComplex32 sub32 (const IComplex16& left, const IComplex8&  right);
  IComplex32 sub32 (const IComplex16& left, const IComplex16& right);
  IComplex32 sub32 (const IComplex8&  left, const IComplex32& right);
  IComplex32 sub32 (const IComplex32& left, const IComplex8&  right);
  IComplex32 sub32 (const IComplex16& left, const IComplex32& right);
  IComplex32 sub32 (const IComplex32& left, const IComplex16& right);
  IComplex32 sub32 (const IComplex32& left, const IComplex32& right);

  // left*right
  IComplex8  operator* (const IComplex8&  left, const IComplex8&  right);
  IComplex16 operator* (const IComplex16& left, const IComplex16& right);
  IComplex16 operator* (const IComplex16& left, const IComplex8&  right);
  IComplex16 operator* (const IComplex8&  left, const IComplex16& right);
  IComplex32 operator* (const IComplex32& left, const IComplex32& right);
  IComplex32 operator* (const IComplex32& left, const IComplex16& right);
  IComplex32 operator* (const IComplex32& left, const IComplex8&  right);
  IComplex32 operator* (const IComplex16& left, const IComplex32& right);
  IComplex32 operator* (const IComplex8&  left, const IComplex32& right);
  IComplex8  mul8 ( const IComplex8&  left, const IComplex8&  right);
  IComplex16 mul16 (const IComplex8&  left, const IComplex8&  right);
  IComplex16 mul16 (const IComplex8&  left, const IComplex16& right);
  IComplex16 mul16 (const IComplex16& left, const IComplex8&  right);
  IComplex16 mul16 (const IComplex16& left, const IComplex16& right);
  IComplex32 mul32 (const IComplex8&  left, const IComplex8&  right);
  IComplex32 mul32 (const IComplex8&  left, const IComplex16& right);
  IComplex32 mul32 (const IComplex16& left, const IComplex8&  right);
  IComplex32 mul32 (const IComplex16& left, const IComplex16& right);
  IComplex32 mul32 (const IComplex8&  left, const IComplex32& right);
  IComplex32 mul32 (const IComplex32& left, const IComplex8&  right);
  IComplex32 mul32 (const IComplex16& left, const IComplex32& right);
  IComplex32 mul32 (const IComplex32& left, const IComplex16& right);
  IComplex32 mul32 (const IComplex32& left, const IComplex32& right);

  // left/right
  IComplex8  operator/ (const IComplex8&  left, const IComplex8&  right);
  IComplex16 operator/ (const IComplex16& left, const IComplex16& right);
  IComplex16 operator/ (const IComplex16& left, const IComplex8&  right);
  IComplex16 operator/ (const IComplex8&  left, const IComplex16& right);
  IComplex32 operator/ (const IComplex32& left, const IComplex32& right);
  IComplex32 operator/ (const IComplex32& left, const IComplex16& right);
  IComplex32 operator/ (const IComplex32& left, const IComplex8&  right);
  IComplex32 operator/ (const IComplex16& left, const IComplex32& right);
  IComplex32 operator/ (const IComplex8&  left, const IComplex32& right);
  IComplex8  div8  (const IComplex8&  left, const IComplex8&  right);
  IComplex16 div16 (const IComplex8&  left, const IComplex8&  right);
  IComplex16 div16 (const IComplex8&  left, const IComplex16& right);
  IComplex16 div16 (const IComplex16& left, const IComplex8&  right);
  IComplex16 div16 (const IComplex16& left, const IComplex16& right);
  IComplex32 div32 (const IComplex8&  left, const IComplex8&  right);
  IComplex32 div32 (const IComplex8&  left, const IComplex16& right);
  IComplex32 div32 (const IComplex16& left, const IComplex8&  right);
  IComplex32 div32 (const IComplex16& left, const IComplex16& right);
  IComplex32 div32 (const IComplex8&  left, const IComplex32& right);
  IComplex32 div32 (const IComplex32& left, const IComplex8&  right);
  IComplex32 div32 (const IComplex16& left, const IComplex32& right);
  IComplex32 div32 (const IComplex32& left, const IComplex16& right);
  IComplex32 div32 (const IComplex32& left, const IComplex32& right);

  // left*conj(right)
  IComplex8  mulconj (const IComplex8&  left, const IComplex8&  right);
  IComplex16 mulconj (const IComplex16& left, const IComplex16& right);
  IComplex16 mulconj (const IComplex16& left, const IComplex8&  right);
  IComplex16 mulconj (const IComplex8&  left, const IComplex16& right);
  IComplex32 mulconj (const IComplex32& left, const IComplex32& right);
  IComplex32 mulconj (const IComplex32& left, const IComplex16& right);
  IComplex32 mulconj (const IComplex32& left, const IComplex8&  right);
  IComplex32 mulconj (const IComplex16& left, const IComplex32& right);
  IComplex32 mulconj (const IComplex8&  left, const IComplex32& right);
  IComplex8  mulconj8 ( const IComplex8&  left, const IComplex8&  right);
  IComplex16 mulconj16 (const IComplex8&  left, const IComplex8&  right);
  IComplex16 mulconj16 (const IComplex8&  left, const IComplex16& right);
  IComplex16 mulconj16 (const IComplex16& left, const IComplex8&  right);
  IComplex16 mulconj16 (const IComplex16& left, const IComplex16& right);
  IComplex32 mulconj32 (const IComplex8&  left, const IComplex8&  right);
  IComplex32 mulconj32 (const IComplex8&  left, const IComplex16& right);
  IComplex32 mulconj32 (const IComplex16& left, const IComplex8&  right);
  IComplex32 mulconj32 (const IComplex16& left, const IComplex16& right);
  IComplex32 mulconj32 (const IComplex8&  left, const IComplex32& right);
  IComplex32 mulconj32 (const IComplex32& left, const IComplex8&  right);
  IComplex32 mulconj32 (const IComplex16& left, const IComplex32& right);
  IComplex32 mulconj32 (const IComplex32& left, const IComplex16& right);
  IComplex32 mulconj32 (const IComplex32& left, const IComplex32& right);



  inline IComplex8::IComplex8()
    : itsre(0),
      itsim(0)
  {}
  inline IComplex8::IComplex8 (char re)
    : itsre(re),
      itsim(0)
  {}
  inline IComplex8::IComplex8 (char re, char im)
    : itsre(re),
      itsim(im)
  {}

  inline char IComplex8::re() const
  { return itsre; }
  inline char IComplex8::im() const
  { return itsim; }

  inline bool IComplex8::operator== (const IComplex8& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex8::operator== (const IComplex16& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex8::operator== (const IComplex32& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex8::operator!= (const IComplex8& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }
  inline bool IComplex8::operator!= (const IComplex16& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }
  inline bool IComplex8::operator!= (const IComplex32& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }


  inline IComplex16::IComplex16()
    : itsre(0),
      itsim(0)
  {}
  inline IComplex16::IComplex16 (int16 re)
    : itsre(re),
      itsim(0)
  {}
  inline IComplex16::IComplex16 (int16 re, int16 im)
    : itsre(re),
      itsim(im)
  {}

  inline int16 IComplex16::re() const
  { return itsre; }
  inline int16 IComplex16::im() const
  { return itsim; }

  inline bool IComplex16::operator== (const IComplex8& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex16::operator== (const IComplex16& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex16::operator== (const IComplex32& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex16::operator!= (const IComplex8& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }
  inline bool IComplex16::operator!= (const IComplex16& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }
  inline bool IComplex16::operator!= (const IComplex32& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }


  inline IComplex32::IComplex32()
    : itsre(0),
      itsim(0)
  {}
  inline IComplex32::IComplex32 (int32 re)
    : itsre(re),
      itsim(0)
  {}
  inline IComplex32::IComplex32 (int32 re, int32 im)
    : itsre(re),
      itsim(im)
  {}

  inline int32 IComplex32::re() const
  { return itsre; }
  inline int32 IComplex32::im() const
  { return itsim; }

  inline bool IComplex32::operator== (const IComplex8& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex32::operator== (const IComplex16& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex32::operator== (const IComplex32& that) const
  { return itsre == that.re()  &&  itsim == that.im(); }
  inline bool IComplex32::operator!= (const IComplex8& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }
  inline bool IComplex32::operator!= (const IComplex16& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }
  inline bool IComplex32::operator!= (const IComplex32& that) const
  { return itsre != that.re()  ||  itsim != that.im(); }



  inline IComplex8 conj (const IComplex8& x)
  { return IComplex8 (x.re(), -x.im()); }
  inline IComplex16 conj (const IComplex16& x)
  { return IComplex16 (x.re(), -x.im()); }
  inline IComplex32 conj (const IComplex32& x)
  { return IComplex32 (x.re(), -x.im()); }

  inline int32 norm (const IComplex8& x)
  { return int32(x.re())*x.re() + int32(x.im())*x.im(); }
  inline int32 norm (const IComplex16& x)
  { return int32(x.re())*x.re() + int32(x.im())*x.im(); }
  inline int32 norm (const IComplex32& x)
  { return x.re()*x.re() + x.im()*x.im(); }

  inline void IComplex8::operator+= (const IComplex8& that)
  { itsre += that.re(); itsim += that.im(); }
  inline void IComplex16::operator+= (const IComplex8& that)
  { itsre += that.re(); itsim += that.im(); }
  inline void IComplex16::operator+= (const IComplex16& that)
  { itsre += that.re(); itsim += that.im(); }
  inline void IComplex32::operator+= (const IComplex8& that)
  { itsre += that.re(); itsim += that.im(); }
  inline void IComplex32::operator+= (const IComplex16& that)
  { itsre += that.re(); itsim += that.im(); }
  inline void IComplex32::operator+= (const IComplex32& that)
  { itsre += that.re(); itsim += that.im(); }

  inline void IComplex8::operator-= (const IComplex8& that)
  { itsre -= that.re(); itsim -= that.im(); }
  inline void IComplex16::operator-= (const IComplex8& that)
  { itsre -= that.re(); itsim -= that.im(); }
  inline void IComplex16::operator-= (const IComplex16& that)
  { itsre -= that.re(); itsim -= that.im(); }
  inline void IComplex32::operator-= (const IComplex8& that)
  { itsre -= that.re(); itsim -= that.im(); }
  inline void IComplex32::operator-= (const IComplex16& that)
  { itsre -= that.re(); itsim -= that.im(); }
  inline void IComplex32::operator-= (const IComplex32& that)
  { itsre -= that.re(); itsim -= that.im(); }

  inline void IComplex8::operator*= (const IComplex8& that)
  { char re = itsre * that.re() - itsim * that.im();
  itsim = itsre * that.im() + itsim * that.re(); itsre = re; }
  inline void IComplex16::operator*= (const IComplex8& that)
  { int16 re = itsre * that.re() - itsim * that.im();
  itsim = itsre * that.im() + itsim * that.re(); itsre = re; }
  inline void IComplex16::operator*= (const IComplex16& that)
  { int16 re = itsre * that.re() - itsim * that.im();
  itsim = itsre * that.im() + itsim * that.re(); itsre = re; }
  inline void IComplex32::operator*= (const IComplex8& that)
  { int32 re = itsre * that.re() - itsim * that.im();
  itsim = itsre * that.im() + itsim * that.re(); itsre = re; }
  inline void IComplex32::operator*= (const IComplex16& that)
  { int32 re = itsre * that.re() - itsim * that.im();
  itsim = itsre * that.im() + itsim * that.re(); itsre = re; }
  inline void IComplex32::operator*= (const IComplex32& that)
  { int32 re = itsre * that.re() - itsim * that.im();
  itsim = itsre * that.im() + itsim * that.re(); itsre = re; }

  inline void IComplex8::operator/= (const IComplex8& that)
  { int32 n = norm(that);
  int32 re = (itsre * that.re() + itsim * that.im()) / n;
  itsim = (itsim * that.re() - itsre * that.im()) / n; itsre = re; }
  inline void IComplex16::operator/= (const IComplex8& that)
  { int32 n = norm(that);
  int32 re = (itsre * that.re() + itsim * that.im()) / n;
  itsim = (itsim * that.re() - itsre * that.im()) / n; itsre = re; }
  inline void IComplex16::operator/= (const IComplex16& that)
  { int32 n = norm(that);
  int32 re = (itsre * that.re() + itsim * that.im()) / n;
  itsim = (itsim * that.re() - itsre * that.im()) / n; itsre = re; }
  inline void IComplex32::operator/= (const IComplex8& that)
  { int32 n = norm(that);
  int32 re = (itsre * that.re() + itsim * that.im()) / n;
  itsim = (itsim * that.re() - itsre * that.im()) / n; itsre = re; }
  inline void IComplex32::operator/= (const IComplex16& that)
  { int32 n = norm(that);
  int32 re = (itsre * that.re() + itsim * that.im()) / n;
  itsim = (itsim * that.re() - itsre * that.im()) / n; itsre = re; }
  inline void IComplex32::operator/= (const IComplex32& that)
  { int32 n = norm(that);
  int32 re = (itsre * that.re() + itsim * that.im()) / n;
  itsim = (itsim * that.re() - itsre * that.im()) / n; itsre = re; }

  inline void IComplex8::mulconj (const IComplex8& that)
  { char re = itsre * that.re() + itsim * that.im();
  itsim = itsim * that.re() - itsre * that.im(); itsre = re; }
  inline void IComplex16::mulconj (const IComplex8& that)
  { int16 re = itsre * that.re() + itsim * that.im();
  itsim = itsim * that.re() - itsre * that.im(); itsre = re; }
  inline void IComplex16::mulconj (const IComplex16& that)
  { int32 re = itsre * that.re() + itsim * that.im();
  itsim = itsim * that.re() - itsre * that.im(); itsre = re; }
  inline void IComplex32::mulconj (const IComplex8& that)
  { int32 re = itsre * that.re() + itsim * that.im();
  itsim = itsim * that.re() - itsre * that.im(); itsre = re; }
  inline void IComplex32::mulconj (const IComplex16& that)
  { int32 re = itsre * that.re() + itsim * that.im();
  itsim = itsim * that.re() - itsre * that.im(); itsre = re; }
  inline void IComplex32::mulconj (const IComplex32& that)
  { int32 re = itsre * that.re() + itsim * that.im();
  itsim = itsim * that.re() - itsre * that.im(); itsre = re; }


  inline IComplex8 operator+ (const IComplex8& left, const IComplex8& right)
  { return add8 (left, right); }
  inline IComplex16 operator+ (const IComplex16& left, const IComplex16& right)
  { return add16 (left, right); }
  inline IComplex16 operator+ (const IComplex16& left, const IComplex8& right)
  { return add16 (left, right); }
  inline IComplex16 operator+ (const IComplex8& left, const IComplex16& right)
  { return add16 (left, right); }
  inline IComplex32 operator+ (const IComplex32& left, const IComplex32& right)
  { return add32 (left, right); }
  inline IComplex32 operator+ (const IComplex32& left, const IComplex16& right)
  { return add32 (left, right); }
  inline IComplex32 operator+ (const IComplex32& left, const IComplex8& right)
  { return add32 (left, right); }
  inline IComplex32 operator+ (const IComplex16& left, const IComplex32& right)
  { return add32 (left, right); }
  inline IComplex32 operator+ (const IComplex8& left, const IComplex32& right)
  { return add32 (left, right); }

  inline IComplex8 operator- (const IComplex8& left, const IComplex8& right)
  { return sub8 (left, right); }
  inline IComplex16 operator- (const IComplex16& left, const IComplex16& right)
  { return sub16 (left, right); }
  inline IComplex16 operator- (const IComplex16& left, const IComplex8& right)
  { return sub16 (left, right); }
  inline IComplex16 operator- (const IComplex8& left, const IComplex16& right)
  { return sub16 (left, right); }
  inline IComplex32 operator- (const IComplex32& left, const IComplex32& right)
  { return sub32 (left, right); }
  inline IComplex32 operator- (const IComplex32& left, const IComplex16& right)
  { return sub32 (left, right); }
  inline IComplex32 operator- (const IComplex32& left, const IComplex8& right)
  { return sub32 (left, right); }
  inline IComplex32 operator- (const IComplex16& left, const IComplex32& right)
  { return sub32 (left, right); }
  inline IComplex32 operator- (const IComplex8& left, const IComplex32& right)
  { return sub32 (left, right); }

  inline IComplex8 operator* (const IComplex8& left, const IComplex8& right)
  { return mul8 (left, right); }
  inline IComplex16 operator* (const IComplex16& left, const IComplex16& right)
  { return mul16 (left, right); }
  inline IComplex16 operator* (const IComplex16& left, const IComplex8& right)
  { return mul16 (left, right); }
  inline IComplex16 operator* (const IComplex8& left, const IComplex16& right)
  { return mul16 (left, right); }
  inline IComplex32 operator* (const IComplex32& left, const IComplex32& right)
  { return mul32 (left, right); }
  inline IComplex32 operator* (const IComplex32& left, const IComplex16& right)
  { return mul32 (left, right); }
  inline IComplex32 operator* (const IComplex32& left, const IComplex8& right)
  { return mul32 (left, right); }
  inline IComplex32 operator* (const IComplex16& left, const IComplex32& right)
  { return mul32 (left, right); }
  inline IComplex32 operator* (const IComplex8& left, const IComplex32& right)
  { return mul32 (left, right); }

  inline IComplex8 operator/ (const IComplex8& left, const IComplex8& right)
  { return div8 (left, right); }
  inline IComplex16 operator/ (const IComplex16& left, const IComplex16& right)
  { return div16 (left, right); }
  inline IComplex16 operator/ (const IComplex16& left, const IComplex8& right)
  { return div16 (left, right); }
  inline IComplex16 operator/ (const IComplex8& left, const IComplex16& right)
  { return div16 (left, right); }
  inline IComplex32 operator/ (const IComplex32& left, const IComplex32& right)
  { return div32 (left, right); }
  inline IComplex32 operator/ (const IComplex32& left, const IComplex16& right)
  { return div32 (left, right); }
  inline IComplex32 operator/ (const IComplex32& left, const IComplex8& right)
  { return div32 (left, right); }
  inline IComplex32 operator/ (const IComplex16& left, const IComplex32& right)
  { return div32 (left, right); }
  inline IComplex32 operator/ (const IComplex8& left, const IComplex32& right)
  { return div32 (left, right); }

  inline IComplex8 mulconj (const IComplex8& left, const IComplex8& right)
  { return mulconj8 (left, right); }
  inline IComplex16 mulconj (const IComplex16& left, const IComplex16& right)
  { return mulconj16 (left, right); }
  inline IComplex16 mulconj (const IComplex16& left, const IComplex8& right)
  { return mulconj16 (left, right); }
  inline IComplex16 mulconj (const IComplex8& left, const IComplex16& right)
  { return mulconj16 (left, right); }
  inline IComplex32 mulconj (const IComplex32& left, const IComplex32& right)
  { return mulconj32 (left, right); }
  inline IComplex32 mulconj (const IComplex32& left, const IComplex16& right)
  { return mulconj32 (left, right); }
  inline IComplex32 mulconj (const IComplex32& left, const IComplex8& right)
  { return mulconj32 (left, right); }
  inline IComplex32 mulconj (const IComplex16& left, const IComplex32& right)
  { return mulconj32 (left, right); }
  inline IComplex32 mulconj (const IComplex8& left, const IComplex32& right)
  { return mulconj32 (left, right); }


  inline IComplex8 add8 (const IComplex8& left, const IComplex8& right)
  { return IComplex8 (left.re() + right.re(),
		      left.im() + right.im()); }
  inline IComplex16 add16 (const IComplex8& left, const IComplex8& right)
  { return IComplex16 (int16(left.re()) + right.re(),
		       int16(left.im()) + right.im()); }
  inline IComplex16 add16 (const IComplex8& left, const IComplex16& right)
  { return IComplex16 (left.re() + right.re(),
		       left.im() + right.im()); }
  inline IComplex16 add16 (const IComplex16& left, const IComplex8& right)
  { return IComplex16 (left.re() + right.re(),
		       left.im() + right.im()); }
  inline IComplex16 add16 (const IComplex16& left, const IComplex16& right)
  { return IComplex16 (left.re() + right.re(),
		       left.im() + right.im()); }
  inline IComplex32 add32 (const IComplex8& left, const IComplex8& right)
  { return IComplex32 (int32(left.re()) + right.re(),
		       int32(left.im()) + right.im()); }
  inline IComplex32 add32 (const IComplex8& left, const IComplex16& right)
  { return IComplex32 (int32(left.re()) + right.re(),
		       int32(left.im()) + right.im()); }
  inline IComplex32 add32 (const IComplex16& left, const IComplex8& right)
  { return IComplex32 (int32(left.re()) + right.re(),
		       int32(left.im()) + right.im()); }
  inline IComplex32 add32 (const IComplex16& left, const IComplex16& right)
  { return IComplex32 (int32(left.re()) + right.re(),
		       int32(left.im()) + right.im()); }
  inline IComplex32 add32 (const IComplex8& left, const IComplex32& right)
  { return IComplex32 (left.re() + right.re(),
		       left.im() + right.im()); }
  inline IComplex32 add32 (const IComplex32& left, const IComplex8& right)
  { return IComplex32 (left.re() + right.re(),
		       left.im() + right.im()); }
  inline IComplex32 add32 (const IComplex16& left, const IComplex32& right)
  { return IComplex32 (left.re() + right.re(),
		       left.im() + right.im()); }
  inline IComplex32 add32 (const IComplex32& left, const IComplex16& right)
  { return IComplex32 (left.re() + right.re(),
		       left.im() + right.im()); }
  inline IComplex32 add32 (const IComplex32& left, const IComplex32& right)
  { return IComplex32 (left.re() + right.re(),
		       left.im() + right.im()); }

  inline IComplex8 sub8 (const IComplex8& left, const IComplex8& right)
  { return IComplex8 (left.re() - right.re(),
		      left.im() - right.im()); }
  inline IComplex16 sub16 (const IComplex8& left, const IComplex8& right)
  { return IComplex16 (int16(left.re()) - right.re(),
		       int16(left.im()) - right.im()); }
  inline IComplex16 sub16 (const IComplex8& left, const IComplex16& right)
  { return IComplex16 (left.re() - right.re(),
		       left.im() - right.im()); }
  inline IComplex16 sub16 (const IComplex16& left, const IComplex8& right)
  { return IComplex16 (left.re() - right.re(),
		       left.im() - right.im()); }
  inline IComplex16 sub16 (const IComplex16& left, const IComplex16& right)
  { return IComplex16 (left.re() - right.re(),
		       left.im() - right.im()); }
  inline IComplex32 sub32 (const IComplex8& left, const IComplex8& right)
  { return IComplex32 (int32(left.re()) - right.re(),
		       int32(left.im()) - right.im()); }
  inline IComplex32 sub32 (const IComplex8& left, const IComplex16& right)
  { return IComplex32 (int32(left.re()) - right.re(),
		       int32(left.im()) - right.im()); }
  inline IComplex32 sub32 (const IComplex16& left, const IComplex8& right)
  { return IComplex32 (int32(left.re()) - right.re(),
		       int32(left.im()) - right.im()); }
  inline IComplex32 sub32 (const IComplex16& left, const IComplex16& right)
  { return IComplex32 (int32(left.re()) - right.re(),
		       int32(left.im()) - right.im()); }
  inline IComplex32 sub32 (const IComplex8& left, const IComplex32& right)
  { return IComplex32 (left.re() - right.re(),
		       left.im() - right.im()); }
  inline IComplex32 sub32 (const IComplex32& left, const IComplex8& right)
  { return IComplex32 (left.re() - right.re(),
		       left.im() - right.im()); }
  inline IComplex32 sub32 (const IComplex16& left, const IComplex32& right)
  { return IComplex32 (left.re() - right.re(),
		       left.im() - right.im()); }
  inline IComplex32 sub32 (const IComplex32& left, const IComplex16& right)
  { return IComplex32 (left.re() - right.re(),
		       left.im() - right.im()); }
  inline IComplex32 sub32 (const IComplex32& left, const IComplex32& right)
  { return IComplex32 (left.re() - right.re(),
		       left.im() - right.im()); }

  inline IComplex8 mul8 (const IComplex8& left, const IComplex8& right)
  { return IComplex8 (left.re() * right.re() - left.im() * right.im(),
		      left.re() * right.im() + left.im() * right.re()); }
  inline IComplex16 mul16 (const IComplex8& left, const IComplex8& right)
  { return IComplex16 (int16(left.re()) * right.re() -
		       int16(left.im()) * right.im(),
		       int16(left.re()) * right.im() +
		       int16(left.im()) * right.re()); }
  inline IComplex16 mul16 (const IComplex8& left, const IComplex16& right)
  { return IComplex16 (left.re() * right.re() - left.im() * right.im(),
		       left.re() * right.im() + left.im() * right.re()); }
  inline IComplex16 mul16 (const IComplex16& left, const IComplex8& right)
  { return IComplex16 (left.re() * right.re() - left.im() * right.im(),
		       left.re() * right.im() + left.im() * right.re()); }
  inline IComplex16 mul16 (const IComplex16& left, const IComplex16& right)
  { return IComplex16 (left.re() * right.re() - left.im() * right.im(),
		       left.re() * right.im() + left.im() * right.re()); }
  inline IComplex32 mul32 (const IComplex8& left, const IComplex8& right)
  { return IComplex32 (int32(left.re()) * right.re() -
		       int32(left.im()) * right.im(),
		       int32(left.re()) * right.im() +
		       int32(left.im()) * right.re()); }
  inline IComplex32 mul32 (const IComplex8& left, const IComplex16& right)
  { return IComplex32 (int32(left.re()) * right.re() -
		       int32(left.im()) * right.im(),
		       int32(left.re()) * right.im() +
		       int32(left.im()) * right.re()); }
  inline IComplex32 mul32 (const IComplex16& left, const IComplex8& right)
  { return IComplex32 (int32(left.re()) * right.re() -
		       int32(left.im()) * right.im(),
		       int32(left.re()) * right.im() +
		       int32(left.im()) * right.re()); }
  inline IComplex32 mul32 (const IComplex16& left, const IComplex16& right)
  { return IComplex32 (int32(left.re()) * right.re() -
		       int32(left.im()) * right.im(),
		       int32(left.re()) * right.im() +
		       int32(left.im()) * right.re()); }
  inline IComplex32 mul32 (const IComplex8& left, const IComplex32& right)
  { return IComplex32 (left.re() * right.re() - left.im() * right.im(),
		       left.re() * right.im() + left.im() * right.re()); }
  inline IComplex32 mul32 (const IComplex32& left, const IComplex8& right)
  { return IComplex32 (left.re() * right.re() - left.im() * right.im(),
		       left.re() * right.im() + left.im() * right.re()); }
  inline IComplex32 mul32 (const IComplex16& left, const IComplex32& right)
  { return IComplex32 (left.re() * right.re() - left.im() * right.im(),
		       left.re() * right.im() + left.im() * right.re()); }
  inline IComplex32 mul32 (const IComplex32& left, const IComplex16& right)
  { return IComplex32 (left.re() * right.re() - left.im() * right.im(),
		       left.re() * right.im() + left.im() * right.re()); }
  inline IComplex32 mul32 (const IComplex32& left, const IComplex32& right)
  { return IComplex32 (left.re() * right.re() - left.im() * right.im(),
		       left.re() * right.im() + left.im() * right.re()); }

  inline IComplex8 div8 (const IComplex8& left, const IComplex8& right)
  { int32 n = norm(right);
  return IComplex8 ((int32(left.re()) * right.re() +
                     int32(left.im()) * right.im()) / n,
                    (int32(left.im()) * right.re() -
                     int32(left.re()) * right.im()) / n); }
  inline IComplex16 div16 (const IComplex8& left, const IComplex8& right)
  { int32 n = norm(right);
  return IComplex16 ((int32(left.re()) * right.re() +
                      int32(left.im()) * right.im()) / n,
                     (int32(left.im()) * right.re() -
                      int32(left.re()) * right.im()) / n); }
  inline IComplex16 div16 (const IComplex8& left, const IComplex16& right)
  { int32 n = norm(right);
  return IComplex16 ((int32(left.re()) * right.re() +
                      int32(left.im()) * right.im()) / n,
                     (int32(left.im()) * right.re() -
                      int32(left.re()) * right.im()) / n); }
  inline IComplex16 div16 (const IComplex16& left, const IComplex8& right)
  { int32 n = norm(right);
  return IComplex16 ((int32(left.re()) * right.re() +
                      int32(left.im()) * right.im()) / n,
                     (int32(left.im()) * right.re() -
                      int32(left.re()) * right.im()) / n); }
  inline IComplex16 div16 (const IComplex16& left, const IComplex16& right)
  { int32 n = norm(right);
  return IComplex16 ((int32(left.re()) * right.re() +
                      int32(left.im()) * right.im()) / n,
                     (int32(left.im()) * right.re() -
                      int32(left.re()) * right.im()) / n); }
  inline IComplex32 div32 (const IComplex8& left, const IComplex8& right)
  { int32 n = norm(right);
  return IComplex32 ((int32(left.re()) * right.re() +
                      int32(left.im()) * right.im()) / n,
                     (int32(left.im()) * right.re() -
                      int32(left.re()) * right.im()) / n); }
  inline IComplex32 div32 (const IComplex8& left, const IComplex16& right)
  { int32 n = norm(right);
  return IComplex32 ((int32(left.re()) * right.re() +
                      int32(left.im()) * right.im()) / n,
                     (int32(left.im()) * right.re() -
                      int32(left.re()) * right.im()) / n); }
  inline IComplex32 div32 (const IComplex16& left, const IComplex8& right)
  { int32 n = norm(right);
  return IComplex32 ((int32(left.re()) * right.re() +
                      int32(left.im()) * right.im()) / n,
                     (int32(left.im()) * right.re() -
                      int32(left.re()) * right.im()) / n); }
  inline IComplex32 div32 (const IComplex16& left, const IComplex16& right)
  { int32 n = norm(right);
  return IComplex32 ((int32(left.re()) * right.re() +
                      int32(left.im()) * right.im()) / n,
                     (int32(left.im()) * right.re() -
                      int32(left.re()) * right.im()) / n); }
  inline IComplex32 div32 (const IComplex8& left, const IComplex32& right)
  { int32 n = norm(right);
  return IComplex32 ((left.re() * right.re() +
                      left.im() * right.im()) / n,
                     (left.im() * right.re() -
                      left.re() * right.im()) / n); }
  inline IComplex32 div32 (const IComplex32& left, const IComplex8& right)
  { int32 n = norm(right);
  return IComplex32 ((left.re() * right.re() +
                      left.im() * right.im()) / n,
                     (left.im() * right.re() -
                      left.re() * right.im()) / n); }
  inline IComplex32 div32 (const IComplex16& left, const IComplex32& right)
  { int32 n = norm(right);
  return IComplex32 ((left.re() * right.re() +
                      left.im() * right.im()) / n,
                     (left.im() * right.re() -
                      left.re() * right.im()) / n); }
  inline IComplex32 div32 (const IComplex32& left, const IComplex16& right)
  { int32 n = norm(right);
  return IComplex32 ((left.re() * right.re() +
                      left.im() * right.im()) / n,
                     (left.im() * right.re() -
                      left.re() * right.im()) / n); }
  inline IComplex32 div32 (const IComplex32& left, const IComplex32& right)
  { int32 n = norm(right);
  return IComplex32 ((left.re() * right.re() +
                      left.im() * right.im()) / n,
                     (left.im() * right.re() -
                      left.re() * right.im()) / n); }

  inline IComplex8 mulconj8 (const IComplex8& left, const IComplex8& right)
  { return IComplex8 (left.re() * right.re() + left.im() * right.im(),
		      left.im() * right.re() - left.re() * right.im()); }
  inline IComplex16 mulconj16 (const IComplex8& left, const IComplex8& right)
  { return IComplex16 (int16(left.re()) * right.re() +
		       int16(left.im()) * right.im(),
		       int16(left.im()) * right.re() -
		       int16(left.re()) * right.im()); }
  inline IComplex16 mulconj16 (const IComplex8& left, const IComplex16& right)
  { return IComplex16 (left.re() * right.re() + left.im() * right.im(),
		       left.im() * right.re() - left.re() * right.im()); }
  inline IComplex16 mulconj16 (const IComplex16& left, const IComplex8& right)
  { return IComplex16 (left.re() * right.re() + left.im() * right.im(),
		       left.im() * right.re() - left.re() * right.im()); }
  inline IComplex16 mulconj16 (const IComplex16& left, const IComplex16& right)
  { return IComplex16 (left.re() * right.re() + left.im() * right.im(),
		       left.im() * right.re() - left.re() * right.im()); }
  inline IComplex32 mulconj32 (const IComplex8& left, const IComplex8& right)
  { return IComplex32 (int32(left.re()) * right.re() +
		       int32(left.im()) * right.im(),
		       int32(left.im()) * right.re() -
		       int32(left.re()) * right.im()); }
  inline IComplex32 mulconj32 (const IComplex8& left, const IComplex16& right)
  { return IComplex32 (int32(left.re()) * right.re() +
		       int32(left.im()) * right.im(),
		       int32(left.im()) * right.re() -
		       int32(left.re()) * right.im()); }
  inline IComplex32 mulconj32 (const IComplex16& left, const IComplex8& right)
  { return IComplex32 (int32(left.re()) * right.re() +
		       int32(left.im()) * right.im(),
		       int32(left.im()) * right.re() -
		       int32(left.re()) * right.im()); }
  inline IComplex32 mulconj32 (const IComplex16& left, const IComplex16& right)
  { return IComplex32 (int32(left.re()) * right.re() +
		       int32(left.im()) * right.im(),
		       int32(left.im()) * right.re() -
		       int32(left.re()) * right.im()); }
  inline IComplex32 mulconj32 (const IComplex8& left, const IComplex32& right)
  { return IComplex32 (left.re() * right.re() + left.im() * right.im(),
		       left.im() * right.re() - left.re() * right.im()); }
  inline IComplex32 mulconj32 (const IComplex32& left, const IComplex8& right)
  { return IComplex32 (left.re() * right.re() + left.im() * right.im(),
		       left.im() * right.re() - left.re() * right.im()); }
  inline IComplex32 mulconj32 (const IComplex16& left, const IComplex32& right)
  { return IComplex32 (left.re() * right.re() + left.im() * right.im(),
		       left.im() * right.re() - left.re() * right.im()); }
  inline IComplex32 mulconj32 (const IComplex32& left, const IComplex16& right)
  { return IComplex32 (left.re() * right.re() + left.im() * right.im(),
		       left.im() * right.re() - left.re() * right.im()); }
  inline IComplex32 mulconj32 (const IComplex32& left, const IComplex32& right)
  { return IComplex32 (left.re() * right.re() + left.im() * right.im(),
		       left.im() * right.re() - left.re() * right.im()); }

} // namespace LOFAR

#endif
