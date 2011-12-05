//#  ResourceAllocator.h: singleton class; Registers allocation of resources on a station 
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

#ifndef RESOURCEALLOCATOR_H
#define RESOURCEALLOCATOR_H

#define BOOST_SP_USE_PTHREADS
#include <boost/shared_ptr.hpp>
#include <set>
#include <bitset>

namespace LOFAR {
  namespace APLCommon {

class LogicalDevice;

class ResourceAllocator
{
public:   
	typedef boost::shared_ptr<ResourceAllocator> ResourceAllocatorPtr;
	typedef boost::shared_ptr<LogicalDevice> 	 LogicalDevicePtr;
	typedef std::bitset<256> 					 TRcuSubset;

	~ResourceAllocator();
	static ResourceAllocatorPtr instance();

	// member functions
	bool claimSO	(ResourceAllocator::LogicalDevicePtr ld, 
					 uint16 							 priority, 
					 double 							 samplingFrequency);
	void releaseSO	(ResourceAllocator::LogicalDevicePtr ld);
	void logSOallocation() const;

	bool claimSRG	(ResourceAllocator::LogicalDevicePtr ld, 
					 uint16 							 priority, 
					 TRcuSubset 						 rcuSubset, 
					 int16 								 nyquistZone, 
					 uint8 								 rcuControl);
	void releaseSRG	(ResourceAllocator::LogicalDevicePtr ld);
	void logSRGallocation() const;

private:
	ResourceAllocator();

    // generic types
    struct TAllocation {
		LogicalDevicePtr ld;
		uint16           priority;

		TAllocation (LogicalDevicePtr _ld, uint16 _priority) :
			ld(_ld),
			priority(_priority)
		{ };

		//
		// The comparison operator used to order the priority queue.
		//
		struct higherPriority {
			bool operator()(const TAllocation &a, const TAllocation &b) const {
				return (a.priority < b.priority);
			}
		};
      
	};

	// SO types
	struct TSOAllocation : public TAllocation {
		double samplingFrequency;

		TSOAllocation( LogicalDevicePtr _ld, uint16 _priority, double _sf) :
			TAllocation(_ld, _priority),
			samplingFrequency(_sf)
		{ };

	};

	// SRG types
	struct TSRGAllocation : public TAllocation {
		TRcuSubset  rcuSubset;
		int16       nyquistZone;
		uint8       rcuControl;

		TSRGAllocation (LogicalDevicePtr _ld, uint16 _priority, 
						TRcuSubset _rcuSubset, int16 _nyquistZone, uint8 _rcuControl) :
			TAllocation(_ld, _priority),
			rcuSubset(_rcuSubset),
			nyquistZone(_nyquistZone),
			rcuControl(_rcuControl)
		{ };

	};

	struct TRcuSettings {
		int16 nyquistZone;
		uint8 rcuControl;

		TRcuSettings() : 
			nyquistZone(0), rcuControl(0) {};
		TRcuSettings(int16 n, uint8 r) : 
			nyquistZone(n), rcuControl(r) {};
	};

    // typedefs
    typedef std::map     <uint16, 		  TRcuSettings> 			   TRcuSettingsMap;
    typedef std::multiset<TSOAllocation,  TAllocation::higherPriority> TSOAllocationSet;
    typedef std::multiset<TSRGAllocation, TAllocation::higherPriority> TSRGAllocationSet;
    
    // private functions
    bool _matchSOparameters(const TSOAllocation& current, const TSOAllocation& requested);
    bool _matchSRGparameters(const TSRGAllocation& requested);
    void _addSRGallocation(TRcuSubset rcuSubset, int16 nyquistZone, uint8 rcuControl);
    
    // private data
    TSOAllocationSet  			m_allocatedSOSet;
    TSRGAllocationSet 			m_allocatedSRGSet;
    TRcuSubset        			m_allocatedSRGRcuSubset;
    TRcuSettingsMap   			m_allocatedSRGRcuSettingsMap;

	// admin members  
    static ResourceAllocatorPtr m_theInstance;
     
};

 } // namespace APLCommon
} // namespace LOFAR
#endif
