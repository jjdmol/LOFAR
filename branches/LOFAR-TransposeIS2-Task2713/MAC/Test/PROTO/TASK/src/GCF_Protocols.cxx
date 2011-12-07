//  FProtocols.cc: protocols used by the framework
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

#include "GCF_Protocols.hxx"

/**
 * F_FSM_PROTOCOL signal names
 */
const char* F_FSM_PROTOCOL_names[] =
  {
    "F_FSM_PROTOCOL: invalid signal",
    "F_ENTRY_SIG (IN)",
    "F_EXIT_SIG (IN)",
    "F_INIT_SIG (IN)",
  };

/**
 * F_PORT_PROTOCOL signal names
 */
const char* F_PORT_PROTOCOL_names[] =
  {
    "F_PORT_PROTOCOL: invalid signal",
    "F_CONNECT_SIG (OUT)",
    "F_CONNECTED_SIG (IN)",
    "F_DISCONNECTED_SIG (IN)",
    "F_CLOSED_SIG (IN)",
    "F_TIMER_SIG (IN)",
    "F_DATAIN_SIG (IN)",
    "F_DATAOUT_SIG (IN)",
    "F_RAW_SIG (IN_OUT)",
  };

/**
 * F_PVSS_PROTOCOL signal names
 */
const char* F_PVSS_PROTOCOL_names[] =
  {
    "F_PVSS_PROTOCOL: invalid signal",
    "F_DPCONNECTED_SIG (IN)",
    "F_DPDISCONNECTED_SIG (IN)",
    "F_DPCREATED_SIG (IN)",
    "F_DPDELETED_SIG (IN)",
    "F_VCHANGEMSG_SIG (IN)",
    "F_VGETRESP_SIG (IN)",
    "F_VSETRESP_SIG (IN)",
  };
