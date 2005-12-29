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

#undef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))

namespace LOFAR {
  namespace EPA_Protocol {

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
      static const unsigned int SIZE = 16;

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
       * Definition of the addr.dstid values.
       * Any of DST_BLP* and DST_RSP may be or-ed together
       * to multicast to those destinations.
       */
      static const uint16 DST_BLP0     = 0x0001; /* BLP 0, byte 0, bit 0  */
      static const uint16 DST_BLP1     = 0x0002; /* BLP 1, byte 0, bit 1  */
      static const uint16 DST_BLP2     = 0x0004; /* BLP 2, byte 0, bit 2  */
      static const uint16 DST_BLP3     = 0x0008; /* BLP 3, byte 0, bit 3  */
      static const uint16 DST_RSP      = 0x0100; /* RSP,   byte 1, bit 0 */

      /* multicast constants */
      static const uint16 DST_ALL_BLPS = DST_BLP0 | DST_BLP1 | DST_BLP2 | DST_BLP3; /* All BLP, but not the RSP */
      static const uint16 DST_ALL      = DST_ALL_BLPS | DST_RSP;                    /* All FPGA's (including RSP) */
      /*@}*/

      /*@{*/
      /**
       * Process IDs
       *
       * Constants extracted from MEP VHDL
       *
       * CONSTANT c_mep_pid_rsr     : NATURAL :=  1;
       * CONSTANT c_mep_pid_rsu     : NATURAL :=  2;
       * CONSTANT c_mep_pid_diag    : NATURAL :=  3;
       * CONSTANT c_mep_pid_ss      : NATURAL :=  4;
       * CONSTANT c_mep_pid_bf      : NATURAL :=  5;
       * CONSTANT c_mep_pid_bst     : NATURAL :=  6;
       * CONSTANT c_mep_pid_sst     : NATURAL :=  7;
       * CONSTANT c_mep_pid_rcuh    : NATURAL :=  8;
       * CONSTANT c_mep_pid_cr      : NATURAL :=  9;
       * CONSTANT c_mep_pid_xst     : NATURAL := 10;
       * CONSTANT c_mep_pid_cdo     : NATURAL := 11;
       * CONSTANT c_mep_pid_bs      : NATURAL := 12;
       * CONSTANT c_mep_pid_serdes  : NATURAL := 13;
       * CONSTANT c_mep_pid_tdsh    : NATURAL := 14;
       * CONSTANT c_mep_pid_tbb     : NATURAL := 15;
       * 
       */
      static const uint8 RSR       = 0x01; /* Status overview            [RSP/BLP] */
      static const uint8 RSU       = 0x02; /* FPGA remote system update. [RSP    ] */
      static const uint8 DIAG      = 0x03; /* Diagnostics                [RSP/BLP] */
      static const uint8 SS        = 0x04; /* Subband select             [    BLP] */
      static const uint8 BF        = 0x05; /* Beamformer                 [    BLP] */
      static const uint8 BST       = 0x06; /* Beamformer statistics      [RSP    ] */
      static const uint8 SST       = 0x07; /* Subband statistics         [    BLP] */
      static const uint8 RCU       = 0x08; /* RCU control                [    BLP] */
      static const uint8 CR        = 0x09; /* Clock and reset            [RSP/BLP] */
      static const uint8 XST       = 0x0A; /* Clock and reset            [RSP/BLP] */
      static const uint8 CDO       = 0x0B; /* CEP Data Output            [RSP    ] */
      static const uint8 BS        = 0x0C; /* Block Synchronization      [    BLP] */
      static const uint8 SERDES    = 0x0D; /* Reserved1                            */
      static const uint8 TDS       = 0x0E; /* Time distribution board    [RSP    ] */
      static const uint8 TBB       = 0x0F; /* Transient Buffer Board     [    BLP] */ /* Is this correct? Should it not be RSP? */

      static const int MIN_PID = RSR; /* loops over PID should be from */ 
      static const int MAX_PID = TBB; /* pid = MIN_PID; pid <= MAX_PID */
      /*@}*/

      /*@{*/
      /**
       * Register IDs
       */
      static const uint8 RSR_STATUS       = 0x00;
      static const uint8 RSR_VERSION      = 0x01;

      static const uint8 RSU_FLASHRW      = 0x01;
      static const uint8 RSU_FLASHERASE   = 0x02;
      static const uint8 RSU_RECONFIGURE  = 0x03;
      static const uint8 RSU_RESET        = 0x04;

      static const uint8 DIAG_WGX         = 0x00;
      static const uint8 DIAG_WGY         = 0x01;
      static const uint8 DIAG_WGXWAVE     = 0x02;
      static const uint8 DIAG_WGYWAVE     = 0x03;
      static const uint8 DIAG_BYPASS      = 0x04;
      static const uint8 DIAG_RESULTS     = 0x05;
      static const uint8 DIAG_SELFTEST    = 0x06;
      
      static const uint8 SS_SELECT        = 0x00;

      static const uint8 BF_XROUT         = 0x00;
      static const uint8 BF_XIOUT         = 0x01;
      static const uint8 BF_YROUT         = 0x02;
      static const uint8 BF_YIOUT         = 0x03;

