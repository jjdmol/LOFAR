/* packet-epa.c
 * Routines for LOFAR Embedded Processing Application protocol dissection
 * Copyright 2004, Klaas Jan Wierenga <wierenga@astron.nl>
 *
 * $Id$
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 1998 Gerald Combs
 *
 * Copied from WHATEVER_FILE_YOU_USED (where "WHATEVER_FILE_YOU_USED"
 * is a dissector file; if you just copied this from README.developer,
 * don't bother with the "Copied from" - you don't even need to put
 * in a "Copied from" if you copied an existing dissector, especially
 * if the bulk of the code in the new dissector is your code)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#ifdef NEED_SNPRINTF_H
# include "snprintf.h"
#endif

#include <epan/packet.h>
#include "packet-epa.h"

#define FRAGMENT_SIZE_BYTES  1024
#define SS_SELECT_SIZE_BYTES 512

/*@{*/
/**
 * Message types.
 */
#define TYPE_UNSET 0x00
#define READ       0x01
#define WRITE      0x02
#define READACK    0x03
#define WRITEACK   0x04

#define MAX_TYPE 0x04 /* counting from 0 */
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
#define DST_BLP            0x00 /* BLP's are addressed starting from 0x00 */
#define DST_RSP            0x80 /* Destination id of the RSP board */
#define DST_BROADCAST_BLPS 0x7F /* Broadcast to all BLP's but not the RSP */
#define DST_BROADCAST      0xFF /* Broadcast to RSP and all BLP's */
/*@}*/

/*@{*/
/**
 * Process IDs
 */
#define RSR     0x00 /* Status overview */
#define TST     0x01 /* Selftest functionality */
#define CFG     0x02 /* FPGA configuration and reset */
#define WG      0x03 /* Waveform generator */
#define SS      0x04 /* Subband select */
#define BF      0x05 /* Beamformer */
#define BST     0x06 /* Beamformer statistics */
#define SST     0x07 /* Subband statistics */
#define RCU     0x08 /* RCU control */
#define CRR     0x09 /* RSP clock and reset */
#define CRB     0x0A /* BLP clock and reset */
#define CDO     0x0B /* CEP Data Output */

#define MAX_PID CDO /* counting from 0 */
/*@}*/

/*@{*/
/**
 * Register IDs
 */
#define RSR_STATUS    0x00
#define RSR_VERSION   0x01

#define TST_SELFTEST  0x00

#define CFG_RESET     0x00
#define CFG_REPROGRAM 0x01

#define WG_XSETTINGS  0x00
#define WG_YSETTINGS  0x01
#define WG_XWAVE      0x02
#define WG_YWAVE      0x03

#define SS_SELECT     0x00

#define BF_XROUT      0x00
#define BF_XIOUT      0x01
#define BF_YROUT      0x02
#define BF_YIOUT      0x03

#define BST_POWER     0x00

#define SST_POWER     0x00

#define RCU_SETTINGS  0x00

#define CRR_SOFTRESET 0x00
#define CRR_SOFTPPS   0x01

#define CRB_SOFTRESET 0x00
#define CRB_SOFTPPS   0x01

#define CDO_SETTINGS  0x00

#define MAX_REGID 0x03

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
#define N_SUBBANDS 512
#define N_BEAMLETS 256 //128; // FTS-1 spec, final remote station will be 256
#define N_POL      2                // number of polarizations
#define N_PHASE    2                // number of phases in a complex number
#define N_PHASEPOL (N_PHASE * N_POL)  // number of phase polarizations

//
// Registers too large to send in a single ethernet frame
// (> 1500 bytes) will be sent in a number of fragments of this size.
//
#define FRAGMENT_SIZE 1024

/**
 * Read/write sizes in octets (= bytes)
 */
#define RSR_STATUS_SIZE    96
#define RSR_VERSION_SIZE   3

#define TST_SELFTEST_SIZE  1

#define CFG_RESET_SIZE     1
#define CFG_REPROGRAM_SIZE 1

#define WG_XSETTINGS_SIZE  7
#define WG_YSETTINGS_SIZE  7
#define WG_XWAVE_SIZE      1024
#define WG_YWAVE_SIZE      1024

#define SS_SELECT_SIZE     (N_BEAMLETS * N_POL * sizeof(uint16))

#define BF_XROUT_SIZE      (N_BEAMLETS * N_PHASEPOL * sizeof(int16))
#define BF_XIOUT_SIZE      (N_BEAMLETS * N_PHASEPOL * sizeof(int16))
#define BF_YROUT_SIZE      (N_BEAMLETS * N_PHASEPOL * sizeof(int16))
#define BF_YIOUT_SIZE      (N_BEAMLETS * N_PHASEPOL * sizeof(int16))

#define BST_POWER_SIZE     (N_BEAMLETS * N_POL * sizeof(uint32))

#define SST_POWER_SIZE     (N_SUBBANDS * N_POL * sizeof(uint32))

#define RCU_SETTINGS_SIZE  2

#define CRR_SOFTRESET_SIZE 1
#define CRR_SOFTPPS_SIZE   1

#define CRB_SOFTRESET_SIZE 1
#define CRB_SOFTPPS_SIZE   1

#define CDO_SETTINGS_SIZE  10
/*@}*/

/**
 * EPA protocol constants and value to string mappings.
 */
static const value_string type_info_vals[] =
{
  { TYPE_UNSET, "Invalid " },
  { READ,       "READ    " },
  { WRITE,      "WRITE   " },
  { READACK,    "READACK " },
  { WRITEACK,   "WRITEACK" },
  { 0,     NULL                   },
};

static const value_string type_vals[] =
{
  { TYPE_UNSET, "Invalid message type"         },
  { READ,       "Read  request     (READ)"     },
  { WRITE,      "Write command     (WRITE)"    },
  { READACK,    "Read  acknowledge (READACK)"  },
  { WRITEACK,   "Write acknowledge (WRITEACK)" },
  { 0,     NULL                   },
};

static const value_string dst_vals[] =
{
  { DST_BLP,     "Beamlet processor 0" },
  { DST_BLP + 1, "Beamlet processor 1" },
  { DST_BLP + 2, "Beamlet processor 2" },
  { DST_BLP + 3, "Beamlet processor 3" },
  { DST_RSP,     "RSP main FPGA"     },
  { 0,     NULL               },  
};

static const value_string pid_info_vals[] =
{
  { RSR, "RSR" },
  { TST, "TST" },
  { CFG, "CFG" },
  { WG,  "WG " },
  { SS,  "SS " },
  { BF,  "BF " },
  { BST, "BST" },
  { SST, "SST" },
  { RCU, "RCU" },
  { CRR, "CRR" },
  { CRB, "CRB" },
  { CDO, "CDO" },
  { 0,   NULL  },
};

static const value_string pid_vals[] =
{
  { RSR, "Status overview (RSR)"              },
  { TST, "Selftest functionality (TST)"       },
  { CFG, "FPGA configuration and reset (CFG)" },
  { WG,  "Waveform generator (WG)"            },
  { SS,  "Subband select (SS)"                },
  { BF,  "Beamformer (BF)"                    },
  { BST, "Beamlet statistics (BST)"           },
  { SST, "Subband statistics (SST)"           },
  { RCU, "RCU Control (RCU)"                  },
  { CRR, "RSP Clock and Reset",               },
  { CRB, "BLP Clock and Reset",               },
  { CDO, "CEP Data Output",                   },
  { 0,     NULL                                },
};

