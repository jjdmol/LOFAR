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
#ifndef DIRECTBASELINEREADER_H
#define DIRECTBASELINEREADER_H

#include <map>
#include <vector>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include <AOFlagger/msio/antennainfo.h>
#include <AOFlagger/msio/baselinereader.h>
#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/mask2d.h>
#include <AOFlagger/msio/measurementset.h>

typedef boost::shared_ptr<class BaselineReader> BaselineReaderPtr;
typedef boost::shared_ptr<const class BaselineReader> BaselineReaderCPtr;

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class DirectBaselineReader : public BaselineReader {
	public:
		explicit DirectBaselineReader(const std::string &msFile);
		~DirectBaselineReader();

		void PerformReadRequests();
		void PerformWriteRequests();
		
		void PartInfo(size_t maxTimeScans, size_t &timeScanCount, size_t &partCount);
		void ShowStatistics();
	private:
		struct BaselineCacheItem
		{
			BaselineCacheItem() { }
			BaselineCacheItem(const BaselineCacheItem &source)
			: antenna1(source.antenna1), antenna2(source.antenna2), spectralWindow(source.spectralWindow), rows(source.rows)
			{
			}
			void operator=(const BaselineCacheItem &source)
			{
				antenna1 = source.antenna1;
				antenna2 = source.antenna2;
				spectralWindow = source.spectralWindow;
				rows = source.rows;
			}
			
			int antenna1, antenna2, spectralWindow;
			std::vector<size_t> rows;
		};
		
		void initBaselineCache();
		
		void addRequestRows(ReadRequest request, size_t requestIndex, std::vector<std::pair<size_t, size_t> > &rows);
		void addRequestRows(WriteRequest request, size_t requestIndex, std::vector<std::pair<size_t, size_t> > &rows);
		void addRowToBaselineCache(int antenna1, int antenna2, int spectralWindow, size_t row);
		void readUVWData();

		casa::ROArrayColumn<casa::Complex> *CreateDataColumn(enum DataKind kind, class casa::Table &table);
		void readTimeData(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<casa::Complex> data, const casa::Array<casa::Complex> *model);
		void readTimeFlags(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<bool> flag);
		void readWeights(size_t requestIndex, size_t xOffset, int frequencyCount, const casa::Array<float> weight);

		std::vector<BaselineCacheItem> _baselineCache;
};

#endif // DIRECTBASELINEREADER_H
