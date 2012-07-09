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

#ifndef AOREMOTE__NODE_COMMAND_MAP_H
#define AOREMOTE__NODE_COMMAND_MAP_H

#include "clusteredobservation.h"

namespace aoRemote {

class NodeCommandMap
{
	public:
		/**
		 * Adds all measurement sets in the observation to the 'command list'. Each node
		 * will receive the list of measurement sets that are stored on the specific
		 * node.
		 */
		void Initialize(const ClusteredObservation &observation)
		{
			const std::vector<ClusteredObservationItem> &items = observation.GetItems();
			for(std::vector<ClusteredObservationItem>::const_iterator i=items.begin();i!=items.end();++i)
			{
				_nodeMap[i->HostName()].push_back(*i);
			}
		}
		
		/**
		 * Removes the top clustered observation item ('command') from the node map and
		 * returns it.
		 * @returns @c true when the node had another item ('command')
		 */
		bool GetNext(const std::string &hostname, ClusteredObservationItem &item)
		{
			NodeMap::iterator iter = _nodeMap.find(hostname);
			if(iter == _nodeMap.end())
			{
				return false;
			}
			else {
				std::deque<ClusteredObservationItem> &items = iter->second;
				if(items.empty())
				{
					_nodeMap.erase(iter);
					return false;
				}
				else
				{
					item = items.front();
					items.pop_front();
					return true;
				}
			}
		}
		
		/**
		 * Removes all commands that had to be executed for the given node.
		 * @returns @c true when the hostname was found and removed.
		 */
		bool RemoveNode(const std::string &hostname)
		{
			NodeMap::iterator iter = _nodeMap.find(hostname);
			if(iter == _nodeMap.end())
			{
				// There were no more commands for this client
				return false;
			}
			else {
				_nodeMap.erase(iter);
				return true;
			}
		}
		
		bool Empty() const
		{
			return _nodeMap.empty();
		}
		
		void NodeList(std::vector<std::string> &dest) const
		{
			dest.resize(_nodeMap.size());
			size_t p = 0;
			for(std::map<std::string, std::deque<ClusteredObservationItem> >::const_iterator i=_nodeMap.begin();i!=_nodeMap.end();++i)
			{
				dest[p] = i->first;
				++p;
			}
		}
		
		std::string CurrentFilename(const std::string &hostname) const
		{
			NodeMap::const_iterator iter = _nodeMap.find(hostname);
			return iter->second.front().LocalPath();
		}
	private:
		typedef std::map<std::string, std::deque<ClusteredObservationItem> > NodeMap;
		NodeMap _nodeMap;
};

}

#endif
