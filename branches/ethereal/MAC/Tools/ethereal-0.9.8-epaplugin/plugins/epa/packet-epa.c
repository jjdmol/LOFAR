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
static const value_string type_vals[] =
{
  { 0x00, "Invalid message type"  },
  { 0x01, "Read request (READ)"   },
  { 0x02, "Write command (WRITE)" },
  { 0x03, "Read result (READRES)" },
  { 0x04, "Read error (READERR)"  },
  { 0,     NULL                   },
};

static const value_string page_vals[] =
{
  { 0x00, "Write page for LCU (INACTIVE)" },
  { 0x01, "Read page for FPGA (ACTIVE)"   },
  { 0,     NULL                           },
};

static const value_string dst_vals[] =
{
  { 0x00, "Beamlet processor" },
  { 0x80, "RSP Main FPGA"     },
  { 0,     NULL               },  
};

static const value_string pid_vals[] =
{
  { 0x00, "Status overview (STATUS)"           },
  { 0x01, "Selftest functionality (TST)"       },
  { 0x02, "FPGA configuration and reset (CFG)" },
  { 0x03, "Waveform generator (WG)"            },
  { 0x04, "Subband select (SS)"                },
  { 0x05, "Beamformer (BF)"                    },
  { 0x06, "Beamlet statistics (ST)"            },
  { 0x07, "Subband statistics (STSUB)"         },
  { 0x08, "RCU Control (RCU)"                  },
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
  { 0x00, "Waveform generator settings" },
  { 0x01, "User waveform"               },
  { 0x02, "Soft PPS"                    },
  { 0,     NULL                         },
};

static const value_string ss_vals[] =
{
  { 0x00, "Number of selected subbands" },
  { 0x01, "Subband Select parameters"   },
  { 0,     NULL                         },
};

static const value_string bf_vals[] =
{
  { 0x00, "Coefs Xre" },
  { 0x01, "Coefs Xim" },
  { 0x02, "Coefs Yre" },
  { 0x03, "Coefs Yim" },
  { 0,     NULL       },
};

static const value_string st_vals[] =
{
  { 0x00, "Mean"  },
  { 0x01, "Power" },
  { 0,     NULL   },
};

static const value_string stsub_vals[] =
{
  { 0x00, "Mean"  },
  { 0x01, "Power" },
  { 0,     NULL   },
};

static const value_string rcu_vals[] =
{
  { 0x00, "RCU Settings"  },
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
static int hf_epa_ffi         = -1;
static int hf_epa_seqnr       = -1;
static int hf_epa_addr_dstid  = -1;
static int hf_epa_addr_pid    = -1;
static int hf_epa_addr_regid  = -1;
static int hf_epa_addr_pageid = -1;
static int hf_epa_offset      = -1;
static int hf_epa_size        = -1;
static int hf_epa_data        = -1;

/* Initialize the subtree pointers */
static gint ett_epa = -1;

/* Code to actually dissect the packets */
static void
dissect_epa(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{

  /* Set up structures needed to add the protocol subtree and manage it */
  proto_item *ti;
  proto_tree *epa_tree;

  /* Make entries in Protocol column and Info column on summary display */
  if (check_col(pinfo->cinfo, COL_PROTOCOL)) 
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "epa");
    
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

  /* In the interest of speed, if "tree" is NULL, don't do any work not
     necessary to generate protocol tree items. */
  if (tree)
  {

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
    proto_tree_add_item(epa_tree, hf_epa_ffi,         tvb,  1,  1, FALSE);
    proto_tree_add_item(epa_tree, hf_epa_seqnr,       tvb,  2,  2, TRUE);
    proto_tree_add_item(epa_tree, hf_epa_addr_dstid,  tvb,  4,  1, FALSE);
    proto_tree_add_item(epa_tree, hf_epa_addr_pid,    tvb,  5,  1, FALSE);
    proto_tree_add_item(epa_tree, hf_epa_addr_regid,  tvb,  6,  1, FALSE);
    proto_tree_add_item(epa_tree, hf_epa_addr_pageid, tvb,  7,  1, FALSE);
    proto_tree_add_item(epa_tree, hf_epa_offset,      tvb,  8,  2, TRUE);
    proto_tree_add_item(epa_tree, hf_epa_size,        tvb, 10,  2, TRUE);
    proto_tree_add_item(epa_tree, hf_epa_data,        tvb, 12, -1, FALSE);

  }

  /* fill the INFO column */
  if (check_col(pinfo->cinfo, COL_INFO))
  {
    char*  typestr = type_vals[tvb_get_guint8(tvb, 0)].strptr;
    char*  regstr  = 0;
    guint8 reg     = tvb_get_guint8(tvb, 6);
    guint8 pid     = tvb_get_guint8(tvb, 5);
    
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
	regstr = st_vals[reg].strptr;
	break;

      case 0x07:
	regstr = stsub_vals[reg].strptr;
	break;

      case 0x08:
	regstr = rcu_vals[reg].strptr;
	break;
    }

    if (!typestr) typestr = "Unknown type?";
    if (!regstr)  regstr  = "Unknown register?";
      
    col_add_fstr(pinfo->cinfo, COL_INFO, "%s; %s", typestr, regstr);
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
    { &hf_epa_ffi,
      { "ffi",           "epa.ffi",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"For Future Information", HFILL }
    },
    { &hf_epa_seqnr,
      { "seqnr",           "epa.seqnr",
	FT_UINT16, BASE_DEC, NULL, 0x0,          
	"Sequence number", HFILL }
    },
    { &hf_epa_addr_dstid,
      { "addr_dstid",           "epa.addr_dstid",
	FT_UINT8, BASE_HEX, VALS(dst_vals), 0x0,          
	"Destination ID", HFILL }
    },
    { &hf_epa_addr_pid,
      { "addr_pid",           "epa.addr_pid",
	FT_UINT8, BASE_HEX, VALS(pid_vals), 0x0,          
	"Process ID", HFILL }
    },
    { &hf_epa_addr_regid,
      { "addr_regid",           "epa.addr_regid",
	FT_UINT8, BASE_HEX, NULL, 0x0,          
	"Register ID", HFILL }
    },
    { &hf_epa_addr_pageid,
      { "addr_pageid",           "epa.addr_pageid",
	FT_UINT8, BASE_HEX, VALS(page_vals), 0x0,          
	"Page ID", HFILL }
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

  /* Setup protocol subtree array */
  static gint *ett[] = {
    &ett_epa,
  };

  /* Register the protocol name and description */
  proto_epa = proto_register_protocol("LOFAR EPA Protocol",
				      "EPA", "epa");

  /* Required function calls to register the header fields and subtrees used */
  proto_register_field_array(proto_epa, hf, array_length(hf));
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
