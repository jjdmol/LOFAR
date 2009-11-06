//#  ResourceAllocator.cc: 
//#
//#  Copyright (C) 2002-2005
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
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_sstream.h>

#include <unistd.h>

#include <APL/APLCommon/ResourceAllocator.h>
#include <APL/APLCommon/LogicalDevice.h>
#include <APL/APLCommon/APL_Defines.h>

using namespace std;

namespace LOFAR {
 namespace APLCommon {

ResourceAllocator::ResourceAllocatorPtr ResourceAllocator::m_theInstance;

//
// ResourceAllocator()
//
ResourceAllocator::ResourceAllocator() :
	m_allocatedSOSet(),
	m_allocatedSRGSet(),
	m_allocatedSRGRcuSubset()
{ }

//
// ~ResourceAllocator()
//
ResourceAllocator::~ResourceAllocator()
{ }

//
// instance()
//
ResourceAllocator::ResourceAllocatorPtr ResourceAllocator::instance()
{
	if (0 == m_theInstance) {    
		m_theInstance.reset(new ResourceAllocator);
	}

	return (m_theInstance);
}

//
// claimSO(LogicalDevicePtr, prio, sampleFreq)
//
bool ResourceAllocator::claimSO(ResourceAllocator::LogicalDevicePtr ld, uint16 priority, double samplingFrequency)
{
	LOG_TRACE_FLOW_STR("claimSO(" << priority << "," << samplingFrequency << ")");

	bool allocated = false;

	TSOAllocation allocationRequest(ld, priority, samplingFrequency);
	if (m_allocatedSOSet.size() == 0) {		// first allocation request?
		// simply insert request.
		m_allocatedSOSet.insert(allocationRequest);
		allocated = true;
	}
	else {
		// check required parameters against current parameters
		TSOAllocationSet::iterator currentAllocation = m_allocatedSOSet.begin();
		if (_matchSOparameters(*currentAllocation, allocationRequest)) {
			// required parameters match. Add the LD to the allocation
			m_allocatedSOSet.insert(allocationRequest);
			allocated=true;
		}
		else {
			// required parameters do not match. Check priority and suspend 
			// the lower priority LD's
			if (priority < currentAllocation->priority) {
				m_allocatedSOSet.insert(allocationRequest);
				allocated=true;

				// higher priority, 
				//    suspend lower priority LD's with different settings, 
				//    resume lower priority LD's with the same settings
				TSOAllocationSet::iterator it = m_allocatedSOSet.begin();
				++it; // the first item was just added and has the highest priority.
				while (it != m_allocatedSOSet.end()) {
					if (_matchSOparameters(allocationRequest, *it)) { 
						// *it is now the requested allocation, 
						// because the other one has a higher priority
						LOG_INFO(formatString("Resuming LogicalDevice %s",it->ld->getName().c_str()));
#ifndef TESTBUILD
						it->ld->resume();
#endif
					}
					else {
						LOG_INFO(formatString("Suspending LogicalDevice %s",it->ld->getName().c_str()));
#ifndef TESTBUILD
						it->ld->suspend(LD_RESULT_LOW_PRIORITY);
#endif
					}
					++it;
				}
			}
		} 
	}
	LOG_INFO(formatString("Resource claim %sgranted",(allocated?"":"NOT ")));
	LOG_INFO(formatString("%d SO's allocated",m_allocatedSOSet.size()));

	return (allocated);
}

//
// releaseSO(logicalDevicePtr)
//
void ResourceAllocator::releaseSO(ResourceAllocator::LogicalDevicePtr ld)
{
	LOG_TRACE_FLOW("releaseSO");

	// remove the LD from the queue
	bool found(false);
	TSOAllocationSet::iterator it = m_allocatedSOSet.begin();
	while (!found && it != m_allocatedSOSet.end()) {
		if (it->ld == ld) {
			m_allocatedSOSet.erase(it);
			found = true;
		}
		else {
			++it;
		}
	}

	// resume all LD's with the same samplingfrequency as the highest priority one
	TSOAllocationSet::iterator currentAllocation = m_allocatedSOSet.begin();
	it = m_allocatedSOSet.begin();
	while (it != m_allocatedSOSet.end()) {
		if (_matchSOparameters(*currentAllocation, *it)) {
			LOG_INFO(formatString("Resuming LogicalDevice %s",it->ld->getName().c_str()));
#ifndef TESTBUILD
			it->ld->resume();
#endif
		}
		++it;
	}

	LOG_INFO(formatString("%d SO's allocated",m_allocatedSOSet.size()));
}

//
// logSOallocation()
//
void ResourceAllocator::logSOallocation() const
{
	stringstream logStream;
	logStream << "SO allocation log" << endl;
	logStream << "LD name, priority, sampling frequency" << endl;

	for (TSOAllocationSet::iterator it = m_allocatedSOSet.begin(); 
									it != m_allocatedSOSet.end(); ++it) {
#ifndef TESTBUILD
		logStream << it->ld->getName().c_str() << ", ";
#endif
		logStream << it->priority << ", ";
		logStream << it->samplingFrequency << endl;
	}

	LOG_DEBUG(logStream.str().c_str());
}

//
// _matchSOparameters(currentTSOAlloc, requestedTSOAlloc)
//
bool ResourceAllocator::_matchSOparameters(const TSOAllocation& current, 
										   const TSOAllocation& requested)
{
 	return (current.samplingFrequency == requested.samplingFrequency);
}


//
// claimSRG(LogicalDevicePtr, prio, rcuSubset, nyquistZone, rcuControl)
//
bool ResourceAllocator::claimSRG(ResourceAllocator::LogicalDevicePtr 	ld, 
								 uint16 								priority, 
								 TRcuSubset 							rcuSubset, 
								 int16 									nyquistZone, 
								 uint8 									rcuControl)
{
	LOG_TRACE_FLOW("claimSRG");

	bool allocated = false;
	TSRGAllocation allocationRequest(ld, priority, rcuSubset, nyquistZone, rcuControl);

	if (m_allocatedSRGSet.size() == 0) {		// first claim request?
		// simply insert.
		m_allocatedSRGSet.insert(allocationRequest);
		_addSRGallocation(rcuSubset,nyquistZone,rcuControl);
		allocated=true;
	}
	else {
		// check required parameters against current parameters
		if (_matchSRGparameters(allocationRequest)) {
			// required parameters match. Add the LD to the allocation
			m_allocatedSRGSet.insert(allocationRequest);
			_addSRGallocation(rcuSubset,nyquistZone,rcuControl);
			allocated=true;
		}
		else {
			// required parameters do not match. Check priority and suspend 
			// the lower priority LD's
			if (priority < m_allocatedSRGSet.begin()->priority) {
				m_allocatedSRGSet.insert(allocationRequest);
				m_allocatedSRGRcuSubset = 0;  // rebuild 
				m_allocatedSRGRcuSettingsMap.clear();
				_addSRGallocation(rcuSubset,nyquistZone,rcuControl);

				// higher priority, 
				//    suspend lower priority LD's with different settings, 
				//    resume lower priority LD's with the same settings
				TSRGAllocationSet::iterator it = m_allocatedSRGSet.begin();
				++it; // the first item was just added and has the highest priority.
				while (it != m_allocatedSRGSet.end()) {
					if (_matchSRGparameters(*it)) {
						LOG_INFO(formatString("Resuming LogicalDevice %s",it->ld->getName().c_str()));
#ifndef TESTBUILD
						it->ld->resume();
#endif
						_addSRGallocation(rcuSubset, nyquistZone, rcuControl);
					}
					else {
						LOG_INFO(formatString("Suspending LogicalDevice %s",it->ld->getName().c_str()));
#ifndef TESTBUILD
						it->ld->suspend(LD_RESULT_LOW_PRIORITY);
#endif
					}
					++it;
				}
			}
		} 
	}

	LOG_INFO(formatString("Resource claim %sgranted",(allocated?"":"NOT ")));
	LOG_INFO(formatString("%d SRG's allocated",m_allocatedSRGSet.size()));

	return allocated;
}

//
// releaseSRG(LogicalDevicePtr)
//
void ResourceAllocator::releaseSRG(ResourceAllocator::LogicalDevicePtr ld)
{
	LOG_TRACE_FLOW("releaseSRG");

	// remove the LD from the queue
	bool found(false);
	TSRGAllocationSet::iterator it = m_allocatedSRGSet.begin();
	while(!found && it != m_allocatedSRGSet.end()) {
		if(it->ld == ld) {
			m_allocatedSRGSet.erase(it);
			found = true;
		}
		else {
			++it;
		}
	}

	// resume all LD's with non-overlapping parameters with the highest priority one
	m_allocatedSRGRcuSubset = 0;
	m_allocatedSRGRcuSettingsMap.clear();
	it = m_allocatedSRGSet.begin();
	while (it != m_allocatedSRGSet.end()) {
		if (_matchSRGparameters(*it)) {
			LOG_INFO(formatString("Resuming LogicalDevice %s",it->ld->getName().c_str()));
#ifndef TESTBUILD
			it->ld->resume();
#endif
			_addSRGallocation(it->rcuSubset, it->nyquistZone, it->rcuControl);
		}
		++it;
	}

	LOG_INFO(formatString("%d SRG's allocated",m_allocatedSRGSet.size()));
}

//
// logSRGallocation()
//
void ResourceAllocator::logSRGallocation() const
{
	stringstream logStream;
	logStream << "SRG allocation log" << endl;
	logStream << "LD name, priority, nyquistZone, rcuControl, rcu subset" << endl;

	for (TSRGAllocationSet::iterator it = m_allocatedSRGSet.begin(); 
									 it != m_allocatedSRGSet.end(); ++it) {
#ifndef TESTBUILD
		logStream << it->ld->getName().c_str() << ", ";
#endif
		logStream << it->priority << ", ";
		logStream << it->nyquistZone << ", ";
		logStream << "0x" << std::hex << static_cast<uint16>(it->rcuControl) << ", " << std::dec;
		logStream << it->rcuSubset.to_string<char,char_traits<char>,allocator<char> >().c_str() << endl;
	}

	LOG_DEBUG(logStream.str().c_str());
}

//
// _matchSRGparameters(requestedSRGAlloc)
//
bool ResourceAllocator::_matchSRGparameters(const TSRGAllocation& requested)
{

	if(!(requested.rcuSubset & m_allocatedSRGRcuSubset)) {	// no overlap?
		return (true);
	}

	// overlapping rcu usage. Check settings
	for (size_t i = 0; i < requested.rcuSubset.size(); i++) {
		// check overlap
		if (requested.rcuSubset.test(i) && m_allocatedSRGRcuSubset.test(i)) {
			// check settings
			if(m_allocatedSRGRcuSettingsMap[i].nyquistZone != requested.nyquistZone ||
				m_allocatedSRGRcuSettingsMap[i].rcuControl  != requested.rcuControl) {
				return (false);
			}
		}
	}

	return (true);
}    

//
// _addSRGallocation(rcuSubset, nyquistZone, rcuControl)
//
void ResourceAllocator::_addSRGallocation(TRcuSubset 	rcuSubset, 
										  int16 		nyquistZone, 
										  uint8 		rcuControl)
{
	m_allocatedSRGRcuSubset |= rcuSubset;
	for (size_t i = 0; i < rcuSubset.size(); i++) {
		if (rcuSubset.test(i)) {
			m_allocatedSRGRcuSettingsMap[i] = TRcuSettings(nyquistZone,rcuControl);
		}
	}
}


 } // namespace APLCommon
} // namespace LOFAR
