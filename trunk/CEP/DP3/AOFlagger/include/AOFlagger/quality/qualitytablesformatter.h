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

#define QUALITY_TABLES_VERSION      1
#define QUALITY_TABLES_VERSION_STR "1"

class QualityTablesFormatter {
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
			SumP3Statistic,
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
		
		struct TimePosition
		{
			double time;
			double frequency;
		};
		
		struct FrequencyPosition
		{
			double frequency;
		};
		
		struct BaselinePosition
		{
			unsigned antenna1;
			unsigned antenna2;
			double frequency;
		};
		
		struct BaselineTimePosition
		{
			double time;
			unsigned antenna1;
			unsigned antenna2;
			double frequency;
		};
		
		QualityTablesFormatter(const std::string &measurementSetName) :
			_measurementSet(0),
			_measurementSetName(measurementSetName),
			_kindNameTable(0),
			_timeTable(0),
			_frequencyTable(0),
			_baselineTable(0),
			_baselineTimeTable(0)
		{
		}
		
		~QualityTablesFormatter()
		{
			Close();
		}
		
		void Close()
		{
			if(_kindNameTable != 0)
				delete _kindNameTable;
			_kindNameTable = 0;
			if(_timeTable != 0)
				delete _timeTable;
			_timeTable = 0;
			if(_frequencyTable != 0)
				delete _frequencyTable;
			_frequencyTable = 0;
			if(_baselineTable != 0)
				delete _baselineTable;
			_baselineTable = 0;
			if(_baselineTimeTable != 0)
				delete _baselineTimeTable;
			_baselineTimeTable = 0;
			
			closeMainTable();
		}
		
		bool TableExists(enum QualityTable table) const
		{
			return _measurementSet->isReadable(TableToFilename(table));
		}
		
		static const std::string &KindToName(const enum StatisticKind kind)
		{
			return _kindToNameTable[(int) kind];
		}
		
		static enum StatisticKind NameToKind(const std::string &kindName);
		
		static const std::string &TableToName(const enum QualityTable table)
		{
			return _tableToNameTable[(int) table];
		}
		
		const std::string TableToFilename(const enum QualityTable table) const
		{
			return _measurementSetName + '/' + TableToName(table);
		}
		
		enum QualityTable DimensionToTable(const enum StatisticDimension dimension) const
		{
			return _dimensionToTableTable[(int) dimension];
		}
		
		bool IsStatisticAvailable(enum StatisticDimension dimension, enum StatisticKind kind)
		{
			QualityTable table = DimensionToTable(dimension);
			if(!TableExists(KindNameTable) || !TableExists(table))
				return false;
			unsigned kindIndex;
			if(!QueryKindIndex(kind, kindIndex))
				return false;
			return hasOneEntry(table, kindIndex);
		}
		
