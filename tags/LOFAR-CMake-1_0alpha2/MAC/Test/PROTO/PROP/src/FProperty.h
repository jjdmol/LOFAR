//
//  FProperty.h: 
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

#ifndef FPROPERTY_H
#define FPROPERTY_H


#include "FPValue.h"
#ifdef ERTC
#include "FEvent.h"
#include "FPortInterface.h"
#include "FSupervisedTask.h"
#endif
const uchar FP_SUBSCRIPTION_AND_GET_ALLOWED = 0x01;
const uchar FP_SET_ALLOWED = 0x02;
const uchar FP_SET_IMMEDIATE = 0x04; /* otherwise delayed/synchronous */

/**
  *@author pvss
  */

class FProperty {
public:
	FProperty(const char* name, FPValue::ValueType type);
	virtual ~FProperty();
  /** Read property of FPValue* pOnlineValue_. */
  virtual const FPValue* getValue() const;
  /** Read property of FPValue* pOldValue_. */
  virtual const FPValue* getOldValue() const;
  /** Write property of int accessMode. */
  virtual void setAccessMode( const uchar& _newVal);
  /** Read property of int accessMode. */
  inline uchar getAccessMode() const {return accessMode_; }
  /** Write property of bool subscribed_. */
  virtual int enableSubscription( const bool& newVal);
  /** Read property of bool subscribed_. */
  inline bool getSubscriptionMode() {return subscribed_;}

  /** No descriptions */
  inline const char* getName() const {return name_;}
  /** No descriptions */
  void syncValues();
#ifdef ERTC
private:
#endif
  /** No descriptions */
  uint unpack(const char* valBuf);
  /** No descriptions */
  uint pack(char* valBuf) const;
  /** Write property of FPValue* pOriginalValue_.*/
  short setValue( const FPValue& pNewVal);

#ifdef ERTC
  friend int FSupervisedTask::operational_state(FEvent& , FPortInterface& );
  friend void FSupervisedTask::sendValue(FEvent&, const FProperty *);
  friend short FSupervisedTask::setValue(const char*, const FPValue &);
#endif

private: // Private attributes
  /**  */
  FPValue* pOldValue_;
  /**  */
  FPValue* pOnlineValue_;
  /**  */
  FPValue* pOriginalValue_;
  /**  */
  bool subscribed_;
  /**  */
  uchar accessMode_;
  /** */
  char* name_;
};

#endif