      static const uint8 BST_POWER        = 0x00;

      static const uint8 SST_POWER        = 0x00;

      static const uint8 RCU_SETTINGS     = 0x00;
      static const uint8 RCU_PROTOCOLX    = 0x01;
      static const uint8 RCU_RESULTSX     = 0x02;
      static const uint8 RCU_PROTOCOLY    = 0x03;
      static const uint8 RCU_RESULTSY     = 0x04;

      static const uint8 CR_CONTROL       = 0x00;

      static const uint8 XST_STATS        = 0x00;
      static const uint8 XST_0_X          = XST_STATS + 0; // 8 registers, two for each AP
      static const uint8 XST_0_Y          = XST_STATS + 1;
      static const uint8 XST_1_X          = XST_STATS + 2;
      static const uint8 XST_1_Y          = XST_STATS + 3;
      static const uint8 XST_2_X          = XST_STATS + 4;
      static const uint8 XST_2_Y          = XST_STATS + 5;
      static const uint8 XST_3_X          = XST_STATS + 6;
      static const uint8 XST_3_Y          = XST_STATS + 7;
      static const uint8 XST_NR_STATS     = XST_3_Y + 1;

      /**
       * The CDO register will be extended to 
       * allow setting the UDP/IP header and some
       * settings to set interleaving of beamlets.
       */
      static const uint8 CDO_SETTINGS  = 0x00;
      static const uint8 CDO_HEADER    = 0x01;

      static const uint8 BS_NOF_SAMPLES_PER_SYNC = 0x00;

      static const uint8 TDS_PROTOCOL            = 0x00;
      static const uint8 TDS_RESULTS             = 0x01;

      /**
       * Placeholder register for future TBB control via the RSP board.
       */
      static const uint8 TBB_CONTROL      = 0x00;

      static const int MAX_REGID          = XST_3_Y; // XST_3_Y

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
      static const uint16 N_SUBBANDS     = 512;
      static const uint16 N_GLOBAL_XLETS = 54;  // Total number of crosslets over whole station (FTS-1.5 spec)
      static const uint16 N_XLETS        = 4;   // Number of crosslets per RSP board
      static const uint16 N_BEAMLETS     = 54;  // FTS-1.5 spec, final design will have > 200 BEAMLETS + N_XLETS
      static const uint16 N_XBLETS       = N_XLETS + N_BEAMLETS; // RSP registers combine XLETS and BEAMLETS
      static const uint16 N_POL          = 2;                    // number of polarizations
      static const uint16 N_PHASE        = 2;                    // number of phases in a complex number
      static const uint16 N_PHASEPOL     = N_PHASE * N_POL;      // number of phase polarizations
      static const uint16 XLET_SIZE      = N_POL * sizeof(std::complex<uint32>);
 
      //
      // Registers too large to send in a single ethernet frame
      // (> 1500 bytes) will be sent in a number of fragments of this size.
      //
      static const uint16 FRAGMENT_SIZE = 1024;
    
      //
      // XST register will be too large to get in one message when N_GLOBAL_XLETS = 96
      // When 96 crosslets are supported, the XST_FRAGMENT_SIZE should be divided by 2.
      //
      static const uint16 MAX_XLETS_PER_FRAGMENT = FRAGMENT_SIZE / XLET_SIZE;
      static const uint16 XST_FRAGMENT_SIZE = MIN(N_GLOBAL_XLETS, MAX_XLETS_PER_FRAGMENT) * XLET_SIZE;
      /*@}*/

      
      /*@{*/
      /**
       * Define size of each register.
       */
      static const uint16 RSR_STATUS_SIZE       = 164;
      static const uint16 RSR_VERSION_SIZE      = 3;
      
      static const uint16 RSU_FLASHRW_SIZE      = 1024;
      static const uint16 RSU_FLASHERASE_SIZE   = 1;
      static const uint16 RSU_RECONFIGURE_SIZE  = 1;
      static const uint16 RSU_RESET_SIZE        = 1;

      static const uint16 DIAG_WGX_SIZE         = 12;
      static const uint16 DIAG_WGY_SIZE         = 12;
      static const uint16 DIAG_WGXWAVE_SIZE     = 1024;
      static const uint16 DIAG_WGYWAVE_SIZE     = 1024;
      static const uint16 DIAG_BYPASS_SIZE      = 1;
      static const uint16 DIAG_RESULTS_SIZE     = 4096;
      static const uint16 DIAG_SELFTEST_SIZE    = 4;
      
      static const uint16 SS_SELECT_SIZE        = N_XBLETS * N_POL * sizeof(uint16);

      static const uint16 BF_XROUT_SIZE         = N_XBLETS * N_PHASEPOL * sizeof(int16);
      static const uint16 BF_XIOUT_SIZE         = N_XBLETS * N_PHASEPOL * sizeof(int16);
      static const uint16 BF_YROUT_SIZE         = N_XBLETS * N_PHASEPOL * sizeof(int16);
      static const uint16 BF_YIOUT_SIZE         = N_XBLETS * N_PHASEPOL * sizeof(int16);

