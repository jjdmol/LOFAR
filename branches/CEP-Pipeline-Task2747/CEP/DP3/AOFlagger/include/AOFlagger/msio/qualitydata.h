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

#define QUALITY_DATA_VERSION      1
#define QUALITY_DATA_VERSION_STR "1"

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
		
		void StoreTimeValue(double time, double frequency, const class StatisticalValue &value);
		
	private:
		casa::Table *_measurementSet;
		
		const static std::string _kindToNameTable[];
		const static std::string _tableToNameTable[];
		
		const static std::string ColumnNameFrequency;
		const static std::string ColumnNameKind;
		const static std::string ColumnNameName;
		const static std::string ColumnNameTime;
		const static std::string ColumnNameValue;
		
		int getKindIndex(enum StatisticKind kind) const;
		bool hasOneEntry(enum QualityTable table, int kindIndex) const;
		void removeStatistic(enum QualityTable table, enum StatisticKind kind);
		void removeEntries(enum QualityTable table);
		
		void addTimeColumn(casa::TableDesc &tableDesc);
		void addFrequencyColumn(casa::TableDesc &tableDesc);
		void addValueColumn(casa::TableDesc &tableDesc);
		
		void createTable(enum QualityTable table)
		{
			switch(table)
			{
				case KindNameTable:      createKindNameTable(); break;
				case TimeStatisticTable: createTimeStatisticTable(); break;
				case TimeStatisticTable: createFrequencyStatisticTable(); break;
				case TimeStatisticTable: createBaselineStatisticTable(); break;
				case TimeStatisticTable: createBaselineTimeStatisticTable(); break;
				default: break;
			}
		}
		
		void createKindNameTable();
		void createTimeStatisticTable();
		void createFrequencyStatisticTable();
		void createBaselineStatisticTable();
		void createBaselineTimeStatisticTable();
};

#endif
