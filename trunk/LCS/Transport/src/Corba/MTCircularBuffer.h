//////////////////////////////////////////////////////////////////////
//
//  MTCircularBuffer.h: Thread safe circular buffer implementation.
//
//  Copyright (C) 2000, 2001
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
//
//
//
//////////////////////////////////////////////////////////////////////

#ifndef _MTCIRCULAR_BUFFER_H_
#define _MTCIRCULAR_BUFFER_H_

#ifdef HAVE_CONFIG_H
//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#endif

#include "CEPFrame/Lock.h"

/** A circular buffer template, like a fifo */
template <class TYPE> class MTCircularBuffer
{
	protected:
        THREAD_SAFE_MTX;

	TYPE *	m_buffer;
	int 	m_ndxHead;
	int 	m_ndxTail;
	int 	m_count;
	int	m_max;

	public:

	MTCircularBuffer(int num)
	{
		int cb = sizeof(TYPE) * num;
		m_buffer = (TYPE *) malloc(cb);

		m_ndxHead = 0;
		m_ndxTail = 0;
		m_count = 0;
		m_max = num;
	}

	~MTCircularBuffer()
	{
		THREAD_SAFE_LOCK;
		free((void *)m_buffer);
	}

	bool Put(TYPE t)
	{
	        THREAD_SAFE_LOCK;
		if(m_count < m_max)
		{
			m_buffer[m_ndxHead++]=t;

			if(m_ndxHead >= m_max)
				m_ndxHead=0;
			m_count++;
			return true;
		}
		return false;
	}

	bool Get(TYPE *pt)
	{
	        THREAD_SAFE_LOCK;
		if(pt && (m_count > 0))
		{
			*pt = m_buffer[m_ndxTail++];
			if(m_ndxTail >= m_max)
				m_ndxTail=0;
			m_count--;
			return true;
			
		}
		return false;
	}

};

#endif
