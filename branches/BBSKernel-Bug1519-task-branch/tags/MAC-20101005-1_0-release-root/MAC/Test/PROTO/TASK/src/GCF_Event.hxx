//  GCFEvent.h: finite state machine events
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

#ifndef GCF_EVENT_HXX
#define GCF_EVENT_HXX

#include <sys/types.h>

struct GCFEvent
{
  GCFEvent() :
    signal(0), pad0(0), length(sizeof(GCFEvent))
  {}

  GCFEvent(unsigned short sig) :
    signal(sig), pad0(0), length(sizeof(GCFEvent))
  {}
  
  enum { ERROR = -1, HANDLED = 0, NOT_HANDLED = 1};

  /**
   * @code
   * Signal format 
   *
   * 2 most significant bits indicate direction of signal:
   *   F_IN    = 0b01
   *   F_OUT   = 0b10
   *   F_INOUT = 0b11 (F_IN_SIGNAL | F_OUT_SIGNAL)
   *
   * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
   * | O | I | P | P | P | P | P | S | S | S | S | S | S | S | S | S |
   * +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
   *  15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
   * <- I/O-><--- protocol ---------><--------- signal -------------->
   * @endcode
   */
  unsigned short  signal; // lsb contains signal id (0-255)
                          // msb contains protocol id (0-255)
  unsigned short pad0; // DO NOT USE
  size_t          length; // length of the event (should be <= SSIZE_MAX)
};

/**
 * Macros to aid in decoding the signal field.
 */
#define F_EVT_INOUT_MASK    (0xc000)
#define F_EVT_PROTOCOL_MASK (0x3f00)
#define F_EVT_SIGNAL_MASK   (0x00ff)

#define F_EVT_INOUT(e)    (((e).signal & F_EVT_INOUT_MASK) >> 14)
#define F_EVT_PROTOCOL(e) (((e).signal & F_EVT_PROTOCOL_MASK) >> 8)
#define F_EVT_SIGNAL(e)    ((e).signal & F_EVT_SIGNAL_MASK)

#endif
