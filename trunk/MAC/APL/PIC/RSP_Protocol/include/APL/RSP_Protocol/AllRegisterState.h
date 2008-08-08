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

// This class is used to contain the RegisterState of all registers.
//
// It is used in the following events:
// GETREGISTERSTATEACK
// UPDREGISTERSTATE
//
class AllRegisterState
{
public:
	// Constructors for a AllRegisterState object.
	AllRegisterState() : m_nrcus(0) {}
	virtual ~AllRegisterState() {}

	//
	// init(nrRSP, nrBLP, nrRCU)
	//
	void init(int nrRspBoards, int nrBlps, int nrRcus) {
		m_nrcus = nrRcus;

		//
		// resize to appropriate size and mark modified
		// to force initial update
		//
		sys_state.resize(nrRspBoards);
		bf_state.resize(nrRcus * EPA_Protocol::MEPHeader::N_PHASE); // XR, XI, YR, YI
		ss_state.resize(nrBlps);
		rcusettings_state.resize(nrRcus);
		rcuprotocol_state.resize(nrRcus);
		hbaprotocol_state.resize(nrRcus);
		rsuclear_state.resize(nrRspBoards);
		diagwgsettings_state.resize(nrRcus * EPA_Protocol::MEPHeader::N_DIAG_WG_REGISTERS);
		sst_state.resize(nrBlps * EPA_Protocol::MEPHeader::SST_N_FRAGMENTS);
		bst_state.resize(nrRspBoards);
		xst_state.resize(nrRspBoards * EPA_Protocol::MEPHeader::XST_NR_STATS);
		cdo_state.resize(nrRspBoards * EPA_Protocol::MEPHeader::N_CDO_REGISTERS);
		bs_state.resize(nrBlps);
		tdclear_state.resize(nrRspBoards);
		tdwrite_state.resize(nrRspBoards);
		tdread_state.resize(nrRspBoards);
		rad_state.resize(nrRspBoards);
		ts_state.resize(nrRspBoards);
		tdstatuswrite_state.resize(nrRspBoards);
		tdstatusread_state.resize(nrRspBoards);
		tbbsettings_state.resize(nrRcus);
		tbbbandsel_state.resize(nrRcus);
		bypasssettings_state.resize(nrBlps);
		rawdatawrite_state.resize(nrRspBoards);
		rawdataread_state.resize(nrRspBoards);
	}

	// Force update of some (not all) register.
	// Register excluded from update are 
	// tds, bs, rsuclear
	//
	// force()
	//
	void force() {
		sys_state.reset();
		bf_state.reset();
		ss_state.reset();
		rcusettings_state.reset();
		rcuprotocol_state.reset();
		hbaprotocol_state.reset();
		rsuclear_state.reset();
		diagwgsettings_state.reset();
		sst_state.reset();
		bst_state.reset();
		xst_state.reset();
		cdo_state.reset();
		bs_state.reset();
		tdclear_state.reset();
		tdwrite_state.reset();
		tdread_state.reset();
		rad_state.reset();
		ts_state.reset();
		tdstatuswrite_state.reset();
		tdstatusread_state.reset();
		tbbsettings_state.reset();
		tbbbandsel_state.reset();
		bypasssettings_state.reset();
		rawdatawrite_state.reset();
		rawdataread_state.reset();

		sys_state.read();
		bf_state.write();
		ss_state.write();
		rcusettings_state.write();
		rcuprotocol_state.write();
		hbaprotocol_state.write();
		rsuclear_state.check();
		diagwgsettings_state.write();
		sst_state.read();
		bst_state.read();
		xst_state.read();
		cdo_state.check();
		bs_state.check();
		tdclear_state.check();
		tdwrite_state.check();
		tdread_state.check();
		rad_state.write();
		ts_state.write();
		tdstatuswrite_state.write();
		tdstatusread_state.read();
		tbbsettings_state.check();
		tbbbandsel_state.check();
		bypasssettings_state.write(); // REO: When is this function called???
		rawdatawrite_state.check();
		rawdataread_state.check();
	}

	//
	// schedule()
	//
	void schedule() {
		sys_state.read();
		bf_state.write(); // always write bf
		ss_state.write(); // always write ss
		rcusettings_state.check();
		rcuprotocol_state.check();
		hbaprotocol_state.check();
		rsuclear_state.check();
		diagwgsettings_state.check();
		sst_state.read();
		bst_state.read();
		xst_state.read();
		cdo_state.check();
		bs_state.check();
		tdclear_state.check();
		tdwrite_state.check();
		tdread_state.check();
		rad_state.check();
		ts_state.write(); // always write timestamp
		tdstatuswrite_state.write();
		tdstatusread_state.read();
		tbbsettings_state.check();
		tbbbandsel_state.check();
		bypasssettings_state.check(); // REO ?
		rawdatawrite_state.check();
		rawdataread_state.check();
	}

	//
	// clear()
	//
	void clear() {
		sys_state.clear();
		bf_state.clear();
		ss_state.clear();
		rcusettings_state.clear();
		rcuprotocol_state.clear();
		hbaprotocol_state.clear();
		rsuclear_state.clear();
		diagwgsettings_state.clear();
		sst_state.clear();
		bst_state.clear();
		xst_state.clear();
		cdo_state.clear();
		bs_state.clear();
		tdclear_state.clear();
		tdwrite_state.clear();
		tdread_state.clear();
		rad_state.clear();
		ts_state.clear();
		tdstatuswrite_state.clear();
		tdstatusread_state.clear();
		tbbsettings_state.clear();
		tbbbandsel_state.clear();
		bypasssettings_state.clear();
		rawdatawrite_state.clear();
		rawdataread_state.clear();
	}

