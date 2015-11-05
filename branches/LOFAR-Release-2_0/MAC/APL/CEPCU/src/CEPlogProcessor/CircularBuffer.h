//#  CircularBuffer.h: Moves the operator info from the logfiles to PVSS
//#
//#  Copyright (C) 2009
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
//#  $Id: CEPlogProcessor.h 16954 2010-12-15 10:03:09Z mol $
#ifndef LOFAR_APL_CIRCULARBUFFER_H
#define LOFAR_APL_CIRCULARBUFFER_H

// \file
// Daemon for launching Application Controllers

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes

namespace LOFAR {
	namespace APL {

class CircularBuffer {
public:
  CircularBuffer( unsigned capacity ):
    buffer(0),
    begin(0),
    end(0),
    head(0),
    tail(0),
    capacity(capacity),
    full(capacity == 0)
  {
    buffer = new char[capacity];
    begin  = buffer;
    end    = buffer + capacity;
    head   = buffer;
    tail   = buffer;
  }

  ~CircularBuffer() {
    delete[] buffer;
  }

  bool empty() const {
    return head == tail && !full;
  }

  unsigned freeSpace() const {
    return full ? 0 : (head <= tail ? end - tail + head - begin : tail - head);
  }

  unsigned tailFreeSpace() const {
    return full ? 0 : (head <= tail ?  end - tail : head - tail);
  }

  void incTail( unsigned len ) {
    tail += len;
    if (tail == end) tail = begin;
    if (tail == head) full = true;
  }

  unsigned putData( char *buf, unsigned buflen ) {
    if (full)
      return 0;

    if (buflen == 0)
      return 0;

    unsigned first_buflen = tailFreeSpace();
    unsigned second_buflen = freeSpace() - first_buflen;

    if (buflen <= first_buflen) {
      first_buflen = buflen;
      second_buflen = 0;
    } else if (buflen <= first_buflen + second_buflen) {
      second_buflen = buflen - first_buflen;
    } else {
      // discard data beyond freeSpace()
    }

    memcpy( tail, buf, first_buflen );
    incTail( first_buflen );

    if (second_buflen) {
      memcpy( tail, buf + first_buflen, second_buflen );
      incTail( second_buflen );
    }

    return first_buflen + second_buflen;
  }

  bool getLine( char *buf, unsigned buflen ) {
    if (empty())
      return false;

    if (buflen == 0)
      return true;

    if (buflen == 1) {
      *buf = 0;
      return true;
    }

    buflen--; // reserve space for the trailing 0

    char *c;
    bool twoparts = head >= tail;

    char *firstend = twoparts ? end : tail;

    for (c = head; c == head || c != firstend; c++) {
      if (*c != '\n')
        continue;

      // line found
      unsigned linelen = c - head;
      if (linelen >= buflen )
        linelen = buflen;

      memcpy( buf, head, linelen );
      buf[linelen] = 0;

      head = c+1;
      if (head == end)
        head = begin;

      full = false;

      return true;
    }

    if (twoparts) {
      for (c = begin; c != tail; c++) {
        if (*c != '\n')
          continue;

        // line found
        unsigned first_linelen = end - head;
        unsigned second_linelen = c - begin;

        if (first_linelen >= buflen) {
          first_linelen = buflen;
          second_linelen = 0;
        } else if (first_linelen + second_linelen >= buflen) {
          second_linelen = buflen - first_linelen;
        }

        memcpy( buf, head, first_linelen );
        memcpy( buf + first_linelen, begin, second_linelen );

        buf[first_linelen + second_linelen] = 0;

        head = c+1;
        if (head == end)
          head = begin;

        full = false;

        return true;
      }
    }

    // no line found
    return false;
  }

public:
  char *buffer;
  char *begin, *end;
  char *head, *tail;
  unsigned capacity;
  bool full;

};

// @} addgroup
  } // namespace APL
} // namespace LOFAR

#endif

