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
#ifndef INDIRECTBASELINEREADER_H
#define INDIRECTBASELINEREADER_H

#include <map>
#include <vector>
#include <stdexcept>

#include <AOFlagger/msio/baselinereader.h>
#include <AOFlagger/msio/directbaselinereader.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class IndirectBaselineReader : public BaselineReader {
	public:
		explicit IndirectBaselineReader(const std::string &msFile);
		~IndirectBaselineReader();

		void PerformReadRequests();
		void PerformWriteRequests();
		
		void ShowStatistics();
		virtual size_t GetMinRecommendedBufferSize(size_t /*threadCount*/) { return 1; }
		virtual size_t GetMaxRecommendedBufferSize(size_t /*threadCount*/) { return 2; }
	private:
		void reorderMS();
		void removeTemporaryFiles();

		DirectBaselineReader _directReader;
		bool _msIsReordered;
		size_t _maxMemoryUse;
};

#endif // INDIRECTBASELINEREADER_H
