//#  -*- mode: c++ -*-
//#
//#  MEPData.h: MEP Payload object.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef MEPDATA_H_
#define MEPDATA_H_

#include <unistd.h>

class MEPData
{
  public:
  /**
   * Constructors for a MEPData object.
   */
  MEPData() : m_dataptr(0), m_count(0) { }
	  
  /* Destructor for MEPData. */
  virtual ~MEPData() {}

  public:
  /**
   * Member access.
   */
  void  setBuffer(void* buf, size_t size);
  void* getBuffer() const;

  public:
  /*@{*/
  /**
   * marshalling methods
   */
  unsigned int getSize();
  unsigned int pack  (void* buffer);
  unsigned int unpack(void *buffer);
  /*@}*/

  private:
  /**
   * MEP Payload data
   */
  void*  m_dataptr; // pointer to user data, not owned by this class
  size_t m_count;
};

#endif /* MEPDATA_H_ */
