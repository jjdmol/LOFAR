/***************************************************************************
                          FPArray.h  -  description
                             -------------------
    begin                : Tue Jul 1 2003
    copyright            : (C) 2003 by pvss
    email                : pvss@sun.solarsystem
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FPARRAY_H
#define FPARRAY_H

#include <FPValue.h>
#include <vector>

/**
  *@author pvss
  */

class FPArray : public FPValue  {
public: 
	FPArray() : FPValue(ARRAY_VAL) {};
	~FPArray();
  
  /** Write property of float value_. */
  virtual inline void setValue( const bool newVal) {value_ = newVal;};
  /** Read property of float value_. */
  virtual inline bool getValue() const {return value_;};
  /** No descriptions */
  virtual unsigned int unpack(const char* valBuf);
  /** No descriptions */
  virtual unsigned int pack(char* valBuf) const;
  /** No descriptions */
  virtual FPValue* clone() const;
  /** No descriptions */
  virtual void copy(const FPValue* pValue);

private:
  std::vector<FPValue*> values_;
};

#endif