static const value_string status_vals[] =
{
  { RSR_STATUS,  "RSP Status" },
  { RSR_VERSION, "Version"    },
  { 0,     NULL        },
};

static const value_string tst_vals[] =
{
  { TST_SELFTEST, "Selftest" },
  { 0,     NULL        },
};

static const value_string cfg_vals[] =
{
  { CFG_RESET,     "Reset"     },
  { CFG_REPROGRAM, "Reprogram" },
  { 0,     NULL       },
};

static const value_string wg_vals[] =
{
  { WG_XSETTINGS, "Waveform generator settings X polarization" },
  { WG_YSETTINGS, "Waveform generator settings Y polarization" },
  { WG_XWAVE,     "User waveform X polarization"               },
  { WG_YWAVE,     "User waveform Y polarization"               },
  { 0,     NULL                         },
};

static const value_string ss_vals[] =
{
  { SS_SELECT, "Subband Select parameters"   },
  { 0,     NULL                         },
};

static const value_string bf_vals[] =
{
  { BF_XROUT, "XR,XI,YR,YI coefficients for XR output" },
  { BF_XIOUT, "XR,XI,YR,YI coefficients for XI output" },
  { BF_YROUT, "XR,XI,YR,YI coefficients for YR output" },
  { BF_YIOUT, "XR,XI,YR,YI coefficients for YI output" },
  { 0,     NULL       },
};

static const value_string bst_vals[] =
{
  { BST_POWER, "Beamlet Statistics - X,Y Power"  },
  { 0,     NULL   },
};

static const value_string sst_vals[] =
{
  { SST_POWER, "Subband Statistics - X,Y Power"  },
  { 0,     NULL   },
};

static const value_string rcu_vals[] =
{
  { RCU_SETTINGS, "RCU Settings"  },
  { 0,     NULL   },
};

static const value_string crr_vals[] =
{
  { CRR_SOFTRESET, "Soft Reset"  },
  { CRR_SOFTPPS,   "Soft PPS"  },
  { 0,     NULL   },
};

static const value_string crb_vals[] =
{
  { CRB_SOFTRESET, "Soft Reset"  },
  { CRB_SOFTPPS,   "Soft PPS"  },
  { 0,     NULL   },
};

static const value_string cdo_vals[] =
{
  { CDO_SETTINGS, "CEP Data Output Settings"  },
  { 0,     NULL   },
};

static const value_string eth_error_vals[] =
{
  { 0, "The ethernet frame was received correctly"           },
  { 1, "Preamble had other value than 0xAA"                  },
  { 2, "Frame delimiter had other value than 0xAB"           },
  { 3, "Not enough preamble nibbles"                         },
  { 4, "Frame ended during frame header."                    },
  { 5, "Calculated CRC does not match received CRC"          },
  { 6, "An odd number of nibbles was received from ethernet" },
  { 7, "Length specified in the frame size field does not match the real number of received bytes" },
  { 0,     NULL   },
};

static const value_string mep_error_vals[] =
{
  { 0, "The MEP message was processed successfully"          },
  { 1, "Unknown message type"                                },
  { 2, "DSTID is too large"                                  },
  { 3, "Invalid PID"                                         },
  { 4, "Register does not exist"                             },
  { 5, "Message is too large"                                },
  { 6, "Error occurred during inter-FPGA transmission."      },
  { 0, NULL },
};

static const value_string wg_mode_vals[] =
{
  { 0, "off" },
  { 1, "calc" },
  { 3, "single" },
  { 5, "repeat" },
  { 0, NULL },
};


/**
 * Pluginize BEGIN
 *
 * Pluginize the EPA dissector
 */
#include "plugins/plugin_api.h"
#include "moduleinfo.h"
#include <gmodule.h>
#include "plugins/plugin_api_defs.h"

#ifndef __ETHEREAL_STATIC__
G_MODULE_EXPORT const gchar version[] = "3.0";
G_MODULE_EXPORT void plugin_init(plugin_address_table_t *pat);
G_MODULE_EXPORT void plugin_reg_handoff(void);
#endif 

/**
 * Pluginize END
 */

/**
 * Constants
 */
#define ETHERTYPE_EPA 0x10FA

/* Initialize the protocol and registered fields */
static int proto_epa          = -1;
static int hf_epa_type        = -1;
static int hf_epa_error       = -1;
static int hf_epa_seqnr       = -1;
static int hf_epa_addr        = -1;
static int hf_epa_addr_dstid  = -1;
static int hf_epa_addr_pid    = -1;
static int hf_epa_addr_regid  = -1;
static int hf_epa_addr_ffi    = -1;
static int hf_epa_offset      = -1;
static int hf_epa_size        = -1;
static int hf_epa_data        = -1;
static int hf_epa_int16       = -1;
static int hf_epa_uint16      = -1;
static int hf_epa_int32       = -1;
static int hf_epa_uint32      = -1;
static int hf_epa_double      = -1;

/**
 * RSP Status register fields.
 */
/*static int df_rspstatus             = -1;*/
static int df_rspstatus_voltage_15  = -1;
static int df_rspstatus_voltage_33  = -1;
/*static int df_fpgastatus            = -1;*/
static int df_fpgastatus_bp_status  = -1;
static int df_fpgastatus_bp_temp    = -1;
static int df_fpgastatus_ap1_status = -1;
static int df_fpgastatus_ap1_temp   = -1;
static int df_fpgastatus_ap2_status = -1;
static int df_fpgastatus_ap2_temp   = -1;
static int df_fpgastatus_ap3_status = -1;
static int df_fpgastatus_ap3_temp   = -1;
static int df_fpgastatus_ap4_status = -1;
static int df_fpgastatus_ap4_temp   = -1;
/*static int df_ethstatus             = -1;*/
static int df_ethstatus_nof_frames  = -1;
static int df_ethstatus_nof_errors  = -1;
static int df_ethstatus_last_error  = -1;
/*static int df_mepstatus             = -1;*/
static int df_mepstatus_seqnr  = -1;
static int df_mepstatus_error  = -1;
/*static int df_syncstatus = -1; */
static int df_ap1_sync_sample_count = -1;
static int df_ap1_sync_sync_count   = -1;
static int df_ap1_sync_error_count  = -1;
static int df_ap2_sync_sample_count = -1;
static int df_ap2_sync_sync_count   = -1;
static int df_ap2_sync_error_count  = -1;
static int df_ap3_sync_sample_count = -1;
static int df_ap3_sync_sync_count   = -1;
static int df_ap3_sync_error_count  = -1;
static int df_ap4_sync_sample_count = -1;
static int df_ap4_sync_sync_count   = -1;
static int df_ap4_sync_error_count  = -1;

/**
 * RSP Version register fields.
 */
static int df_rsp_version = -1;
static int df_bp_version  = -1;
static int df_ap_version  = -1;

/**
 * RCU Settings register fields
 */
