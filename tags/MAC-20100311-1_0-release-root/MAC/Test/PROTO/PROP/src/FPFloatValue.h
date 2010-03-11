//
//  FPFloatValue.h: 
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#ifndef FPFLOATVALUE_H
#define FPFLOATVALUE_H

#include "FPValue.h"

/**
  *@author pvss
  */

class FPFloatValue : public FPValue
{
public: 
	FPFloatValue(const float val = 0.0f) : FPValue(FLOAT_VAL), value_(val) {;}
	virtual ~FPFloatValue() {;}

  /** Write property of float value_. */
  inline void setValue( const float newVal) {value_ = newVal;}
  /** Read property of float value_. */
  inline float getValue() const {return value_;}
  /** No descriptions */
  virtual FPValue* clone() const;
  /** No descriptions */
  virtual void copy(const FPValue* pValue);

private: // Private attributes
  /** No descriptions */
  virtual uint unpack(const char* valBuf);
  /** No descriptions */
  virtual uint pack(char* valBuf) const;
  /**  */
  float value_;
};

class FPComplexFloatValue : public FPValue  {
public:
	FPComplexFloatValue(const float val[2]) : FPValue(COMPLEX_FLOAT_VAL)
    {value_[0] = val[0]; value_[1] = val[1];}
	FPComplexFloatValue(const float real = 0.0f, const float imag = 0.0f) : FPValue(COMPLEX_FLOAT_VAL)
    {value_[0] = real; value_[1] = imag;}

  virtual ~FPComplexFloatValue() {;}

  /** Write property of float value_. */
  inline void setValue( const float val[2]) {value_[0] = val[0]; value_[1] = val[1];}
  /** Write the real part of property of float value_. */
  inline void setReal( const float real) {value_[0] = real;}
  /** Write the imag part of property of float value_. */
  inline void setImag( const float imag) {value_[1] = imag;}
  /** Read property of float value_. */
  inline const float* getValue() const {return value_;}
  /** No descriptions */
  virtual uint unpack(const char* valBuf);
  /** No descriptions */
  virtual uint pack(char* valBuf) const;
  /** No descriptions */
  virtual FPValue* clone() const;
  /** No descriptions */
  virtual void copy(const FPValue* pValue);

private: // Private attributes
  /**  */
  float value_[2];
};
#endif
