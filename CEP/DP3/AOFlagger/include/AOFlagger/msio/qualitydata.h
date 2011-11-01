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
#ifndef MSIO_QUALITY_DATA_H
#define MSIO_QUALITY_DATA_H

#include <ms/MeasurementSets/MeasurementSet.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/

struct StatisticalValue {
	StatisticalValue(unsigned _polarizationCount) :
		polarizationCount(_polarizationCount),
		values(new std::complex<float>[_polarizationCount])
	{
	}
	
	StatisticalValue(const StatisticalValue &source) :
		polarizationCount(source.polarizationCount),
		values(new std::complex<float>[source.polarizationCount])
	{
		kindIndex = source.kindIndex;
		for(unsigned i=0;i<polarizationCount;++i)
			values[i] = source.values[i];
	}
	
	~StatisticalValue()
	{
		delete[] values;
	}
	
	StatisticalValue &operator=(const StatisticalValue &source)
	{
		if(polarizationCount != source.polarizationCount)
		{
			polarizationCount = source.polarizationCount;
			delete[] values;
			values = new std::complex<float>[polarizationCount];
		}
		kindIndex = source.kindIndex;
		for(unsigned i=0;i<polarizationCount;++i)
			values[i] = source.values[i];
		return *this;
	}
	
	unsigned polarizationCount;
	int kindIndex;
	std::complex<float> *values;
};

class QualityData {
	public:
		enum StatisticKind
		{
			RFIRatioStatistic,
			FlaggedRatioStatistic,
			CountStatistic,
			MeanStatistic,
			RFIMeanStatistic,
			RFICountStatistic,
			SumP2Statistic,
			SumP4Statistic,
			VarianceStatistic,
			VarianceOfVarianceStatistic,
			SkewnessStatistic,
			KurtosisStatistic,
			SignalToNoiseStatistic,
			DMeanStatistic,
			DSumP2Statistic,
			DSumP4Statistic,
			DVarianceStatistic,
			DVarianceOfVarianceStatistic,
			DCountStatistic
		};
		
		enum StatisticDimension
		{
			TimeDimension,
			FrequencyDimension,
			BaselineDimension,
			BaselineTimeDimension
		};
		
		enum QualityTable
		{
			KindNameTable,
			TimeStatisticTable,
			FrequencyStatisticTable,
			BaselineStatisticTable,
			BaselineTimeStatisticTable
		};
		
		QualityData(casa::Table &measurementSet)
		{
			_measurementSet = new casa::Table(measurementSet);
		}
		
		QualityData(const std::string &measurementSetName)
		{
			_measurementSet = new casa::Table(measurementSetName);
		}
		
		~QualityData()
		{
			delete _measurementSet;
		}
		
		bool TableExists(enum QualityTable table) const
		{
			return _measurementSet->isReadable(TableToName(table));
		}
		
		const std::string &KindToName(const enum StatisticKind kind) const
		{
			return _kindToNameTable[(int) kind];
		}
		
		const std::string &TableToName(const enum QualityTable table) const
		{
			return _tableToNameTable[(int) table];
		}
		
		bool StatisticAvailable(enum QualityTable table, enum StatisticKind kind) const
		{
			return TableExists(KindNameTable) && TableExists(table) && hasOneEntry(table, kind);
		}
		
		void InitializeEmptyStatistic(enum QualityTable table, enum StatisticKind kind)
		{
			if(!TableExists(table))
				InitializeEmptyTable(table);
			else
				removeStatistic(table, kind);
		}
		
		void InitializeEmptyTable(enum QualityTable table)
		{
			if(TableExists(table))
				removeEntries(table);
			else
				createTable(table);
		}
		
		void RemoveTable(enum QualityTable table)
		{
			if(TableExists(table))
			{
				casa::Table::deleteTable(TableToName(table));
			}
		}
		
		void RemoveAllStatistics()
		{
			RemoveTable(BaselineTimeStatisticTable);
			RemoveTable(BaselineStatisticTable);
			RemoveTable(FrequencyStatisticTable);
			RemoveTable(TimeStatisticTable);
			RemoveTable(KindNameTable);
		}
		
		int StoreKindName(enum StatisticKind kind);
		
		void StoreTimeValue(double time, double frequency, const StatisticalValue &value);
		
	private:
		casa::Table *_measurementSet;
		
		const static std::string _kindToNameTable[];
		const static std::string _tableToNameTable[];
		
		bool hasOneEntry(enum QualityTable table, enum StatisticKind kind) const;
		void removeStatistic(enum QualityTable table, enum StatisticKind kind);
		void removeEntries(enum QualityTable table);
		void createTable(enum QualityTable table);
};

#endif