      static const uint16 BST_POWER_SIZE        = N_BEAMLETS * N_POL * sizeof(uint32); // TODO: should this be N_XBLETS too?

      static const uint16 SST_POWER_SIZE        = N_SUBBANDS * N_POL * sizeof(uint32);

      static const uint16 RCU_SETTINGS_SIZE     = 3;
      static const uint16 RCU_PROTOCOLX_SIZE    = 128;
      static const uint16 RCU_RESULTSX_SIZE     = 128;
      static const uint16 RCU_PROTOCOLY_SIZE    = 128;
      static const uint16 RCU_RESULTSY_SIZE     = 128;

      static const uint16 CR_CONTROL_SIZE       = 1;

      static const uint16 XST_STATS_SIZE     = (N_GLOBAL_XLETS + N_XLETS) * XLET_SIZE;

      /**
       * The CDO register will be extended to 
       * allow setting the UDP/IP header and some
       * settings to set interleaving of beamlets.
       */
      static const uint16 CDO_SETTINGS_SIZE  = 16;
      static const uint16 CDO_HEADER_SIZE    = 32;

      static const uint16 BS_NOF_SAMPLES_PER_SYNC_SIZE     = 4;

      static const uint16 TDS_PROTOCOL_SIZE     = 128;
      static const uint16 TDS_RESULTS_SIZE      = 128;

      /**
       * Placeholder register for future TBB control via the RSP board.
       */
      static const uint16 TBB_CONTROL_SIZE      = 0;

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
       * MEP 4.x header fields
       */
      typedef struct
      {
	uint8  type;           /* Message type */
	uint8  status;         /* Error indicator */
	uint16 frame_length;   /* Frame length */
	typedef struct 
	{
	  uint16 dstid;        /* Beamlet processor and RSP ID */
	  uint8  pid;          /* Process ID */
	  uint8  regid;        /* Register ID */
	} AddrType;
	AddrType addr;         /* addr */
	uint16 offset;         /* Offset address */
	uint16 payload_length; /* Payload length (size) */
	uint16 seqnr;          /* Sequence number */
	uint16 _reserved;      /* Do not use; future use */
      } FieldsType;

      FieldsType m_fields;

    public:

      /*@{*/
      /**
       * Methods to set header fields.
       */
      void set(uint8  type,
	       uint16 dstid,
	       uint8  pid,
	       uint8  regid,
	       uint16 payload_length,
	       uint16 offset = 0);

      void set(MEPHeader::FieldsType hdrtemplate,
	       uint16 dstid          = DST_RSP,
	       uint8  type           = MEPHeader::TYPE_UNSET,
	       uint16 payload_length = 0,
	       uint16 offset         = 0);
      /*@}*/

      /**
       * Method to check if this MEPHeader is a valid
       * acknowledgement for a specific request.
       */
      bool isValidAck(const MEPHeader& reqhdr);

      /*@{*/
      //
      // The following static constants are templates
      // to produce EPA message headers. This is done by 
      // copying the template and then make modifications
      // to the appropriate parts of the header.
      //
      static const FieldsType RSR_STATUS_HDR;
      static const FieldsType RSR_VERSION_HDR;
      
      static const FieldsType RSU_FLASHRW_HDR;
      static const FieldsType RSU_FLASHERASE_HDR;
      static const FieldsType RSU_RECONFIGURE_HDR;
      static const FieldsType RSU_RESET_HDR;

      static const FieldsType DIAG_WGX_HDR;
      static const FieldsType DIAG_WGY_HDR;
      static const FieldsType DIAG_WGXWAVE_HDR;
      static const FieldsType DIAG_WGYWAVE_HDR;
      static const FieldsType DIAG_BYPASS_HDR;
      static const FieldsType DIAG_RESULTS_HDR;
      static const FieldsType DIAG_SELFTEST_HDR;
      
      static const FieldsType SS_SELECT_HDR;

      static const FieldsType BF_XROUT_HDR;
      static const FieldsType BF_XIOUT_HDR;
      static const FieldsType BF_YROUT_HDR;
      static const FieldsType BF_YIOUT_HDR;

      static const FieldsType BST_POWER_HDR;

      static const FieldsType SST_POWER_HDR;

      static const FieldsType RCU_SETTINGS_HDR;
      static const FieldsType RCU_PROTOCOLX_HDR;
      static const FieldsType RCU_RESULTSX_HDR;
      static const FieldsType RCU_PROTOCOLY_HDR;
      static const FieldsType RCU_RESULTSY_HDR;

      static const FieldsType CR_CONTROL_HDR;

      static const FieldsType XST_STATS_HDR;

      static const FieldsType CDO_SETTINGS_HDR;
      static const FieldsType CDO_HEADER_HDR;

      static const FieldsType BS_NOF_SAMPLES_PER_SYNC_HDR;

      static const FieldsType TDS_PROTOCOL_HDR;
      static const FieldsType TDS_RESULTS_HDR;

      static const FieldsType TBB_CONTROL_HDR;
      /*@}*/
    };
  };
};

#endif /* MEPHEADER_H_ */
