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

/**
 * EPA protocol constants and value to string mappings.
 */
static const value_string type_info_vals[] =
{
  { 0x00, "Invalid message type"  },
  { 0x01, "READ   "  },
  { 0x02, "WRITE  "  },
  { 0x03, "READACK"  },
  { 0x04, "WRITEACK" },
  { 0,     NULL                   },
};

static const value_string type_vals[] =
{
  { 0x00, "Invalid message type"          },
  { 0x01, "Read  request     (READ)"      },
  { 0x02, "Write command     (WRITE)"     },
  { 0x03, "Read  acknowledge (READACK)"   },
  { 0x04, "Write acknowledge (WRITEACK)"  },
  { 0,     NULL                   },
};

static const value_string dst_vals[] =
{
  { 0x00, "Beamlet processor" },
  { 0x80, "RSP main FPGA"     },
  { 0,     NULL               },  
};

static const value_string pid_info_vals[] =
{
  { 0x00, "RSR   " },
  { 0x01, "TST   " },
  { 0x02, "CFG   " },
  { 0x03, "WG    " },
  { 0x04, "SS    " },
  { 0x05, "BF    " },
  { 0x06, "BST   " },
  { 0x07, "SST   " },
  { 0x08, "RCU   " },
  { 0x09, "CRR   " },
  { 0x0A, "CRB   " },
  { 0x0B, "CDO   " },
  { 0,     NULL    },
};

static const value_string pid_vals[] =
{
  { 0x00, "Status overview (RSR)"              },
  { 0x01, "Selftest functionality (TST)"       },
  { 0x02, "FPGA configuration and reset (CFG)" },
  { 0x03, "Waveform generator (WG)"            },
  { 0x04, "Subband select (SS)"                },
  { 0x05, "Beamformer (BF)"                    },
  { 0x06, "Beamlet statistics (BST)"           },
  { 0x07, "Subband statistics (SST)"           },
  { 0x08, "RCU Control (RCU)"                  },
  { 0x09, "RSP Clock and Reset",               },
  { 0x0A, "BLP Clock and Reset",               },
  { 0x0B, "CEP Data Output",                   },
  { 0,     NULL                                },
};

static const value_string status_vals[] =
{
  { 0x00, "RSP Status" },
  { 0x01, "Version"    },
  { 0,     NULL        },
};

static const value_string tst_vals[] =
{
  { 0x00, "Selftest" },
  { 0,     NULL        },
};

static const value_string cfg_vals[] =
{
  { 0x00, "Reset"     },
  { 0x01, "Reprogram" },
  { 0,     NULL       },
};

static const value_string wg_vals[] =
{
  { 0x00, "Waveform generator settings X polarization" },
  { 0x02, "Waveform generator settings Y polarization" },
  { 0x03, "User waveform X polarization"               },
  { 0x04, "User waveform Y polarization"               },
  { 0,     NULL                         },
};

static const value_string ss_vals[] =
{
  { 0x00, "Subband Select parameters"   },
  { 0,     NULL                         },
};

static const value_string bf_vals[] =
{
  { 0x00, "XR,XI,YR,YI coefficients for XR output" },
  { 0x01, "XR,XI,YR,YI coefficients for XI output" },
  { 0x02, "XR,XI,YR,YI coefficients for YR output" },
  { 0x03, "XR,XI,YR,YI coefficients for YR output" },
  { 0,     NULL       },
};

static const value_string bst_vals[] =
{
  { 0x00, "Beamlet Statistics - XR,XI,YR,YI Mean"   },
  { 0x01, "Beamlet Statistics - XR,XI,YR,YI Power"  },
  { 0,     NULL   },
};

static const value_string sst_vals[] =
{
  { 0x00, "Subband Statistics - XR,XI,YR,YI Mean"   },
  { 0x01, "Subband Statistics - XR,XI,YR,YI Power"  },
  { 0,     NULL   },
};

static const value_string rcu_vals[] =
{
  { 0x00, "RCU Settings"  },
  { 0,     NULL   },
};

static const value_string crr_vals[] =
{
  { 0x00, "Soft Reset"  },
  { 0x01, "Soft PPS"  },
  { 0,     NULL   },
};

static const value_string crb_vals[] =
{
  { 0x00, "Soft Reset"  },
  { 0x01, "Soft PPS"  },
  { 0,     NULL   },
};

static const value_string cdo_vals[] =
{
  { 0x00, "CEP Data Output Settings"  },
  { 0,     NULL   },
};

