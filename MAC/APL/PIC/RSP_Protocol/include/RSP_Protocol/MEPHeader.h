//#  -*- mode: c++ -*-
//#
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

namespace EPA_Protocol
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

      /*@{*/
      /**
       * Message types.
       */
      static const uint8 READ    = 0x01;
      static const uint8 WRITE   = 0x02;
      static const uint8 READRES = 0x03;
      /*@}*/

      /*@{*/
      /**
       * Address constants
       * Destination ID
       */
      static const uint8 DST_BLP_BASE = 0x00; /* BLP's are addressed starting from 0x00 */
      static const uint8 DST_RSP      = 0xFF; /* Board entity */
      /*@}*/

      /*@{*/
      /**
       * Process IDs
       */
      static const uint8 STATUS  = 0x00; /* FPGA status overview */
      static const uint8 TST     = 0x01; /* Selftest functionality */
      static const uint8 CFG     = 0x02; /* FPGA configuration and reset */
      static const uint8 WG      = 0x03; /* Waveform generator */
      static const uint8 SS      = 0x04; /* Subband select */
      static const uint8 BF      = 0x05; /* Beam former */
      static const uint8 ST      = 0x06; /* Statistics */
      static const uint8 RCU     = 0x07; /* RCU control */
      /*@}*/

      /*@{*/
      /**
       * Register IDs
       */
      static const uint8 RSPSTATUS     = 0x00;

      static const uint8 SELFTEST      = 0x00;

      static const uint8 RESET         = 0x00;
      static const uint8 REPROGRAM     = 0x01;

      static const uint8 WGSETTINGS    = 0x00;
      static const uint8 WGUSER        = 0x01;

      static const uint8 NRSUBBANDS    = 0x00;
      static const uint8 SUBBANDSELECT = 0x01;

      static const uint8 BFXRE         = 0x00;
      static const uint8 BFXIM         = 0x01;
      static const uint8 BFYRE         = 0x02;
      static const uint8 BFYIM         = 0x03;

      static const uint8 MEAN          = 0x00;
      static const uint8 POWER         = 0x01;

      static const uint8 RCUSETTINGS   = 0x00;
      /*@}*/

      /*@{*/
      /**
       * Page IDs
       */
      static const uint8 PAGE_INACTIVE = 0x00; /* Write page for LCU */
      static const uint8 PAGE_ACTIVE   = 0x01; /* Read page for FPGA */
      /*@}*/

      /*@{*/
      /**
       * Read/write sizes in octets (= bytes)
       */
      static const uint16 RSPSTATUS_SIZE     = 24; // 22 in EPA documentation!

      static const uint16 SELFTEST_SIZE      = 1;

      static const uint16 RESET_SIZE         = 1;
      static const uint16 REPROGRAM_SIZE     = 1;

      static const uint16 WGSETTINGS_SIZE    = 7;
      static const uint16 WGUSER_SIZE        = 1024;

      static const uint16 NRSUBBANDS_SIZE    = 2;
      static const uint16 SUBBANDSELECT_SIZE = 512;

      static const uint16 BFCOEFS_SIZE       = 1024;

      static const uint16 STSTATS_SIZE       = 1024;

      static const uint16 RCUSETTINGS_SIZE   = 2;
      /*@}*/

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack  (void* buffer);
      unsigned int unpack(void *buffer);
      /*@}*/

    public:
      /**
       * Method to set appropriate fields.
       */
      void set(uint8  type,
	       uint8  dstid,
	       uint8  pid,
	       uint8  regid,
	       uint16 size);

    public:
      /**
       * MEP header fields
       */
      typedef struct
      {
	  uint8  type;      /* Message type */
	  uint8  ffi;       /* for future implementation */
	  uint16 seqnr;     /* Sequence number */
	  typedef struct 
	  {
	      uint8 dstid;  /* Destination ID */
	      uint8 pid;    /* Process ID */
	      uint8 regid;  /* Register ID */
	      uint8 pageid; /* Page ID */
	  } AddrType;
	  AddrType addr;    /* addr */
	  uint16 offset;    /* Register offset */
	  uint16 size;      /* Read/write size */
      } FieldsType;

      FieldsType m_fields;
  };
};

// macro to shorten writing the context of the constants
#define CTX(a) EPA_Protocol::MEPHeader::a

#define MEP_RSPSTATUS(hdr, oper) \
  (hdr).set(oper,         CTX(DST_RSP),              CTX(STATUS), CTX(RSPSTATUS),     (CTX(READ) == oper?0:CTX(RSPSTATUS_SIZE)))
#define MEP_SELFTEST(hdr) \
  (hdr).set(CTX(WRITE),   CTX(DST_RSP),              CTX(TST),    CTX(SELFTEST),      CTX(SELFTEST_SIZE))
#define MEP_RESET(hdr) \
  (hdr).set(CTX(WRITE),   CTX(DST_RSP),              CTX(CFG),    CTX(RESET),         CTX(RESET_SIZE))
#define MEP_REPROGRAM(hdr) \
  (hdr).set(CTX(WRITE),   CTX(DST_RSP),              CTX(CFG),    CTX(REPROGRAM),     CTX(REPROGRAM_SIZE))

#define MEP_WGSETTINGS(hdr, oper, dstid) \
  (hdr).set(oper,         CTX(DST_BLP_BASE) + dstid, CTX(WG),     CTX(WGSETTINGS),    CTX(WGSETTINGS_SIZE))
#define MEP_WGUSER(hdr, oper, dstid) \
  (hdr).set(oper,         CTX(DST_BLP_BASE) + dstid, CTX(WG),     CTX(WGUSER),        CTX(WGUSER_SIZE))
#define MEP_NRSUBBANDS(hdr, oper, dstid) \
  (hdr).set(oper,         CTX(DST_BLP_BASE) + dstid, CTX(SS),     CTX(NRSUBBANDS),    CTX(NRSUBBANDS_SIZE))
#define MEP_SUBBANDSELECT(mpe, oper, dstid) \
  (hdr).set(oper,         CTX(DST_BLP_BASE) + dstid, CTX(SS),     CTX(SUBBANDSELECT), CTX(SUBBANDSELECT_SIZE))
#define MEP_BF(hdr, oper, dstid, regid) \
  (hdr).set(oper,         CTX(DST_BLP_BASE) + dstid, CTX(BF),     (regid),            CTX(BFCOEFS_SIZE))
#define MEP_ST(hdr, oper, dstid, regid) \
  (hdr).set(oper,         CTX(DST_BLP_BASE) + dstid, CTX(ST),     (regid),            CTX(STSTATS_SIZE))
#define MEP_RCUSETTINGS(hdr, oper, dstid) \
  (hdr).set(oper,         CTX(DST_BLP_BASE) + dstid, CTX(RCU),    CTX(RCUSETTINGS),   CTX(RCUSETTINGS_SIZE))

#endif /* MEPHEADER_H_ */
