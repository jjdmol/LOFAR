/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BASELINESTATISTICSMAP_H
#define BASELINESTATISTICSMAP_H

#include <vector>
#include <map>

#include "statistics.h"

class BaselineStatisticsMap
{
	public:
		BaselineStatisticsMap(unsigned polarizationCount) : _polarizationCount(polarizationCount)
		{
		}
		
		Statistics &GetStatistics(unsigned antenna1, unsigned antenna2)
		{
			OuterMap::iterator antenna1Map = _map.insert(OuterPair(antenna1, InnerMap())).first;
			InnerMap &innerMap = antenna1Map->second;
			InnerMap::iterator antenna2Value = innerMap.find(antenna2);
			Statistics *statistics;
			if(antenna2Value == innerMap.end())
			{
				// The baseline does not exist yet, create empty statistics.
				statistics = &(innerMap.insert(InnerPair(antenna2, Statistics(_polarizationCount))).first->second);
			} else {
				statistics = &antenna2Value->second;
			}
			return *statistics;
		}
		
		std::vector<std::pair<unsigned, unsigned> > BaselineList()
		{
			std::vector<std::pair<unsigned, unsigned> > list;
			for(OuterMap::const_iterator outerIter = _map.begin(); outerIter!=_map.end(); ++outerIter)
			{
				const unsigned antenna1 = outerIter->first;
				const InnerMap &innerMap = outerIter->second;
				
				for(InnerMap::const_iterator innerIter = innerMap.begin(); innerIter!=innerMap.end(); ++innerIter)
				{
					const unsigned antenna2 = innerIter->first;
					list.push_back(std::pair<unsigned, unsigned>(antenna1, antenna2));
				}
			}
			return list;
		}
		
		void Clear()
		{
			_map.clear();
		}
	private:
		typedef std::map<unsigned, Statistics> InnerMap;
		typedef std::pair<unsigned, Statistics> InnerPair;
		typedef std::map<unsigned, InnerMap > OuterMap;
		typedef std::pair<unsigned, InnerMap > OuterPair;
		
		OuterMap _map;
		unsigned _polarizationCount;
};

#endif