static const value_string eth_error_vals[] =
{
  { 0, "The ethernet frame was received correctly"           },
  { 1, "Preamble had other value than 0xAA"                  },
  { 2, "Frame delimiter had other value than 0xAB"           },
  { 3, "Not enough preamble nibbles"                         },
  { 4, "Frame ended during frame header."                    },
  { 5, "Caculated CRC does not match received CRC"           },
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
  { 0,     NULL   },
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
G_MODULE_EXPORT const gchar version[] = "1.0";
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
static int df_mepstatus_read_seqnr  = -1;
static int df_mepstatus_read_error  = -1;
static int df_mepstatus_write_seqnr = -1;
static int df_mepstatus_write_error = -1;
/*static int df_syncstatus = -1; */
static int df_syncstatus_sample_count = -1;
static int df_syncstatus_sync_count   = -1;
static int df_syncstatus_error_count  = -1;
/*static int df_rcustatus             = -1;*/
static int df_rcustatus_ap1_rcu   = -1;
static int df_rcustatus_ap2_rcu   = -1;
static int df_rcustatus_ap3_rcu   = -1;
static int df_rcustatus_ap4_rcu   = -1;

/* Initialize the subtree pointers */
static gint ett_epa       = -1;
static gint ett_epa_addr  = -1;
static gint ett_rspstatus = -1;

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
    case 0x00:
      regstr = status_vals[reg].strptr;
      break;

    case 0x01:
      regstr = tst_vals[reg].strptr;
      break;

    case 0x02:
      regstr = cfg_vals[reg].strptr;
      break;

    case 0x03:
      regstr = wg_vals[reg].strptr;
      break;

    case 0x04:
      regstr = ss_vals[reg].strptr;
      break;

    case 0x05:
      regstr = bf_vals[reg].strptr;
      break;

    case 0x06:
      regstr = bst_vals[reg].strptr;
      break;

    case 0x07:
      regstr = sst_vals[reg].strptr;
      break;

    case 0x08:
      regstr = rcu_vals[reg].strptr;
      break;

    case 0x09:
      regstr = crr_vals[reg].strptr;
      break;

    case 0x0A:
      regstr = crb_vals[reg].strptr;
      break;

    case 0x0B:
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

    if (0x03 == type && 0x00 == pid && 0x00 == reg)
    {
      newitem = proto_tree_add_text(epa_tree, tvb, 12, 96, "RSP Status register");
      newtree = proto_item_add_subtree(newitem, ett_rspstatus);

      proto_tree_add_text(newtree, tvb, 12, 4,  "RSP Status");
      proto_tree_add_item(newtree, df_rspstatus_voltage_15  ,tvb, 12, 1,  FALSE);
      proto_tree_add_item(newtree, df_rspstatus_voltage_33  ,tvb, 13, 1,  FALSE);

      proto_tree_add_text(newtree, tvb,  16, 12, "FPGA Status");
      proto_tree_add_item(newtree, df_fpgastatus_bp_status  ,tvb, 16, 1,  FALSE);
      proto_tree_add_item(newtree, df_fpgastatus_bp_temp    ,tvb, 17, 1,  FALSE);
      proto_tree_add_item(newtree, df_fpgastatus_ap1_status ,tvb, 18, 1,  FALSE);
      proto_tree_add_item(newtree, df_fpgastatus_ap1_temp   ,tvb, 19, 1,  FALSE);
      proto_tree_add_item(newtree, df_fpgastatus_ap2_status ,tvb, 20, 1,  FALSE);
      proto_tree_add_item(newtree, df_fpgastatus_ap2_temp   ,tvb, 21, 1,  FALSE);   
      proto_tree_add_item(newtree, df_fpgastatus_ap3_status ,tvb, 22, 1,  FALSE);
      proto_tree_add_item(newtree, df_fpgastatus_ap3_temp   ,tvb, 23, 1,  FALSE);
      proto_tree_add_item(newtree, df_fpgastatus_ap4_status ,tvb, 24, 1,  FALSE);
      proto_tree_add_item(newtree, df_fpgastatus_ap4_temp   ,tvb, 25, 1,  FALSE);   

      proto_tree_add_text(newtree, tvb, 28, 12, "ETH Status");
      proto_tree_add_item(newtree, df_ethstatus_nof_frames  ,tvb, 28, 4,  TRUE);
      proto_tree_add_item(newtree, df_ethstatus_nof_errors  ,tvb, 32, 4,  TRUE);
      proto_tree_add_item(newtree, df_ethstatus_last_error  ,tvb, 36, 1,  FALSE);

      proto_tree_add_text(newtree, tvb, 40, 8, "MEP Status");
      proto_tree_add_item(newtree, df_mepstatus_read_seqnr  ,tvb, 40, 2,  TRUE);
      proto_tree_add_item(newtree, df_mepstatus_read_error  ,tvb, 42, 1,  FALSE);
      proto_tree_add_item(newtree, df_mepstatus_write_seqnr ,tvb, 44, 2,  TRUE);
      proto_tree_add_item(newtree, df_mepstatus_write_error ,tvb, 46, 1,  FALSE);

      proto_tree_add_text(newtree, tvb, 48, 12, "SYNC Status");
      proto_tree_add_item(newtree, df_syncstatus_sample_count ,tvb, 48, 4, TRUE);
      proto_tree_add_item(newtree, df_syncstatus_sync_count   ,tvb, 52, 4, TRUE);
      proto_tree_add_item(newtree, df_syncstatus_error_count  ,tvb, 56, 4, TRUE);

      proto_tree_add_text(newtree, tvb, 60, 48, "RCU Status");
      proto_tree_add_item(newtree, df_rcustatus_ap1_rcu   ,tvb, 60, 12,  FALSE);
      proto_tree_add_item(newtree, df_rcustatus_ap2_rcu   ,tvb, 72, 12,  FALSE);
      proto_tree_add_item(newtree, df_rcustatus_ap3_rcu   ,tvb, 84, 12,  FALSE);
      proto_tree_add_item(newtree, df_rcustatus_ap4_rcu   ,tvb, 96, 12,  FALSE);
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
      &df_mepstatus_read_seqnr,
      {
	"read_seqnr", "epa.data.mepstatus.read_seqnr",
	FT_UINT16, BASE_DEC, NULL, 0x0,
	"Sequence number of last received read message", HFILL 
      }
    },
    {
      &df_mepstatus_read_error,
      {
	"read_error", "epa.data.mepstatus.read_error",
	FT_UINT8, BASE_DEC, VALS(mep_error_vals), 0x0,
	"Error status of last received read message", HFILL 
      }
    },
    {
      &df_mepstatus_write_seqnr,
      {
	"write_seqnr", "epa.data.mepstatus.write_seqnr",
	FT_UINT16, BASE_DEC, NULL, 0x0,
	"Sequence number of last received write message", HFILL 
      }
    },
    {
      &df_mepstatus_write_error,
      {
	"write_error", "epa.data.mepstatus.write_error",
	FT_UINT8, BASE_DEC, VALS(mep_error_vals), 0x0,
	"Error status of last received write message frame", HFILL 
      }
    },
    {
      &df_rcustatus_ap1_rcu,
      {
	"ap1_rcu", "epa.data.rcustatus.ap1_rcu",
	FT_BYTES, BASE_HEX, NULL, 0x0,
	"AP1 RCU status", HFILL 
      }
    },
    {
      &df_rcustatus_ap2_rcu,
      {
	"ap2_rcu", "epa.data.rcustatus.ap2_rcu",
	FT_BYTES, BASE_HEX, NULL, 0x0,
	"AP2 RCU status", HFILL 
      }
    },
    {
      &df_rcustatus_ap3_rcu,
      {
	"ap3_rcu", "epa.data.rcustatus.ap3_rcu",
	FT_BYTES, BASE_HEX, NULL, 0x0,
	"AP3 RCU status", HFILL 
      }
    },
    {
      &df_rcustatus_ap4_rcu,
      {
	"ap4_rcu", "epa.data.rcustatus.ap4_rcu",
	FT_BYTES, BASE_HEX, NULL, 0x0,
	"AP4 RCU status", HFILL 
      }
    },
  };

  /* Setup protocol subtree array */
  static gint* ett[] = {
    &ett_epa,
    &ett_epa_addr,
    &ett_rspstatus,
  };

  /* Register the protocol name and description */
  proto_epa = proto_register_protocol("LOFAR EPA Protocol",
				      "EPA", "epa");

  /* Required function calls to register the header fields and subtrees used */
  proto_register_field_array(proto_epa, hf, array_length(hf));
  proto_register_field_array(proto_epa, addr_fields, array_length(addr_fields));
  proto_register_field_array(proto_epa, rspstatus_fields, array_length(rspstatus_fields));
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
