//#  -*- mode: c++ -*-
//#
//#  RefCount.h: RSP Driver command class
//#
//#  Copyright (C) 2002-2004
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

#ifndef REFCOUNT_H_
#define REFCOUNT_H_

namespace LOFAR {
  namespace RSP {

    class RefCount {
      int crefs;
    public:
      RefCount(void) { crefs = 0; }
      virtual ~RefCount() { }
      
      void upcount(void) { ++crefs; }
      void downcount(void)
      {
	if (--crefs == 0)
	  {
	    delete this;
	  }
      }
    };

    template <class T> class Ptr {
      T* p;
    public:
      Ptr(T* p_) : p(p_) { p->upcount(); }
      Ptr(const Ptr& p_) : p(p_.p) { p->upcount(); }
      virtual ~Ptr(void) { p->downcount(); }
      operator T*(void) { return p; }
      T& operator*(void) { return *p; }
      T* operator->(void) { return p; }
      //Ptr<T>* operator&(void) { return this; }
      Ptr& operator=(Ptr<T> &p_)
      {return operator=((T *) p_);}
      Ptr& operator=(T* p_) {
	p->downcount(); p = p_; p->upcount(); return *this;
      }
    };

  };
};
     
#endif /* REFCOUNT_H_ */
