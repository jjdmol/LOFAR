//#  -*- mode: c++ -*-
//#
//#  AllRegisterState.h: class that can contain the communication/update status of all register.
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

#ifndef ALLREGISTERSTATE_H_
#define ALLREGISTERSTATE_H_

#include <blitz/array.h>
#include <Common/LofarTypes.h>
#include <APL/RSP_Protocol/MEPHeader.h>
#include <APL/RTCCommon/RegisterState.h>
#include <iostream>

namespace LOFAR {
  namespace RSP_Protocol {

    /**
     * This class is used to contain the RegisterState of all registers.
     *
     * It is used in the following events:
     * GETREGISTERSTATEACK
     * UPDREGISTERSTATE
     *
     */
    class AllRegisterState
      {
      public:

	/**
	 * Constructors for a AllRegisterState object.
	 */
	AllRegisterState() : m_nrcus(0) {}

	/* Destructor for AllRegisterState. */
	virtual ~AllRegisterState() {}

      public:

	void init(int nrRspBoards, int nrBlps, int nrRcus) {

	  m_nrcus = nrRcus;

	  //
	  // resize to appropriate size and mark modified
	  // to force initial update
	  //
	  sys_state.resize(nrRspBoards);
	  sys_state.modified();

	  bf_state.resize(nrRcus * EPA_Protocol::MEPHeader::N_PHASE); // XR, XI, YR, YI
	  bf_state.modified();

	  ss_state.resize(nrBlps);
	  ss_state.modified();

	  rcusettings_state.resize(nrRcus);
	  rcusettings_state.modified();

	  rcuprotocol_state.resize(nrRcus);
	  rcuprotocol_state.modified();

	  rsuclear_state.resize(nrRspBoards);
	  rsuclear_state.clear();

	  diagwgsettings_state.resize(nrRcus);
	  diagwgsettings_state.modified();

	  sst_state.resize(nrBlps);
	  sst_state.modified();

	  bst_state.resize(nrRspBoards);
	  bst_state.modified();

	  xst_state.resize(nrRspBoards);
	  xst_state.modified();

	  cdo_state.resize(nrRspBoards);
	  cdo_state.modified();

	  bs_state.resize(nrBlps);
	  bs_state.modified();

	  tds_state.resize(nrRspBoards);
	  tds_state.modified();
	}

	void clear() {
	  sys_state.clear();
	  bf_state.clear();
	  ss_state.clear();
	  rcusettings_state.clear();
	  rcuprotocol_state.clear();
	  rsuclear_state.clear();
	  diagwgsettings_state.clear();
	  sst_state.clear();
	  bst_state.clear();
	  xst_state.clear();
	  cdo_state.clear();
	  bs_state.clear();
	  tds_state.clear();
	}

	void print(std::ostream& out) const {
	  out << "                  ";
	  for (int i = 0; i < m_nrcus * EPA_Protocol::MEPHeader::N_POL; i++) {
	    out << (i % 10);
	  }
	  out << endl;
	  out << "System Status     "; sys_state.print(out);
	  out << "BF                "; bf_state.print(out);
	  out << "Subband Selection "; ss_state.print(out);
	  out << "RCUSettings       "; rcusettings_state.print(out);
	  out << "RCUProtocol       "; rcuprotocol_state.print(out);
	  out << "RSUClear          "; rsuclear_state.print(out);
	  out << "DIAGWGSettings    "; diagwgsettings_state.print(out);
	  out << "SubbandStats      "; sst_state.print(out);
	  out << "BeamletStats      "; bst_state.print(out);
	  out << "XCorrelationStats "; xst_state.print(out);
	  out << "CDO               "; cdo_state.print(out);
	  out << "BS                "; bs_state.print(out);
	  out << "TDS               "; tds_state.print(out);
	  out << endl;
	}

	/*@{*/
	/**
	 * marshalling methods
	 */
	unsigned int getSize();
	unsigned int pack  (void* buffer);
	unsigned int unpack(void *buffer);
	/*@}*/

      public:
	/*@{*/
	/**
	 * Accessor methods
	 */
	RTC::RegisterState& sys()            { return sys_state; }
	RTC::RegisterState& bf()             { return bf_state; }
	RTC::RegisterState& ss()             { return ss_state; }
	RTC::RegisterState& rcusettings()    { return rcusettings_state; }
	RTC::RegisterState& rcuprotocol()    { return rcuprotocol_state; }
	RTC::RegisterState& rsuclear()       { return rsuclear_state; }
	RTC::RegisterState& diagwgsettings() { return diagwgsettings_state; }
	RTC::RegisterState& sst()            { return sst_state; }
	RTC::RegisterState& bst()            { return bst_state; }
	RTC::RegisterState& xst()            { return xst_state; }
	RTC::RegisterState& cdo()            { return cdo_state; }
	RTC::RegisterState& bs()            { return bs_state; }
	RTC::RegisterState& tds()            { return tds_state; }
	/*@}*/

      private:
	RTC::RegisterState sys_state;            // RSR state
	RTC::RegisterState bf_state;             // BF weights state
	RTC::RegisterState ss_state;             // SS state
	RTC::RegisterState rcusettings_state;    // RCU settings state
	RTC::RegisterState rcuprotocol_state;    // RCU protocol state
	RTC::RegisterState rsuclear_state;       // RSU clear state
	RTC::RegisterState diagwgsettings_state; // DIAG WG settings state
	RTC::RegisterState sst_state;            // SST state
	RTC::RegisterState bst_state;            // BST state
	RTC::RegisterState xst_state;            // XST State
	RTC::RegisterState cdo_state;            // CDO state
	RTC::RegisterState bs_state;             // BS register state
	RTC::RegisterState tds_state;            // TDS register state (Clock board)

	int m_nrcus;
      };
  };
}; // namespace LOFAR

#endif /* ALLREGISTERSTATE_H_ */
