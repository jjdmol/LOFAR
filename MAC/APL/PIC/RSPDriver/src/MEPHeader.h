//#  MEPHeader.h: Waveform Generator control information
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

#ifndef MEPHEADER_H_
#define MEPHEADER_H_

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace RSP_Protocol
{
  class MEPHeader
      {
      public:
	  /**
	   * Constructors for a MEPHeader object.
	   */
	  MEPHeader() { }
	  
	  /* Destructor for MEPHeader. */
	  virtual ~MEPHeader() {}

	  /**
	   * Size of MEP header in bytes.
	   */
	  static const unsigned int SIZE = 12;

	  /**
	   * Message types.
	   */
	  /*@{*/
	  static const uint8 READ    = 0x01;
	  static const uint8 WRITE   = 0x02;
	  static const uint8 READRES = 0x03;
	  /*@}*/

	  /**
	   * Address constants
	   * Destination ID
	   */
	  /*@{*/
	  static const uint8 DST_BLP_BASE = 0x00; /* BLP's are addressed starting from 0x00 */
	  static const uint8 DST_RSP      = 0xFF; /* Board entity */
	  /*@}*/

	  /**
	   * Process IDs
	   */
	  /*@{*/
	  static const uint8 STATUS  = 0x00; /* FPGA status overview */
	  static const uint8 TST     = 0x01; /* Selftest functionality */
	  static const uint8 CFG     = 0x02; /* FPGA configuration and reset */
	  static const uint8 WG      = 0x03; /* Waveform generator */
	  static const uint8 SS      = 0x04; /* Subband select */
	  static const uint8 BF      = 0x05; /* Beam former */
	  static const uint8 ST      = 0x06; /* Statistics */
	  static const uint8 RCU     = 0x07; /* RCU control */
	  /*@}*/

	  /**
	   * Page IDs
	   */
	  /*@{*/
	  static const uint8 PAGE_INACTIVE = 0x00; /* Write page for LCU */
	  static const uint8 PAGE_ACTIVE   = 0x01; /* Read page for FPGA */
	  /*@}*/

      public:
	  /**
	   * marshalling methods
	   */
	  /*@{*/
	  unsigned int getSize();
	  unsigned int pack  (void* buffer);
	  unsigned int unpack(void *buffer);
	  /*@}*/

      private:
	  /**
	   * MEP header fields
	   */
	  uint8  m_type;    /* Message type */
	  uint8  m_ffi;     /* for future implementation */
	  uint16 m_seqnr;   /* Sequence number */
	  typedef struct
	  {
	    uint8 m_dstid;  /* Destination ID */
	    uint8 m_pid;    /* Process ID */
	    uint8 m_regid;  /* Register ID */
	    uint8 m_pageid; /* Page ID */
	  } m_addr;
	  uint16 m_offset;  /* Register offset */
	  uint16 m_size;    /* Read/write size */
      };
};
     
#endif /* MEPHEADER_H_ */