		void InitializeEmptyStatistic(enum StatisticDimension dimension, enum StatisticKind kind)
		{
			if(!TableExists(KindNameTable))
				InitializeEmptyTable(KindNameTable);
			
			QualityTable table = DimensionToTable(dimension);
			if(!TableExists(table))
				InitializeEmptyTable(table);
			else
			{
				removeStatisticFromStatTable(table, kind);
			}
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
				Close();
				openMainTable(true);
				_measurementSet->rwKeywordSet().removeField(TableToName(table));
				casa::Table::deleteTable(TableToFilename(table));
			}
		}
		
		void RemoveAllQualityTables()
		{
			RemoveTable(BaselineTimeStatisticTable);
			RemoveTable(BaselineStatisticTable);
			RemoveTable(FrequencyStatisticTable);
			RemoveTable(TimeStatisticTable);
			RemoveTable(KindNameTable);
		}
		
		unsigned StoreKindName(enum StatisticKind kind)
		{
			return StoreKindName(KindToName(kind));
		}

		unsigned StoreKindName(const std::string &name);
		
		void StoreTimeValue(double time, double frequency, const class StatisticalValue &value);
		void StoreFrequencyValue(double frequency, const class StatisticalValue &value);
		void StoreBaselineValue(unsigned antenna1, unsigned antenna2, double frequency, const class StatisticalValue &value);
		void StoreBaselineTimeValue(unsigned antenna1, unsigned antenna2, double time, double frequency, const class StatisticalValue &value);
		
		unsigned QueryKindIndex(enum StatisticKind kind);
		bool QueryKindIndex(enum StatisticKind kind, unsigned &destKindIndex);
		unsigned StoreOrQueryKindIndex(enum StatisticKind kind)
		{
			unsigned kindIndex;
			if(QueryKindIndex(kind, kindIndex))
				return kindIndex;
			else
				return StoreKindName(kind);
		}
		
		unsigned QueryStatisticEntryCount(enum StatisticDimension dimension, unsigned kindIndex);
		
		void QueryTimeStatistic(unsigned kindIndex, std::vector<std::pair<TimePosition, class StatisticalValue> > &entries);
		void QueryFrequencyStatistic(unsigned kindIndex, std::vector<std::pair<FrequencyPosition, class StatisticalValue> > &entries);
		void QueryBaselineStatistic(unsigned kindIndex, std::vector<std::pair<BaselinePosition, class StatisticalValue> > &entries);
		void QueryBaselineTimeStatistic(unsigned kindIndex, std::vector<std::pair<BaselineTimePosition, class StatisticalValue> > &entries);
	private:
		QualityTablesFormatter(const QualityTablesFormatter &) { } // don't allow copies
		void operator=(const QualityTablesFormatter &) { } // don't allow assignment
		
		const static std::string _kindToNameTable[];
		const static std::string _tableToNameTable[];
		const static enum QualityTable _dimensionToTableTable[];
		
		const static std::string ColumnNameAntenna1;
		const static std::string ColumnNameAntenna2;
		const static std::string ColumnNameFrequency;
		const static std::string ColumnNameKind;
		const static std::string ColumnNameName;
		const static std::string ColumnNameTime;
		const static std::string ColumnNameValue;
		
		casa::Table *_measurementSet;
		const std::string _measurementSetName;
		
		casa::Table *_kindNameTable;
		casa::Table *_timeTable;
		casa::Table *_frequencyTable;
		casa::Table *_baselineTable;
		casa::Table *_baselineTimeTable;
		
		bool hasOneEntry(enum QualityTable table, unsigned kindIndex);
		void removeStatisticFromStatTable(enum QualityTable table, enum StatisticKind kind);
		void removeKindNameEntry(enum StatisticKind kind);
		void removeEntries(enum QualityTable table);
		
		void addTimeColumn(casa::TableDesc &tableDesc);
		void addFrequencyColumn(casa::TableDesc &tableDesc);
		void addValueColumn(casa::TableDesc &tableDesc);
		
		void createTable(enum QualityTable table)
		{
			switch(table)
			{
				case KindNameTable:              createKindNameTable(); break;
				case TimeStatisticTable:         createTimeStatisticTable(); break;
				case FrequencyStatisticTable:    createFrequencyStatisticTable(); break;
				case BaselineStatisticTable:     createBaselineStatisticTable(); break;
				case BaselineTimeStatisticTable: createBaselineTimeStatisticTable(); break;
				default: break;
			}
		}
		
		void createKindNameTable();
		void createTimeStatisticTable();
		void createFrequencyStatisticTable();
		void createBaselineStatisticTable();
		void createBaselineTimeStatisticTable();
		unsigned findFreeKindIndex(casa::Table &kindTable);
		
		void openMainTable(bool needWrite);
		void closeMainTable()
		{
			if(_measurementSet != 0)
				delete _measurementSet;
			_measurementSet = 0;
		}
		
		void openTable(QualityTable table, bool needWrite, casa::Table **tablePtr);
		void openKindNameTable(bool needWrite)
		{
			openTable(KindNameTable, needWrite, &_kindNameTable);
		}
		void openTimeTable(bool needWrite)
		{
			openTable(TimeStatisticTable, needWrite, &_timeTable);
		}
		void openFrequencyTable(bool needWrite)
		{
			openTable(FrequencyStatisticTable, needWrite, &_frequencyTable);
		}
		void openBaselineTable(bool needWrite)
		{
			openTable(BaselineStatisticTable, needWrite, &_baselineTable);
		}
		void openBaselineTimeTable(bool needWrite)
		{
			openTable(BaselineTimeStatisticTable, needWrite, &_baselineTimeTable);
		}
		casa::Table &getTable(QualityTable table, bool needWrite)
		{
			casa::Table **tablePtr = 0;
			switch(table)
			{
				case KindNameTable: tablePtr = &_kindNameTable; break;
				case TimeStatisticTable: tablePtr = &_timeTable; break;
				case FrequencyStatisticTable: tablePtr = &_frequencyTable; break;
				case BaselineStatisticTable: tablePtr = &_baselineTable; break;
				case BaselineTimeStatisticTable: tablePtr = &_baselineTimeTable; break;
			}
			openTable(table, needWrite, tablePtr);
			return **tablePtr;
		}
};

#endif
