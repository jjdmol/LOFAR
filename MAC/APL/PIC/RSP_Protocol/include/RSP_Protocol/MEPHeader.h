//#  -*- mode: c++ -*-
//#
//#  MEPHeader.h: Definition of the EPA MEP header.
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
      static const uint8 TYPE_UNSET = 0x00;
      static const uint8 READ       = 0x01;
      static const uint8 WRITE      = 0x02;
      static const uint8 READACK    = 0x03;
      static const uint8 WRITEACK   = 0x04;

      static const int MAX_TYPE = WRITEACK; /* counting from 0 */
      /*@}*/

      /*@{*/
      /**
       * Address constants
       *
       * Destination ID
       * RSP Board: bit7 set, all other bits 0 (= 0x80)
       * BLP:       bit7 unset, other bits indicate which BLP is addressed.
       * Two broadcasts are supported:
       * To all BLP's: 0x7F
       * To all BLP's and the RSP board: 0xFF
       */
      static const uint8 DST_BLP            = 0x00; /* BLP's are addressed starting from 0x00 */
      static const uint8 DST_RSP            = 0x80; /* Destination id of the RSP board */
      static const uint8 DST_BROADCAST_BLPS = 0x7F; /* Broadcast to all BLP's but not the RSP */
      static const uint8 DST_BROADCAST      = 0xFF; /* Broadcast to RSP and all BLP's */
      /*@}*/

      /*@{*/
      /**
       * Process IDs
       */
      static const uint8 RSR     = 0x00; /* Status overview */
      static const uint8 TST     = 0x01; /* Selftest functionality */
      static const uint8 CFG     = 0x02; /* FPGA configuration and reset */
      static const uint8 WG      = 0x03; /* Waveform generator */
      static const uint8 SS      = 0x04; /* Subband select */
      static const uint8 BF      = 0x05; /* Beamformer */
      static const uint8 BST     = 0x06; /* Beamformer statistics */
      static const uint8 SST     = 0x07; /* Subband statistics */
      static const uint8 RCU     = 0x08; /* RCU control */
      static const uint8 CRR     = 0x09; /* RSP clock and reset */
      static const uint8 CRB     = 0x0A; /* BLP clock and reset */
      static const uint8 CDO     = 0x0B; /* CEP Data Output */

      static const int MAX_PID = CDO; /* counting from 0 */
      /*@}*/

      /*@{*/
      /**
       * Register IDs
       */
      static const uint8 RSR_STATUS    = 0x00;
      static const uint8 RSR_VERSION   = 0x01;

      static const uint8 TST_SELFTEST  = 0x00;

      static const uint8 CFG_RESET     = 0x00;
      static const uint8 CFG_REPROGRAM = 0x01;

      static const uint8 WG_XSETTINGS  = 0x00;
      static const uint8 WG_YSETTINGS  = 0x01;
      static const uint8 WG_XWAVE      = 0x02;
      static const uint8 WG_YWAVE      = 0x03;
      
      static const uint8 SS_SELECT     = 0x00;

      static const uint8 BF_XROUT      = 0x00;
      static const uint8 BF_XIOUT      = 0x01;
      static const uint8 BF_YROUT      = 0x02;
      static const uint8 BF_YIOUT      = 0x03;

      static const uint8 BST_MEAN      = 0x00; // used as index in statistics array
      static const uint8 BST_POWER     = 0x01; // used as index in statistics array

      static const uint8 SST_MEAN      = 0x00; // used as index in statistics array
      static const uint8 SST_POWER     = 0x01; // used as index in statistics array

      static const uint8 RCU_SETTINGS  = 0x00;

      static const uint8 CRR_SOFTRESET = 0x00;
      static const uint8 CRR_SOFTPPS   = 0x01;

      static const uint8 CRB_SOFTRESET = 0x00;
      static const uint8 CRB_SOFTPPS   = 0x01;

      static const uint8 CDO_SETTINGS  = 0x00;

      static const int MAX_REGID = 0x03;
      
      /*@}*/

      /*@{*/
      /**
       * Define the number of beamlets N_BEAMLETS
       * supported by the EPA firmware. For FTS-1
       * the number of beamlets supported is 128.
       * For the final LOFAR remote station
       * 256 beamlets will be supported.
       *
       * Many register sizes are derived from
       * the number of beamlets.
       *
       * The N_SUBBANDS(512) defines the number of
       * subbands produced by the EPA digital filter.
       * The N_BEAMLETS are a selection from this
       * number of beamlets.
       */
      static const uint16 N_SUBBANDS = 512;
      static const uint16 N_BEAMLETS = 256; //128; // FTS-1 spec, final remote station will be 256
      static const uint16 N_POL      = 2;                // number of polarizations
      static const uint16 N_PHASE    = 2;                // number of phases in a complex number
      static const uint16 N_PHASEPOL = N_PHASE * N_POL;  // number of phase polarizations
 
      //
      // Registers too large to send in a single ethernet frame
      // (> 1500 bytes) will be sent in a number of fragments of this size.
      //
      static const uint16 FRAGMENT_SIZE = 1024;
      
      /**
       * Read/write sizes in octets (= bytes)
       */
      static const uint16 RSR_STATUS_SIZE    = 96;
      static const uint16 RSR_VERSION_SIZE   = 3;
      
      static const uint16 TST_SELFTEST_SIZE  = 1;
      
      static const uint16 CFG_RESET_SIZE     = 1;
      static const uint16 CFG_REPROGRAM_SIZE = 1;
      
      static const uint16 WG_XSETTINGS_SIZE  = 7;
      static const uint16 WG_YSETTINGS_SIZE  = 7;
      static const uint16 WG_XWAVE_SIZE      = 1024;
      static const uint16 WG_YWAVE_SIZE      = 1024;
      
      static const uint16 SS_SELECT_SIZE     = N_BEAMLETS * N_POL * sizeof(uint16);

      static const uint16 BF_XROUT_SIZE      = N_BEAMLETS * N_PHASEPOL * sizeof(int16);
      static const uint16 BF_XIOUT_SIZE      = N_BEAMLETS * N_PHASEPOL * sizeof(int16);
      static const uint16 BF_YROUT_SIZE      = N_BEAMLETS * N_PHASEPOL * sizeof(int16);
      static const uint16 BF_YIOUT_SIZE      = N_BEAMLETS * N_PHASEPOL * sizeof(int16);
      
      static const uint16 BST_MEAN_SIZE      = N_BEAMLETS * N_PHASEPOL * sizeof(int32);
      static const uint16 BST_POWER_SIZE     = N_BEAMLETS * N_PHASEPOL * sizeof(uint32);

      static const uint16 SST_MEAN_SIZE      = N_SUBBANDS * N_POL * N_PHASE * sizeof(int32);
      static const uint16 SST_POWER_SIZE     = N_SUBBANDS * N_POL * N_PHASE * sizeof(uint32);
      
      static const uint16 RCU_SETTINGS_SIZE  = 2;
      
      static const uint16 CRR_SOFTRESET_SIZE = 1;
      static const uint16 CRR_SOFTPPS_SIZE   = 1;
      
      static const uint16 CRB_SOFTRESET_SIZE = 1;
      static const uint16 CRB_SOFTPPS_SIZE   = 1;
      
      static const uint16 CDO_SETTINGS_SIZE  = 10;
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
       * MEP header fields
       */
      typedef struct
      {
	  uint8  type;      /* Message type */
	  uint8  error;     /* Error indicator */
	  uint16 seqnr;     /* Sequence number */
	  typedef struct 
	  {
	      uint8 dstid;  /* Destination ID */
	      uint8 pid;    /* Process ID */
	      uint8 regid;  /* Register ID */
	      uint8 ffi;    /* for future implementation */
	  } AddrType;
	  AddrType addr;    /* addr */
	  uint16 offset;    /* Register offset */
	  uint16 size;      /* Read/write size */
      } FieldsType;

      FieldsType m_fields;

    public:

      /*@{*/
      /**
       * Methods to set header fields.
       */
      void set(uint8  type,
	       uint8  dstid,
	       uint8  pid,
	       uint8  regid,
	       uint16 size,
	       uint16 offset = 0);

      void set(MEPHeader::FieldsType hdrtemplate,
	       uint8  dstid  = DST_RSP,
	       uint8  type   = MEPHeader::TYPE_UNSET,
	       uint16 size   = 0,
	       uint16 offset = 0);
      
      /*@}*/

      /*@{*/
      //
      // The following static constants are templates
      // to produce EPA message headers. This is done by 
      // copying the template and then make modifications
      // to the appropriate parts of the header.
      //
      static const FieldsType RSR_STATUS_HDR;
      static const FieldsType RSR_VERSION_HDR;

      static const FieldsType TST_SELFTEST_HDR;

      static const FieldsType CFG_RESET_HDR;
      static const FieldsType CFG_REPROGRAM_HDR;

      static const FieldsType WG_XSETTINGS_HDR;
      static const FieldsType WG_YSETTINGS_HDR;
      static const FieldsType WG_XWAVE_HDR;
      static const FieldsType WG_YWAVE_HDR;
      
      static const FieldsType SS_SELECT_HDR;

      static const FieldsType BF_XROUT_HDR;
      static const FieldsType BF_XIOUT_HDR;
      static const FieldsType BF_YROUT_HDR;
      static const FieldsType BF_YIOUT_HDR;

      static const FieldsType BST_MEAN_HDR;
      static const FieldsType BST_POWER_HDR;

      static const FieldsType SST_MEAN_HDR;
      static const FieldsType SST_POWER_HDR;

      static const FieldsType RCU_SETTINGS_HDR;

      static const FieldsType CRR_SOFTRESET_HDR;
      static const FieldsType CRR_SOFTPPS_HDR;

      static const FieldsType CRB_SOFTRESET_HDR;
      static const FieldsType CRB_SOFTPPS_HDR;

      static const FieldsType CDO_SETTINGS_HDR;
      /*@}*/
  };
};

#endif /* MEPHEADER_H_ */
