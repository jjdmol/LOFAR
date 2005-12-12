//#  CyclicCounter.h: 
//#  small class to control a cyclic counter from 0 to itsMaxNumbers-1
//#
//#  Copyright (C) 2000, 2001
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$


#ifndef  TFLOPCORRELATOR_CYCLICCOUNTER_H
#define  TFLOPCORRELATOR_CYCLICCOUNTER_H

#include <Common/lofar_iostream.h>

namespace LOFAR
{    
  class CyclicCounter {
  
  public:
    CyclicCounter(const int maxnum = 0);
  
    void operator+= (int increment);
    void operator++ (int); 
    void operator-= (int decrement);
    void operator-- (int);
    void operator+= (CyclicCounter& other);
    void operator-= (CyclicCounter& other); 

    int operator= (int newValue);
    int operator+ (int increment);
    int operator- (int decrement);
    int operator+ (CyclicCounter& other);  
    int operator- (CyclicCounter& other);

    bool operator== (int other);
    bool operator>= (int other);
    bool operator<= (int other);
    bool operator>  (int other);
    bool operator<  (int other);
    bool operator== (CyclicCounter& other);
    bool operator>= (CyclicCounter& other);
    bool operator<= (CyclicCounter& other);
    bool operator>  (CyclicCounter& other);
    bool operator<  (CyclicCounter& other);

    friend ostream& operator<<(ostream& os, const CyclicCounter& ss);

    int getValue() const;

  private:
    int itsValue;
    int itsMaxNumbers;
    void checkValue();
    int checkValue(int value);
  };

  inline int CyclicCounter::operator= (int newValue)
    {
      itsValue = newValue;
      checkValue();
      return itsValue;
    }

  inline int CyclicCounter::getValue() const
    { 
      return itsValue; 
    }

  inline void CyclicCounter::operator+= (int increment)
    { 
      itsValue += increment; 
      checkValue();   
    }

  inline void CyclicCounter::operator++ (int)
    { 
      itsValue++; 
      checkValue();
    }

  inline void CyclicCounter::operator-= (int decrement)
    {
      itsValue -= decrement;
      checkValue();
    }
  
  inline  void CyclicCounter::operator-- (int)
    {
      itsValue--;
      checkValue();
    }

  inline int CyclicCounter::operator+ (int increment)
    {
      return checkValue(itsValue + increment);
    }
  
  inline int CyclicCounter::operator- (int decrement)
    {
      return checkValue(itsValue - decrement);
    }
  
  inline int CyclicCounter::operator+ (CyclicCounter& other)
    {
      return checkValue(itsValue + other.itsValue);
    }
  
  inline int CyclicCounter::operator- (CyclicCounter& other)
    {
      return checkValue(itsValue - other.itsValue);
    }

  inline bool CyclicCounter::operator== (int other)
    {
      return (itsValue == other);
    }
  
  inline bool CyclicCounter::operator>= (int other)
    {
      return (itsValue >= other);
    }
  
  inline bool CyclicCounter::operator<= (int other)
    {
      return (itsValue <= other);
    }
  
  inline bool CyclicCounter::operator> (int other)
    {
      return (itsValue > other);
    }
  
  inline bool CyclicCounter::operator< (int other)
    {
      return (itsValue < other);
    }
  
  inline bool CyclicCounter::operator== (CyclicCounter& other)
    {
      return (itsValue == other.itsValue);
    }
  
  inline bool CyclicCounter::operator>= (CyclicCounter& other)
    {
      return (itsValue >= other.itsValue);
    }
  
  inline bool CyclicCounter::operator<= (CyclicCounter& other)
    {
      return (itsValue <= other.itsValue);
    }
  
  inline bool CyclicCounter::operator> (CyclicCounter& other)
    {
      return (itsValue > other.itsValue);
    }
  
  inline bool CyclicCounter::operator< (CyclicCounter& other)
    {
      return (itsValue < other.itsValue);
    }
}
#endif