	//
	// print(os)
	//
	void print(std::ostream& out) const {
		out << "                  ";
		for (int i = 0; i < m_nrcus * EPA_Protocol::MEPHeader::N_POL; i++) {
		out << (i % 10) << " ";
		}
		out << endl;
		out << "System Status       "; sys_state.print(out);
		out << "BF                  "; bf_state.print(out);
		out << "Subband Selection   "; ss_state.print(out);
		out << "RCUSettings         "; rcusettings_state.print(out);
		out << "RCUProtocol         "; rcuprotocol_state.print(out);
		out << "HBAProtocol         "; hbaprotocol_state.print(out);
		out << "RSUClear            "; rsuclear_state.print(out);
		out << "DIAGWGSettings      "; diagwgsettings_state.print(out);
		out << "SubbandStats        "; sst_state.print(out);
		out << "BeamletStats        "; bst_state.print(out);
		out << "XCorrelationStats   "; xst_state.print(out);
		out << "CDO                 "; cdo_state.print(out);
		out << "BS                  "; bs_state.print(out);
		out << "TDSClear            "; tdclear_state.print(out);
		out << "TDSWrite            "; tdwrite_state.print(out);
		out << "TDSRead             "; tdread_state.print(out);
		out << "RAD                 "; rad_state.print(out);
		out << "Timestamp           "; ts_state.print(out);
		out << "TDS Status (write)  "; tdstatuswrite_state.print(out);
		out << "TDS Status (read)   "; tdstatusread_state.print(out);
		out << "TBBSettings         "; tbbsettings_state.print(out);
		out << "TBBBandsel          "; tbbbandsel_state.print(out);
		out << "DIAGBypassSettings  "; bypasssettings_state.print(out);
		out << "RawDataBlock(write) "; rawdatawrite_state.print(out);
		out << "RawDataBlock(read)  "; rawdataread_state.print(out);
		out << endl;
	}

	/*@{*/
	// marshalling methods
	unsigned int getSize();
	unsigned int pack  (void* buffer);
	unsigned int unpack(void *buffer);
	/*@}*/

public:
	/*@{*/
	// Accessor methods
	RTC::RegisterState& sys()            { return sys_state; }
	RTC::RegisterState& bf()             { return bf_state; }
	RTC::RegisterState& ss()             { return ss_state; }
	RTC::RegisterState& rcusettings()    { return rcusettings_state; }
	RTC::RegisterState& rcuprotocol()    { return rcuprotocol_state; }
	RTC::RegisterState& hbaprotocol()    { return hbaprotocol_state; }
	RTC::RegisterState& rsuclear()       { return rsuclear_state; }
	RTC::RegisterState& diagwgsettings() { return diagwgsettings_state; }
	RTC::RegisterState& sst()            { return sst_state; }
	RTC::RegisterState& bst()            { return bst_state; }
	RTC::RegisterState& xst()            { return xst_state; }
	RTC::RegisterState& cdo()            { return cdo_state; }
	RTC::RegisterState& bs()             { return bs_state; }
	RTC::RegisterState& tdclear()        { return tdclear_state; }
	RTC::RegisterState& tdwrite()        { return tdwrite_state; }
	RTC::RegisterState& tdread()         { return tdread_state; }
	RTC::RegisterState& rad()            { return rad_state; }
	RTC::RegisterState& ts()             { return ts_state; }
	RTC::RegisterState& tdstatuswrite()  { return tdstatuswrite_state; }
	RTC::RegisterState& tdstatusread()   { return tdstatusread_state; }
	RTC::RegisterState& tbbsettings()    { return tbbsettings_state; }
	RTC::RegisterState& tbbbandsel()     { return tbbbandsel_state; }
	RTC::RegisterState& bypasssettings() { return bypasssettings_state; }
	RTC::RegisterState& rawdatawrite()   { return rawdatawrite_state; }
	RTC::RegisterState& rawdataread()    { return rawdataread_state; }

	/*@}*/

private:
	// ----- data members -----
	RTC::RegisterState sys_state;            // RSR state
	RTC::RegisterState bf_state;             // BF weights state
	RTC::RegisterState ss_state;             // SS state
	RTC::RegisterState rcusettings_state;    // RCU settings state
	RTC::RegisterState rcuprotocol_state;    // RCU protocol state
	RTC::RegisterState hbaprotocol_state;    // HBA protocol state
	RTC::RegisterState rsuclear_state;       // RSU clear state
	RTC::RegisterState diagwgsettings_state; // DIAG WG settings state
	RTC::RegisterState sst_state;            // SST state
	RTC::RegisterState bst_state;            // BST state
	RTC::RegisterState xst_state;            // XST State
	RTC::RegisterState cdo_state;            // CDO state
	RTC::RegisterState bs_state;             // BS register state
	RTC::RegisterState tdclear_state;        // TDS register clear
	RTC::RegisterState tdwrite_state;        // TDS register write
	RTC::RegisterState tdread_state;         // TDS register read
	RTC::RegisterState rad_state;            // RAD register state
	RTC::RegisterState ts_state;             // RSR Timestamp register state
	RTC::RegisterState tdstatuswrite_state;  // TDS status write
	RTC::RegisterState tdstatusread_state;   // TDS status result
	RTC::RegisterState tbbsettings_state;    // TBB settings state
	RTC::RegisterState tbbbandsel_state;     // TBB bandsel state
	RTC::RegisterState bypasssettings_state; // Bypass (specinv) state
	RTC::RegisterState rawdatawrite_state;	 // Write userdefined datablock
	RTC::RegisterState rawdataread_state;	 // Read userdefined datablock

	int m_nrcus;
};

  }; // namespace RSP_Protocol
}; // namespace LOFAR

#endif /* ALLREGISTERSTATE_H_ */