static int df_vddvcc_en    = -1;
static int df_vh_enable    = -1;
static int df_vl_enable    = -1;
static int df_filsel_b     = -1;
static int df_filsel_a     = -1;
static int df_bandsel      = -1;
static int df_hba_enable   = -1;
static int df_lba_enable   = -1;
static int df_nof_overflow = -1;

/**
 * WG settings
 */
static int df_wg_freq  = -1;
static int df_wg_phase = -1;
static int df_wg_ampl  = -1;
static int df_wg_nof_samples = -1;
static int df_wg_mode = -1;

/* Initialize the subtree pointers */
static gint ett_epa         = -1;
static gint ett_epa_addr    = -1;
static gint ett_rspstatus   = -1;
static gint ett_rspstatus_detail = -1;
static gint ett_fpgastatus  = -1;
static gint ett_ethstatus   = -1;
static gint ett_mepstatus   = -1;
static gint ett_syncstatus  = -1;
static gint ett_syncvalues  = -1;
static gint ett_rcustatus   = -1;
static gint ett_rcusettings = -1;
static gint ett_rspversion  = -1;
static gint ett_wgsettings  = -1;
static gint ett_payload     = -1;

/* Code to actually dissect the packets */
static void
dissect_epa(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{

  /* Set up structures needed to add the protocol subtree and manage it */
  proto_item *ti;
  proto_tree *epa_tree;
  char*  typestr = NULL;
  char*  pidstr  = NULL;
  char*  regstr  = NULL;
  guint8 type = tvb_get_guint8(tvb, 0);
  guint8 reg  = tvb_get_guint8(tvb, 6);
  guint8 pid  = tvb_get_guint8(tvb, 5);

#if 0
  /* don't know enough about how this conversation stuff works, implement it later */
  conversation_t* conversation = NULL;

  conversation = find_conversation(&pinfo->net_src, &pinfo->net_dst, pinfo->ptype,
				   0, 0, NO_PORT2);
  if (conversation == NULL)
  {
    /* No conversation, create one */
    conversation = conversation_new(&pinfo->net_src, &pinfo->net_dst, pinfo->ptype,
				    0, 0, NO_PORT_B);
  }
#endif
  
  /* Make entries in Protocol column and Info column on summary display */
  if (check_col(pinfo->cinfo, COL_PROTOCOL)) 
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "EPA");
    
  /* This field shows up as the "Info" column in the display; you should make
     it, if possible, summarize what's in the packet, so that a user looking
     at the list of packets can tell what type of packet it is. See section 1.5
     for more information.

     If you are setting it to a constant string, use "col_set_str()", as
     it's more efficient than the other "col_set_XXX()" calls.

     If you're setting it to a string you've constructed, or will be
     appending to the column later, use "col_add_str()".

     "col_add_fstr()" can be used instead of "col_add_str()"; it takes
     "printf()"-like arguments.  Don't use "col_add_fstr()" with a format
     string of "%s" - just use "col_add_str()" or "col_set_str()", as it's
     more efficient than "col_add_fstr()".

     If you will be fetching any data from the packet before filling in
     the Info column, clear that column first, in case the calls to fetch
     data from the packet throw an exception because they're fetching data
     past the end of the packet, so that the Info column doesn't have data
     left over from the previous dissector; do

     if (check_col(pinfo->cinfo, COL_INFO)) 
     col_clear(pinfo->cinfo, COL_INFO);

  */

  if (check_col(pinfo->cinfo, COL_INFO)) 
    col_clear(pinfo->cinfo, COL_INFO);

  /* decode the typestr */
  typestr = type_info_vals[type].strptr;
  pidstr  = pid_info_vals[pid].strptr;
  switch (pid)
  {
    case RSR:
      regstr = status_vals[reg].strptr;
      break;

    case TST:
      regstr = tst_vals[reg].strptr;
      break;

    case CFG:
      regstr = cfg_vals[reg].strptr;
      break;

    case WG:
      regstr = wg_vals[reg].strptr;
      break;

    case SS:
      regstr = ss_vals[reg].strptr;
      break;

    case BF:
      regstr = bf_vals[reg].strptr;
      break;

    case BST:
      regstr = bst_vals[reg].strptr;
      break;

    case SST:
      regstr = sst_vals[reg].strptr;
      break;

    case RCU:
      regstr = rcu_vals[reg].strptr;
      break;

    case CRR:
      regstr = crr_vals[reg].strptr;
      break;

    case CRB:
      regstr = crb_vals[reg].strptr;
      break;

    case CDO:
      regstr = cdo_vals[reg].strptr;
      break;
  }

  if (!typestr) typestr = "Unknown type?";
  if (!pidstr)  pidstr  = "Uknown process?";
  if (!regstr)  regstr  = "Unknown register?";
      
  /* fill the INFO column */
  if (check_col(pinfo->cinfo, COL_INFO))
    col_add_fstr(pinfo->cinfo, COL_INFO, "%s %s %s", typestr, pidstr, regstr);
  
  /* In the interest of speed, if "tree" is NULL, don't do any work not
     necessary to generate protocol tree items. */
  if (tree)
  {
    proto_item* newitem = 0;
    proto_tree* newtree = 0;
    proto_tree* subtree = 0;
    proto_tree* synctree = 0;
    proto_tree* rcustatus_tree = 0;

    /* NOTE: The offset and length values in the call to
       "proto_tree_add_item()" define what data bytes to highlight in the hex
       display window when the line in the protocol tree display
       corresponding to that item is selected.

       Supplying a length of -1 is the way to highlight all data from the
       offset to the end of the packet. */

    /* create display subtree for the protocol */
    ti = proto_tree_add_item(tree, proto_epa, tvb, 0, -1, FALSE);
    
    epa_tree = proto_item_add_subtree(ti, ett_epa);

    /* add an item to the subtree, see section 1.6 for more information */
    
    /* Continue adding tree items to process the packet here */
    proto_tree_add_item(epa_tree, hf_epa_type,        tvb,  0,  1, FALSE);
    proto_tree_add_item(epa_tree, hf_epa_error,       tvb,  1,  1, FALSE);
    proto_tree_add_item(epa_tree, hf_epa_seqnr,       tvb,  2,  2, TRUE);

    newitem = proto_tree_add_item(epa_tree, hf_epa_addr, tvb,  4,  4, FALSE);
    newtree = proto_item_add_subtree(newitem, ett_epa_addr);

    proto_tree_add_item(newtree, hf_epa_addr_dstid,  tvb,  4,  1, FALSE);
    proto_tree_add_item(newtree, hf_epa_addr_pid,    tvb,  5,  1, FALSE);

    if (regstr)
      proto_tree_add_string(newtree, hf_epa_addr_regid, tvb, 6, 1, regstr);
    else
      proto_tree_add_item(newtree, hf_epa_addr_regid,  tvb,  6,  1, FALSE);

    proto_tree_add_item(newtree, hf_epa_addr_ffi, tvb,  7,  1, FALSE);

    proto_tree_add_item(epa_tree, hf_epa_offset,      tvb,  8,  2, TRUE);
    proto_tree_add_item(epa_tree, hf_epa_size,        tvb, 10,  2, TRUE);

    if (READACK == type && RSR == pid && RSR_STATUS == reg)
    {
      // READACK RSR_STATUS
      newitem = proto_tree_add_text(epa_tree, tvb, 12, RSR_STATUS_SIZE, "RSP Status register");
      newtree = proto_item_add_subtree(newitem, ett_rspstatus);

      newitem = proto_tree_add_text(newtree, tvb, 12, 4,  "RSP Status");
      subtree = proto_item_add_subtree(newitem, ett_rspstatus_detail);
      proto_tree_add_item(subtree, df_rspstatus_voltage_15  ,tvb, 12, 1,  FALSE);
      proto_tree_add_item(subtree, df_rspstatus_voltage_33  ,tvb, 13, 1,  FALSE);

      newitem = proto_tree_add_text(newtree, tvb,  16, 12, "FPGA Status");
      subtree = proto_item_add_subtree(newitem, ett_fpgastatus);
      proto_tree_add_item(subtree, df_fpgastatus_bp_status  ,tvb, 16, 1,  FALSE);
      proto_tree_add_item(subtree, df_fpgastatus_bp_temp    ,tvb, 17, 1,  FALSE);
      proto_tree_add_item(subtree, df_fpgastatus_ap1_status ,tvb, 18, 1,  FALSE);
      proto_tree_add_item(subtree, df_fpgastatus_ap1_temp   ,tvb, 19, 1,  FALSE);
      proto_tree_add_item(subtree, df_fpgastatus_ap2_status ,tvb, 20, 1,  FALSE);
      proto_tree_add_item(subtree, df_fpgastatus_ap2_temp   ,tvb, 21, 1,  FALSE);   
      proto_tree_add_item(subtree, df_fpgastatus_ap3_status ,tvb, 22, 1,  FALSE);
      proto_tree_add_item(subtree, df_fpgastatus_ap3_temp   ,tvb, 23, 1,  FALSE);
      proto_tree_add_item(subtree, df_fpgastatus_ap4_status ,tvb, 24, 1,  FALSE);
      proto_tree_add_item(subtree, df_fpgastatus_ap4_temp   ,tvb, 25, 1,  FALSE);   

      newitem = proto_tree_add_text(newtree, tvb, 28, 12, "ETH Status");
      subtree = proto_item_add_subtree(newitem, ett_ethstatus);
      proto_tree_add_item(subtree, df_ethstatus_nof_frames  ,tvb, 28, 4,  TRUE);
      proto_tree_add_item(subtree, df_ethstatus_nof_errors  ,tvb, 32, 4,  TRUE);
      proto_tree_add_item(subtree, df_ethstatus_last_error  ,tvb, 36, 1,  FALSE);

      newitem = proto_tree_add_text(newtree, tvb, 40, 4, "MEP Status");
      subtree = proto_item_add_subtree(newitem, ett_mepstatus);
      proto_tree_add_item(subtree, df_mepstatus_seqnr  ,tvb, 40, 2,  TRUE);
      proto_tree_add_item(subtree, df_mepstatus_error  ,tvb, 42, 1,  FALSE);
      
      newitem = proto_tree_add_text(newtree, tvb, 44, 12, "SYNC Status");
      synctree = proto_item_add_subtree(newitem, ett_syncstatus);
      {
	/* AP1 sync status */
	newitem = proto_tree_add_text(synctree, tvb, 44, 12, "AP1 sync status");
	subtree = proto_item_add_subtree(newitem, ett_syncvalues);
	proto_tree_add_item(subtree, df_ap1_sync_sample_count ,tvb, 44, 4, TRUE);
	proto_tree_add_item(subtree, df_ap1_sync_sync_count   ,tvb, 48, 4, TRUE);
	proto_tree_add_item(subtree, df_ap1_sync_error_count  ,tvb, 52, 4, TRUE);
      }
      {
	/* AP2 sync status */
	newitem = proto_tree_add_text(synctree, tvb, 56, 12, "AP2 sync status");
	subtree = proto_item_add_subtree(newitem, ett_syncvalues);
	proto_tree_add_item(subtree, df_ap2_sync_sample_count ,tvb, 56, 4, TRUE);
	proto_tree_add_item(subtree, df_ap2_sync_sync_count   ,tvb, 60, 4, TRUE);
	proto_tree_add_item(subtree, df_ap2_sync_error_count  ,tvb, 64, 4, TRUE);
      }
      {
	/* AP3 sync status */
	newitem = proto_tree_add_text(synctree, tvb, 68, 12, "AP3 sync status");
	subtree = proto_item_add_subtree(newitem, ett_syncvalues);
	proto_tree_add_item(subtree, df_ap3_sync_sample_count ,tvb, 68, 4, TRUE);
	proto_tree_add_item(subtree, df_ap3_sync_sync_count   ,tvb, 72, 4, TRUE);
	proto_tree_add_item(subtree, df_ap3_sync_error_count  ,tvb, 76, 4, TRUE);
      }
      {
	/* AP4 sync status */
	newitem = proto_tree_add_text(synctree, tvb, 80, 12, "AP4 sync status");
	subtree = proto_item_add_subtree(newitem, ett_syncvalues);
	proto_tree_add_item(subtree, df_ap4_sync_sample_count ,tvb, 80, 4, TRUE);
	proto_tree_add_item(subtree, df_ap4_sync_sync_count   ,tvb, 84, 4, TRUE);
	proto_tree_add_item(subtree, df_ap4_sync_error_count  ,tvb, 88, 4, TRUE);
      }

      newitem = proto_tree_add_text(newtree, tvb, 92, 48, "RCU Status");
      rcustatus_tree = subtree = proto_item_add_subtree(newitem, ett_rcustatus);

      newitem = proto_tree_add_text(rcustatus_tree, tvb, 92, 12, "AP1 RCU status");
      newtree = proto_item_add_subtree(newitem, ett_rcusettings);
      {
	/* AP1 X-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 92, 12, "X polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree, df_vddvcc_en,  tvb, 92, 1, FALSE);
	proto_tree_add_item(subtree, df_vh_enable,  tvb, 92, 1, FALSE);
	proto_tree_add_item(subtree, df_vl_enable,  tvb, 92, 1, FALSE);
	proto_tree_add_item(subtree, df_filsel_b,   tvb, 92, 1, FALSE);
	proto_tree_add_item(subtree, df_filsel_a,   tvb, 92, 1, FALSE);
	proto_tree_add_item(subtree, df_bandsel,    tvb, 92, 1, FALSE);
	proto_tree_add_item(subtree, df_hba_enable, tvb, 92, 1, FALSE);
	proto_tree_add_item(subtree, df_lba_enable, tvb, 92, 1, FALSE);
	proto_tree_add_item(subtree, df_nof_overflow, tvb, 98, 4, TRUE);
      }
      {
	/* AP1 Y-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 92, 12, "Y polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree, df_vddvcc_en,  tvb, 93, 1, FALSE);
	proto_tree_add_item(subtree, df_vh_enable,  tvb, 93, 1, FALSE);
	proto_tree_add_item(subtree, df_vl_enable,  tvb, 93, 1, FALSE);
	proto_tree_add_item(subtree, df_filsel_b,   tvb, 93, 1, FALSE);
	proto_tree_add_item(subtree, df_filsel_a,   tvb, 93, 1, FALSE);
	proto_tree_add_item(subtree, df_bandsel,    tvb, 93, 1, FALSE);
	proto_tree_add_item(subtree, df_hba_enable, tvb, 93, 1, FALSE);
	proto_tree_add_item(subtree, df_lba_enable, tvb, 93, 1, FALSE);
	proto_tree_add_item(subtree, df_nof_overflow, tvb, 100, 4, TRUE);
      }
      
      newitem = proto_tree_add_text(rcustatus_tree, tvb, 104, 12, "AP2 RCU status");
      newtree = proto_item_add_subtree(newitem, ett_rcusettings);
      {
	/* AP2 X-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 104, 12, "X polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree,df_vddvcc_en,  tvb, 104, 1, FALSE);
	proto_tree_add_item(subtree,df_vh_enable,  tvb, 104, 1, FALSE);
	proto_tree_add_item(subtree,df_vl_enable,  tvb, 104, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_b,   tvb, 104, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_a,   tvb, 104, 1, FALSE);
	proto_tree_add_item(subtree,df_bandsel,    tvb, 104, 1, FALSE);
	proto_tree_add_item(subtree,df_hba_enable, tvb, 104, 1, FALSE);
	proto_tree_add_item(subtree,df_lba_enable, tvb, 104, 1, FALSE);
	proto_tree_add_item(subtree, df_nof_overflow, tvb, 108, 4, TRUE);
      }
      {
	/* AP2 Y-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 104, 12, "Y polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree,df_vddvcc_en,  tvb, 105, 1, FALSE);
	proto_tree_add_item(subtree,df_vh_enable,  tvb, 105, 1, FALSE);
	proto_tree_add_item(subtree,df_vl_enable,  tvb, 105, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_b,   tvb, 105, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_a,   tvb, 105, 1, FALSE);
	proto_tree_add_item(subtree,df_bandsel,    tvb, 105, 1, FALSE);
	proto_tree_add_item(subtree,df_hba_enable, tvb, 105, 1, FALSE);
	proto_tree_add_item(subtree,df_lba_enable, tvb, 105, 1, FALSE);
	proto_tree_add_item(subtree, df_nof_overflow, tvb, 112, 4, TRUE);
      }
      
      newitem = proto_tree_add_text(rcustatus_tree, tvb, 116, 12, "AP3 RCU status");
      newtree = proto_item_add_subtree(newitem, ett_rcusettings);
      {
	/* AP3 X-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 116, 12, "X polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree,df_vddvcc_en,  tvb, 116, 1, FALSE);
	proto_tree_add_item(subtree,df_vh_enable,  tvb, 116, 1, FALSE);
	proto_tree_add_item(subtree,df_vl_enable,  tvb, 116, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_b,   tvb, 116, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_a,   tvb, 116, 1, FALSE);
	proto_tree_add_item(subtree,df_bandsel,    tvb, 116, 1, FALSE);
	proto_tree_add_item(subtree,df_hba_enable, tvb, 116, 1, FALSE);
	proto_tree_add_item(subtree,df_lba_enable, tvb, 116, 1, FALSE);
	proto_tree_add_item(subtree, df_nof_overflow, tvb, 120, 4, TRUE);
      }
      {
	/* AP3 Y-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 116, 12, "Y polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree,df_vddvcc_en,  tvb, 117, 1, FALSE);
	proto_tree_add_item(subtree,df_vh_enable,  tvb, 117, 1, FALSE);
	proto_tree_add_item(subtree,df_vl_enable,  tvb, 117, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_b,   tvb, 117, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_a,   tvb, 117, 1, FALSE);
	proto_tree_add_item(subtree,df_bandsel,    tvb, 117, 1, FALSE);
	proto_tree_add_item(subtree,df_hba_enable, tvb, 117, 1, FALSE);
	proto_tree_add_item(subtree,df_lba_enable, tvb, 117, 1, FALSE);
	proto_tree_add_item(subtree, df_nof_overflow, tvb, 124, 4, TRUE);
      }
      
      newitem = proto_tree_add_text(rcustatus_tree, tvb, 128, 12, "AP4 RCU status");
      newtree = proto_item_add_subtree(newitem, ett_rcusettings);
      {
	/* AP4 X-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 128, 12, "Y polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree,df_vddvcc_en,  tvb, 128, 1, FALSE);
	proto_tree_add_item(subtree,df_vh_enable,  tvb, 128, 1, FALSE);
	proto_tree_add_item(subtree,df_vl_enable,  tvb, 128, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_b,   tvb, 128, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_a,   tvb, 128, 1, FALSE);
	proto_tree_add_item(subtree,df_bandsel,    tvb, 128, 1, FALSE);
	proto_tree_add_item(subtree,df_hba_enable, tvb, 128, 1, FALSE);
	proto_tree_add_item(subtree,df_lba_enable, tvb, 128, 1, FALSE);
	proto_tree_add_item(subtree, df_nof_overflow, tvb, 132, 4, TRUE);
      }
      {
	/* AP4 Y-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 128, 12, "Y polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree,df_vddvcc_en,  tvb, 129, 1, FALSE);
	proto_tree_add_item(subtree,df_vh_enable,  tvb, 129, 1, FALSE);
	proto_tree_add_item(subtree,df_vl_enable,  tvb, 129, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_b,   tvb, 129, 1, FALSE);
	proto_tree_add_item(subtree,df_filsel_a,   tvb, 129, 1, FALSE);
	proto_tree_add_item(subtree,df_bandsel,    tvb, 129, 1, FALSE);
	proto_tree_add_item(subtree,df_hba_enable, tvb, 129, 1, FALSE);
	proto_tree_add_item(subtree,df_lba_enable, tvb, 129, 1, FALSE);
	proto_tree_add_item(subtree, df_nof_overflow, tvb, 136, 4, TRUE);
      }
    }
    else if (READACK == type && RSR == pid && RSR_VERSION == reg)
    {
      // READACK RSR_VERSION
      newitem = proto_tree_add_text(epa_tree, tvb, 12, RSR_VERSION_SIZE, "RSP Version register");
      newtree = proto_item_add_subtree(newitem, ett_rspversion);

      proto_tree_add_item(newtree, df_rsp_version, tvb, 12, 1, FALSE);
      proto_tree_add_item(newtree, df_bp_version,  tvb, 13, 1, FALSE);
      proto_tree_add_item(newtree, df_ap_version,  tvb, 14, 1, FALSE);
    }
    else if ( (WRITE == type || READACK == type) && WG == pid && (WG_XSETTINGS == reg || WG_YSETTINGS == reg))
    {
      // WRITE or READACK WG_[X|Y]SETTINGS
      if (WG_XSETTINGS == reg)
	newitem = proto_tree_add_text(epa_tree, tvb, 12, WG_XSETTINGS_SIZE, "Waveform generator settings X-polarization");
      else
	newitem = proto_tree_add_text(epa_tree, tvb, 12, WG_YSETTINGS_SIZE, "Waveform generator settings Y-polarization");
      newtree = proto_item_add_subtree(newitem, ett_wgsettings);

      proto_tree_add_item(newtree, df_wg_freq,        tvb, 12, 2, TRUE);
      proto_tree_add_item(newtree, df_wg_phase,       tvb, 14, 1, FALSE);
      proto_tree_add_item(newtree, df_wg_ampl,        tvb, 15, 1, FALSE);
      proto_tree_add_item(newtree, df_wg_nof_samples, tvb, 16, 2, TRUE);
      proto_tree_add_item(newtree, df_wg_mode,        tvb, 18, 1, FALSE);
    }
    else if ( (WRITE == type || READACK == type) && WG == pid && (WG_XWAVE == reg || WG_YWAVE == reg))
    {
      // WRITE or READACK WG_[X|Y]WAVE
      if (WG_XWAVE == reg)
	newitem = proto_tree_add_text(epa_tree, tvb, 12, WG_XWAVE_SIZE, "User waveform X-polarization");
      else
	newitem = proto_tree_add_text(epa_tree, tvb, 12, WG_YWAVE_SIZE, "User Waveform Y-polarization");
      newtree = proto_item_add_subtree(newitem, ett_wgsettings);

      int i;
      for (i = 0; i < 16; i++)
      {
	proto_tree_add_item(newtree, hf_epa_int16, tvb,
			    12 + (i*sizeof(gint16)),
			    sizeof(gint16), TRUE);
      }
      proto_tree_add_text(newtree, tvb, 0, 0, "...");
      for (i = 0; i < 8; i++)
      {
	proto_tree_add_item(newtree, hf_epa_int16, tvb,
			    12 + WG_XWAVE_SIZE - (8*sizeof(gint16)) + (i*sizeof(gint16)),
			    sizeof(gint16), TRUE);
      }
    }
    else if ( (WRITE == type || READACK == type) && BF == pid)
    {
      // WRITE BF_*

      newitem = proto_tree_add_text(epa_tree, tvb, 12, -1, "Beamformer Payload");
      newtree = proto_item_add_subtree(newitem, ett_payload);

      int i;
      for (i = 0; i < 16; i++)
      {
	proto_tree_add_item(newtree, hf_epa_int16, tvb,
			    12 + (i*sizeof(gint16)),
			    sizeof(gint16), TRUE);
      }
      proto_tree_add_text(newtree, tvb, 0, 0, "...");
      for (i = 0; i < 8; i++)
      {
	proto_tree_add_item(newtree, hf_epa_int16, tvb,
			    12 + FRAGMENT_SIZE_BYTES - (8*sizeof(gint16)) + (i*sizeof(gint16)),
			    sizeof(gint16), TRUE);
      }
    }
    else if ( (WRITE == type || READACK == type) && SS == pid && SS_SELECT == reg)
    {
      // WRITE SS_SELECT

      newitem = proto_tree_add_text(epa_tree, tvb, 12, -1, "Subband Select Payload");
      newtree = proto_item_add_subtree(newitem, ett_payload);

      int i;
      for (i = 0; i < 16; i++)
      {
	proto_tree_add_item(newtree, hf_epa_uint16, tvb,
			    12 + (i*sizeof(guint16)),
			    sizeof(guint16), TRUE);
      }
      proto_tree_add_text(newtree, tvb, 0, 0, "...");
      for (i = 0; i < 8; i++)
      {
	proto_tree_add_item(newtree, hf_epa_uint16, tvb,
			    12 + SS_SELECT_SIZE_BYTES - (8*sizeof(guint16)) + (i*sizeof(guint16)),
			    sizeof(guint16), TRUE);
      }
    }
    else if ( READACK == type && (BST == pid || SST == pid) && (BST_POWER == reg || SST_POWER == reg) )
    {
      /* READACK [BST|SST]_MEAN POWER */

      newitem = proto_tree_add_text(epa_tree, tvb, 12, -1, "Statistics Payload (POWER)");
      newtree = proto_item_add_subtree(newitem, ett_payload);

      int i;
      /* print first 16 values */
      for (i = 0; i < 16; i++)
      {
	guint64 val64 = tvb_get_ntohl(tvb, 12 + (i*sizeof(guint32)));
	if ((1<<31) && val64) val64 <<= 25;
	double dval = (double)val64;
	
	proto_tree_add_double(newtree, hf_epa_double, tvb,
			      12 + (i*sizeof(guint32)),
			      sizeof(guint32), dval);
      }
      proto_tree_add_text(newtree, tvb, 0, 0, "...");

      /* print last 8 values */
      for (i = 0; i < 8; i++)
      {
	guint64 val64 = tvb_get_ntohl(tvb, 12 + FRAGMENT_SIZE_BYTES - (8*sizeof(guint32)) + (i*sizeof(guint32)));
	if ((1<<31) && val64) val64 <<= 25;
	double dval = (double)val64;

	proto_tree_add_double(newtree, hf_epa_double, tvb,
			      12 + FRAGMENT_SIZE_BYTES - (8*sizeof(guint32)) + (i*sizeof(guint32)),
			      sizeof(guint32), dval);
      }
    }
    else if ( (WRITE == type || READACK == type) && RCU == pid && RCU_SETTINGS == reg)
    {
      // WRITE or READACK RCU_SETTINGS
      newitem = proto_tree_add_text(epa_tree, tvb, 12, 2, "RCU status");
      newtree = proto_item_add_subtree(newitem, ett_rcusettings);
      {
	/* X-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 12, 2, "X polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree, df_vddvcc_en,  tvb, 12, 1, FALSE);
	proto_tree_add_item(subtree, df_vh_enable,  tvb, 12, 1, FALSE);
	proto_tree_add_item(subtree, df_vl_enable,  tvb, 12, 1, FALSE);
	proto_tree_add_item(subtree, df_filsel_b,   tvb, 12, 1, FALSE);
	proto_tree_add_item(subtree, df_filsel_a,   tvb, 12, 1, FALSE);
	proto_tree_add_item(subtree, df_bandsel,    tvb, 12, 1, FALSE);
	proto_tree_add_item(subtree, df_hba_enable, tvb, 12, 1, FALSE);
	proto_tree_add_item(subtree, df_lba_enable, tvb, 12, 1, FALSE);
      }
      {
	/* Y-polarization */
	newitem = proto_tree_add_text(newtree, tvb, 12, 2, "Y polarization");
	subtree = proto_item_add_subtree(newitem, ett_rcusettings);
	proto_tree_add_item(subtree, df_vddvcc_en,  tvb, 13, 1, FALSE);
	proto_tree_add_item(subtree, df_vh_enable,  tvb, 13, 1, FALSE);
	proto_tree_add_item(subtree, df_vl_enable,  tvb, 13, 1, FALSE);
	proto_tree_add_item(subtree, df_filsel_b,   tvb, 13, 1, FALSE);
	proto_tree_add_item(subtree, df_filsel_a,   tvb, 13, 1, FALSE);
	proto_tree_add_item(subtree, df_bandsel,    tvb, 13, 1, FALSE);
	proto_tree_add_item(subtree, df_hba_enable, tvb, 13, 1, FALSE);
	proto_tree_add_item(subtree, df_lba_enable, tvb, 13, 1, FALSE);
      }
    }
    else
    {
      proto_tree_add_item(epa_tree, hf_epa_data,        tvb, 12, -1, FALSE);
    }
  }

  /* If this protocol has a sub-dissector call it here, see section 1.8 */
}


/* Register the protocol with Ethereal */

/* this format is require because a script is used to build the C function
   that calls all the protocol registration.
*/

void
proto_register_epa(void)
{                 

  /* Setup list of header fields  See Section 1.6.1 for details*/
  static hf_register_info hf[] = {
    { &hf_epa_type,
      { "type",           "epa.type",
	FT_UINT8, BASE_HEX, VALS(type_vals), 0x0,          
	"Message type", HFILL }
    },
    {
      &hf_epa_error,
      { "error", "epa.error",
	FT_UINT8, BASE_DEC, VALS(mep_error_vals), 0x0,
	"Error indicator", HFILL }
    },
    { &hf_epa_seqnr,
      { "seqnr",           "epa.seqnr",
	FT_UINT16, BASE_DEC, NULL, 0x0,          
	"Sequence number", HFILL }
    },
    { &hf_epa_addr,
      { "addr",           "epa.addr",
	FT_BYTES, BASE_HEX, NULL, 0x0,          
	"Message addressing", HFILL }
    },
    { &hf_epa_offset,
      { "offset",           "epa.offset",
	FT_UINT16, BASE_DEC, NULL, 0x0,          
	"Register offset", HFILL }
    },
    { &hf_epa_size,
      { "size",           "epa.size",
	FT_UINT16, BASE_DEC, NULL, 0x0,          
	"Read/write size", HFILL }
    },
    { &hf_epa_data,
      { "data",           "epa.data",
	FT_BYTES, BASE_HEX, NULL, 0x0,          
	"Userdata", HFILL }
    },
    {
      &hf_epa_int16,
      { "payload_int16", "epa.payload_int16",
	FT_INT16, BASE_DEC, NULL, 0x0,
	"int16 payload", HFILL }
    },
    {
      &hf_epa_uint16,
      { "payload_uint16", "epa.payload_uint16",
	FT_UINT16, BASE_DEC, NULL, 0x0,
	"uint16 payload", HFILL }
    },
    {
      &hf_epa_int32,
      { "payload_int32", "epa.payload_int32",
	FT_INT32, BASE_DEC, NULL, 0x0,
	"int32 payload", HFILL }
    },
    {
      &hf_epa_uint32,
      { "payload_uint32", "epa.payload_uint32",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"uint32 payload", HFILL }
    },
    {
      &hf_epa_double,
      { "payload_double", "epa.payload_double",
	FT_DOUBLE, BASE_DEC, NULL, 0x0,
	"double precision values payload", HFILL }
    },
  };

  static hf_register_info addr_fields[] = {
    { &hf_epa_addr_dstid,
      { "dstid",           "epa.addr.dstid",
	FT_UINT8, BASE_HEX, VALS(dst_vals), 0x0,          
	"Destination ID", HFILL }
    },
    { &hf_epa_addr_pid,
      { "pid",           "epa.addr.pid",
	FT_UINT8, BASE_HEX, VALS(pid_vals), 0x0,          
	"Process ID", HFILL }
    },
    { &hf_epa_addr_regid,
      { "regid",           "epa.addr.regid",
	FT_STRING, BASE_HEX, NULL, 0x0,          
	"Register ID", HFILL }
    },
    { &hf_epa_addr_ffi,
      { "ffi",           "epa.addr.ffi",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"FFI", HFILL }
    },
  };

  static hf_register_info rspstatus_fields[] = {
    { &df_rspstatus_voltage_15,
      { "voltage_15",           "epa.data.rspstatus.voltage_15",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Measured voltage on 1.5V circuit", HFILL }
    },
    { &df_rspstatus_voltage_33,
      { "voltage_33",           "epa.data.rspstatus.voltage_33",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Measured voltage on 3.3 circuit", HFILL }
    },
    { &df_fpgastatus_bp_status,
      { "bp_status",           "epa.data.fpgastatus.bp_status",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Board Processor status", HFILL }
    },
    { &df_fpgastatus_bp_temp,
      { "bp_temp",           "epa.data.fpgastatus.bp_temp",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Board Processor temperature", HFILL }
    },
    { &df_fpgastatus_ap1_status,
      { "ap1_status",           "epa.data.fpgastatus.ap1_status",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Antenna Processor 1 status", HFILL }
    },
    { &df_fpgastatus_ap1_temp,
      { "ap1_temp",           "epa.data.fpgastatus.ap1_temp",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Antenna Processor 1 temperature", HFILL }
    },
    { &df_fpgastatus_ap2_status,
      { "ap2_status",           "epa.data.fpgastatus.ap2_status",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Antenna Processor 2 status", HFILL }
    },
    { &df_fpgastatus_ap2_temp,
      { "ap2_temp",           "epa.data.fpgastatus.ap2_temp",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Antenna Processor 2 temperature", HFILL }
    },
    { &df_fpgastatus_ap3_status,
      { "ap3_status",           "epa.data.fpgastatus.ap3_status",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Antenna Processor 3 status", HFILL }
    },
    { &df_fpgastatus_ap3_temp,
      { "ap3_temp",           "epa.data.fpgastatus.ap3_temp",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Antenna Processor 3 temperature", HFILL }
    },
    { &df_fpgastatus_ap4_status,
      { "ap4_status",           "epa.data.fpgastatus.ap4_status",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Antenna Processor 4 status", HFILL }
    },
    { &df_fpgastatus_ap4_temp,
      { "ap4_temp",           "epa.data.fpgastatus.ap4_temp",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Current Antenna Processor 4 temperature", HFILL }
    },
    {
      &df_ethstatus_nof_frames,
      {
	"nof_frames", "epa.data.ethstatus.nof_frames",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Number of eth frames received", HFILL 
      }
    },
    {
      &df_ethstatus_nof_errors,
      {
	"nof_errors", "epa.data.ethstatus.nof_errors",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Number of incorrect ethernet frames", HFILL 
      }
    },
    {
      &df_ethstatus_last_error,
      {
	"last_error", "epa.data.ethstatus.last_error",
	FT_UINT8, BASE_DEC, VALS(eth_error_vals), 0x0,
	"Error status of last received ethernet frame", HFILL 
      }
    },
    {
      &df_mepstatus_seqnr,
      {
	"seqnr", "epa.data.mepstatus.seqnr",
	FT_UINT16, BASE_DEC, NULL, 0x0,
	"Sequence number of previously received message", HFILL 
      }
    },
    {
      &df_mepstatus_error,
      {
	"error", "epa.data.mepstatus.error",
	FT_UINT8, BASE_DEC, VALS(mep_error_vals), 0x0,
	"Error status of previously received message", HFILL 
      }
    },
    {
      &df_ap1_sync_sample_count,
      {
	"sample_count", "epa.data.ap1_sync.sample_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Sample count at last sync event", HFILL
      }
    },
    {
      &df_ap1_sync_sync_count,
      {
	"sync_count", "epa.data.ap1_sync.sync_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Counter for the number of sync events", HFILL
      }
    },
    {
      &df_ap1_sync_error_count,
      {
	"error_count", "epa.data.ap1_sync.error_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Counter for the number of sync errors", HFILL
      }
    },
    {
      &df_ap2_sync_sample_count,
      {
	"sample_count", "epa.data.ap2_sync.sample_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Sample count at last sync event", HFILL
      }
    },
    {
      &df_ap2_sync_sync_count,
      {
	"sync_count", "epa.data.ap2_sync.sync_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Counter for the number of sync events", HFILL
      }
    },
    {
      &df_ap2_sync_error_count,
      {
	"error_count", "epa.data.ap2_sync.error_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Counter for the number of sync errors", HFILL
      }
    },
    {
      &df_ap3_sync_sample_count,
      {
	"sample_count", "epa.data.ap3_sync.sample_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Sample count at last sync event", HFILL
      }
    },
    {
      &df_ap3_sync_sync_count,
      {
	"sync_count", "epa.data.ap3_sync.sync_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Counter for the number of sync events", HFILL
      }
    },
    {
      &df_ap3_sync_error_count,
      {
	"error_count", "epa.data.ap3_sync.error_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Counter for the number of sync errors", HFILL
      }
    },
    {
      &df_ap4_sync_sample_count,
      {
	"sample_count", "epa.data.ap4_sync.sample_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Sample count at last sync event", HFILL
      }
    },
    {
      &df_ap4_sync_sync_count,
      {
	"sync_count", "epa.data.ap4_sync.sync_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Counter for the number of sync events", HFILL
      }
    },
    {
      &df_ap4_sync_error_count,
      {
	"error_count", "epa.data.ap4_sync.error_count",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"Counter for the number of sync errors", HFILL
      }
    },
  };

  static hf_register_info rspversion_fields[] = {
    { &df_rsp_version,
      { "rsp_version",           "epa.data.rsp_version",
	FT_UINT8, BASE_DEC, NULL, 0x0,          
	"Version of the RSP board hardware", HFILL }
    },
    { &df_bp_version,
      { "bp_version",           "epa.data.bp_version",
	FT_UINT8, BASE_DEC, NULL, 0x0,          
	"Version of the BP fpga firmware", HFILL }
    },
    { &df_ap_version,
      { "ap_version",           "epa.data.ap_version",
	FT_UINT8, BASE_DEC, NULL, 0x0,          
	"Version of the AP fpga firmware", HFILL }
    }
  };

  static hf_register_info rcusettings_fields[] = {
    { &df_vddvcc_en,
      { "vddvcc_en",           "epa.data.vddvcc_en",
	FT_UINT8, BASE_HEX, NULL, 0x80, // bit 7
	"VDDVCC_EN", HFILL }
    },
    { &df_vh_enable,
      { "vh_enable",           "epa.data.vh_enable",
	FT_UINT8, BASE_HEX, NULL, 0x40, // bit 6
	"VH_ENABLE", HFILL }
    },
    { &df_vl_enable,
      { "vl_enable",           "epa.data.vl_enable",
	FT_UINT8, BASE_HEX, NULL, 0x20, // bit 5
	"VL_ENABLE", HFILL }
    },
    { &df_filsel_b,
      { "filsel_b",           "epa.data.filsel_b",
	FT_UINT8, BASE_HEX, NULL, 0x10, // bit 4
	"FILSEL_B", HFILL }
    },
    { &df_filsel_a,
      { "filsel_a",           "epa.data.filsel_a",
	FT_UINT8, BASE_HEX, NULL, 0x08, // bit 3
	"FILSEL_A", HFILL }
    },
    { &df_bandsel,
      { "bandsel",           "epa.data.bandsel",
	FT_UINT8, BASE_HEX, NULL, 0x04, // bit 2
	"BANDSEL", HFILL }
    },
    { &df_hba_enable,
      { "hba_enable",           "epa.data.hba_enable",
	FT_UINT8, BASE_HEX, NULL, 0x02, // bit 1
	"HBA_ENABLE", HFILL }
    },
    { &df_lba_enable,
      { "lba_enable",           "epa.data.lba_enable",
	FT_UINT8, BASE_HEX, NULL, 0x01, // bit 0
	"LBA_ENABLE", HFILL }
    },
    { &df_nof_overflow,
      { "nof_overflow", "epa.data.nof_overflow",
	FT_UINT32, BASE_DEC, NULL, 0x0,
	"NOF_OVERFLOW", HFILL }
    },
  };

  static hf_register_info wgsettings_fields[] = {
    { &df_wg_freq,
      { "freq", "epa.data.wg.freq",
	FT_UINT16, BASE_DEC, NULL, 0x0,
	"Frequency", HFILL }
    },
    { &df_wg_phase,
      { "phase", "epa.data.wg.phase",
	FT_UINT8, BASE_DEC, NULL, 0x0,
	"Phase", HFILL }
    },
    { &df_wg_ampl,
      { "ampl", "epa.data.wg.ampl",
	FT_UINT8, BASE_DEC, NULL, 0x0,
	"Amplitude", HFILL }
    },
    { &df_wg_nof_samples,
      { "nof_samples", "epa.data.wg.nof_samples",
	FT_UINT16, BASE_DEC, NULL, 0x0,
	"Number of user waveform samples", HFILL }
    },
    { &df_wg_mode,
      { "mode", "epa.data.wg.mode",
	FT_UINT8, BASE_DEC, NULL, 0x0,
	"Mode", HFILL }
    },
  };

  /* Setup protocol subtree array */
  static gint* ett[] = {
    &ett_epa,
    &ett_epa_addr,
    &ett_rspstatus,
    &ett_rspstatus_detail,
    &ett_fpgastatus,
    &ett_ethstatus,
    &ett_mepstatus,
    &ett_syncstatus,
    &ett_syncvalues,
    &ett_rcustatus,
    &ett_rcusettings,
    &ett_rspversion,
    &ett_wgsettings,
    &ett_payload,
  };

  /* Register the protocol name and description */
  proto_epa = proto_register_protocol("LOFAR EPA Protocol",
				      "EPA", "epa");

  /* Required function calls to register the header fields and subtrees used */
  proto_register_field_array(proto_epa, hf, array_length(hf));
  proto_register_field_array(proto_epa, addr_fields, array_length(addr_fields));
  proto_register_field_array(proto_epa, rspstatus_fields, array_length(rspstatus_fields));
  proto_register_field_array(proto_epa, rspversion_fields, array_length(rspversion_fields));
  proto_register_field_array(proto_epa, rcusettings_fields, array_length(rcusettings_fields));
  proto_register_field_array(proto_epa, wgsettings_fields, array_length(wgsettings_fields));
  proto_register_subtree_array(ett, array_length(ett));
}


/* If this dissector uses sub-dissector registration add a registration routine.
   This format is required because a script is used to find these routines and
   create the code that calls these routines.
*/
void
proto_reg_handoff_epa(void)
{
  dissector_handle_t epa_handle;

  epa_handle = create_dissector_handle(dissect_epa, proto_epa);
  dissector_add("ethertype", ETHERTYPE_EPA, epa_handle);
}

void plugin_init(plugin_address_table_t* pat)
{
  /* initialise the table of pointers needed in Win32 DLLs */
  pat = pat; /* shut-up compiler warning on non Win32 hosts */
  plugin_address_table_init(pat);

  /* register the new protocol, protocol fields, and subtrees */
  if (proto_epa == -1) { /* execute protocol initialization only once */
    proto_register_epa();
  }
}

void plugin_reg_handoff(void)
{
  proto_reg_handoff_epa();
}
