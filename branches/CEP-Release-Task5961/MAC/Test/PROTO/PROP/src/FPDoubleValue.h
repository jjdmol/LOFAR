//
//  FPDoubleValue.h: 
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

#ifndef FPDOUBLEVALUE_H
#define FPDOUBLEVALUE_H

#include "FPValue.h"

/**
  *@author pvss
  */

class FPDoubleValue : public FPValue
{
public: 
	FPDoubleValue(const double& val = 0.0) : FPValue(LPT_DOUBLE), value_(val) {;}
	virtual ~FPDoubleValue() {;}

  /** Write property of double value_. */
  virtual inline void setValue( const double& newVal) {value_ = newVal;}
  /** Read property of double value_. */
  virtual inline double getValue() const {return value_;}
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
  double value_;
};

class FPComplexDoubleValue : public FPValue
{
public:
	FPComplexDoubleValue(const double val[]) : FPValue(COMPLEX_LPT_DOUBLE)
    {value_[0] = val[0]; value_[1] = val[1];}
	FPComplexDoubleValue(const double real = 0.0, const double imag = 0.0) : FPValue(COMPLEX_LPT_DOUBLE)
    {value_[0] = real; value_[1] = imag;}
	virtual ~FPComplexDoubleValue() {;}

  /** Write property of double value_. */
  inline void setValue( const double newVal[]) {value_[0] = newVal[0]; value_[1] = newVal[1];}
  /** Write the real part of property of double value_. */
  inline void setReal( const double real) {value_[0] = real;}
  /** Write the imag part of property of double value_. */
  inline void setImag( const double imag) {value_[1] = imag;}
  /** Read property of double value_. */
  inline const double* getValue() const {return value_;}
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
  double value_[2];
};

#endif
