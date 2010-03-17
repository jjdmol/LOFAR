//#  -*- mode: c++ -*-
//#
//#  Cache.h: RSP Driver data cache
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

#ifndef CACHE_H_
#define CACHE_H_

#include <Common/LofarTypes.h>
#include <blitz/array.h>
#include <APL/RSP_Protocol/AllRegisterState.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include "SerdesBuffer.h"

namespace LOFAR {
  namespace RSP {

class Cache; // forward declaration

typedef struct {
	uint32		address;
	uint16		offset;
	uint16		dataLen;
	uint8		data [RSP_RAW_BLOCK_SIZE];
} RawDataBlock_t;

typedef enum {
	NONE = 0,
	HBA,
	RCU_R,
	RCU_W
} I2Cuser;

class CacheBuffer
{
public:
	// Constructors for a Cache object.
	CacheBuffer(Cache* cache);

	// Destructor for Cache. 
	virtual ~CacheBuffer();

	// Reset cache to default values.
	// Also called by constructor to initialize the cache.
	void reset(void);

	/*@{*/
	// Data access methods.
	RTC::Timestamp 			getTimestamp() const 	{ return m_timestamp; } 
	BeamletWeights& 		getBeamletWeights() 	{ return m_beamletweights; } 
	SubbandSelection& 		getSubbandSelection() 	{ return m_subbandselection; } 
	RCUSettings& 			getRCUSettings() 		{ return m_rcusettings; } 
	HBASettings& 			getHBASettings() 		{ return m_hbasettings; } 
	HBASettings& 			getHBAReadings() 		{ return m_hbareadings; } 
	RSUSettings& 			getRSUSettings() 		{ return m_rsusettings; } 
	WGSettings& 			getWGSettings() 		{ return m_wgsettings; } 
	SystemStatus& 			getSystemStatus() 		{ return m_systemstatus; } 
	Statistics& 			getSubbandStats() 		{ return m_subbandstats; } 
	Statistics& 			getBeamletStats() 		{ return m_beamletstats; } 
	XCStatistics& 			getXCStats() 			{ return m_xcstats; } 
	Versions& 				getVersions() 			{ return m_versions; } 
	uint32& 				getClock() 				{ return m_clock; } 
	TDStatus& 				getTDStatus() 			{ return m_tdstatus; } 
	SPUStatus& 				getSPUStatus() 			{ return m_spustatus; } 
	TBBSettings& 			getTBBSettings() 		{ return m_tbbsettings; } 
	BypassSettings& 		getBypassSettings()		{ return m_bypasssettings; } 
	RawDataBlock_t&			getRawDataBlock() 		{ return (itsRawDataBlock); } 
	SerdesBuffer&			getSdsWriteBuffer() 	{ return (itsSdsWriteBuffer); }
	SerdesBuffer&			getSdsReadBuffer(int rspBoardNr);
	Latency&				getLatencys()			{ return (itsLatencys); }
	
	bool isSplitterActive() { return(itsSplitterActive); }
	void setSplitterActive(bool active) { itsSplitterActive = active; }

	bool isCepEnabled() { return(itsCepEnabled); }
	void setCepEnabled(bool enable) { itsCepEnabled = enable; }

	I2Cuser getI2Cuser() { return (itsI2Cuser); }
	void setI2Cuser(I2Cuser user) { itsI2Cuser = user; }
	/*@}*/

	// update timestamp
	void setTimestamp(const RTC::Timestamp& timestamp);

	// Get const pointer to parent cache.
	Cache& getCache() { return *m_cache; }

private:
	// NOTE [reo]: The relation between the RSPprotocol classes,
	//	the EPAProtocol classes and the cache is not implemented 
	//	in the right way. The Cache should consist of (blitz)
	//	arrays of EPA-structures and the RSP classes should
	//	contain a single element of the RSP equivalent of the
	//	EPA information.
	//	In the current implementation the RSP class iso the cache
	//	contains the blitz array. This is very confusing since it
	//	indicates that the RSP class can contain many elements,
	//	which is never the case.

	CacheBuffer(); // prevent default construction

	// --- datamembers ---
	RTC::Timestamp					m_timestamp;
	I2Cuser							itsI2Cuser;
	RSP_Protocol::BeamletWeights	m_beamletweights;
	RSP_Protocol::SubbandSelection	m_subbandselection;
	RSP_Protocol::RCUSettings		m_rcusettings;
	RSP_Protocol::HBASettings		m_hbasettings;
	RSP_Protocol::HBASettings		m_hbareadings;
	RSP_Protocol::RSUSettings		m_rsusettings;
	RSP_Protocol::WGSettings		m_wgsettings;
	RSP_Protocol::Statistics		m_subbandstats;
	RSP_Protocol::Statistics		m_beamletstats;
	RSP_Protocol::XCStatistics		m_xcstats;
	RSP_Protocol::SystemStatus		m_systemstatus;
	RSP_Protocol::Versions			m_versions;
	uint32							m_clock;
	RSP_Protocol::TDStatus			m_tdstatus;
	RSP_Protocol::SPUStatus			m_spustatus;
	RSP_Protocol::TBBSettings		m_tbbsettings;
	RSP_Protocol::BypassSettings	m_bypasssettings;
	RawDataBlock_t					itsRawDataBlock;
	SerdesBuffer					itsSdsWriteBuffer;
	SerdesBuffer					itsSdsReadBuffer[MAX_N_RSPBOARDS];
	bool							itsSplitterActive;
	bool							itsCepEnabled;
	RSP_Protocol::Latency			itsLatencys;
	
	Cache* m_cache;		// pointer to container
};

// Singleton class containing the data caches.
class Cache
{
public:
	/*@{*/
	// Constructor/destructor
	static Cache& getInstance();
	virtual ~Cache();
	/*@}*/

	// Reset cache front and back buffers.
	void reset(void);
	void resetI2Cuser(void);

	// Swap the front and back buffers.
	void swapBuffers();

	// Get front/back buffers.
	CacheBuffer& getFront() { return (*m_front); }
	CacheBuffer& getBack()  { return (*m_back);  }

	// Get register states.
	AllRegisterState& getState() { return m_allstate; }

private:
	// Direct construction not allowed.
	Cache();

	// Keep register update state.
	AllRegisterState m_allstate; // communication status of all register

	/*@{*/
	// Front and back buffers.
	CacheBuffer* m_front;
	CacheBuffer* m_back;
	/*@}*/

	// Singleton class.
	static Cache* m_instance;
};

  }; // namespace 
}; // namespace LOFAR
     
#endif /* CACHE_H_ */
