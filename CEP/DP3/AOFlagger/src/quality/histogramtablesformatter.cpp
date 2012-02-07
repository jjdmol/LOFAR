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

#include <AOFlagger/quality/histogramtablesformatter.h>

#include <ms/MeasurementSets/MSColumns.h>

#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>

#include <measures/TableMeasures/TableMeasDesc.h>

#include <measures/Measures/MEpoch.h>

#include <AOFlagger/quality/statisticalvalue.h>

const std::string HistogramTablesFormatter::_typeToNameTable[] =
{
	"TotalStokesI",
	"TotalXX",
	"TotalXY",
	"TotalYX",
	"TotalYY",
	"RFIStokesI",
	"RFIXX",
	"RFIXY",
	"RFIYX",
	"RFIYY"
};

const std::string HistogramTablesFormatter::ColumnNameType      = "TYPE";
const std::string HistogramTablesFormatter::ColumnNameName      = "NAME";
const std::string HistogramTablesFormatter::ColumnNameCount     = "COUNT";
const std::string HistogramTablesFormatter::ColumnNameBinStart  = "BIN_START";
const std::string HistogramTablesFormatter::ColumnNameBinEnd    = "BIN_END";

unsigned HistogramTablesFormatter::QueryTypeIndex(enum StatisticKind kind)
{
	unsigned kindIndex;
	if(!QueryKindIndex(kind, kindIndex))
		throw std::runtime_error("getKindIndex(): Requested statistic kind not available.");
	return kindIndex;
}

bool HistogramTablesFormatter::QueryKindIndex(enum StatisticKind kind, unsigned &destKindIndex)
{
	openKindNameTable(false);
	casa::ROScalarColumn<int> kindColumn(*_kindNameTable, ColumnNameKind);
	casa::ROScalarColumn<casa::String> nameColumn(*_kindNameTable, ColumnNameName);
	const casa::String nameToFind(KindToName(kind));
	
	const unsigned nrRow = _kindNameTable->nrow();
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(nameColumn(i) == nameToFind)
		{
			destKindIndex = kindColumn(i);
			return true;
		}
	}
	return false;
}

bool HistogramTablesFormatter::hasOneEntry(enum QualityTable table, unsigned kindIndex)
{
	casa::Table &casaTable = getTable(table, false);
	casa::ROScalarColumn<int> kindColumn(casaTable, ColumnNameKind);
	
	const unsigned nrRow = casaTable.nrow();
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == (int) kindIndex)
			return true;
	}
	return false;
}

void HistogramTablesFormatter::createKindNameTable()
{
	casa::TableDesc tableDesc("QUALITY_KIND_NAME_TYPE", QUALITY_TABLES_VERSION_STR, casa::TableDesc::Scratch);
	tableDesc.comment() = "Couples the KIND column in the other quality tables to the name of a statistic (e.g. Mean)";
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameKind, "Index of the statistic kind"));
	tableDesc.addColumn(casa::ScalarColumnDesc<casa::String>(ColumnNameName, "Name of the statistic"));

	casa::SetupNewTable newTableSetup(TableToFilename(KindNameTable), tableDesc, casa::Table::New);
	casa::Table newTable(newTableSetup);
	openMainTable(true);
	_measurementSet->rwKeywordSet().defineTable(TableToName(KindNameTable), newTable);
}

void HistogramTablesFormatter::addTimeColumn(casa::TableDesc &tableDesc)
{
	casa::ScalarColumnDesc<double> timeDesc(ColumnNameTime, "Central time of statistic");
	tableDesc.addColumn(timeDesc);
	
	casa::TableMeasRefDesc measRef(casa::MEpoch::UTC);
	casa::TableMeasValueDesc measVal(tableDesc, ColumnNameTime);
	casa::TableMeasDesc<casa::MEpoch> mepochCol(measVal, measRef);
	mepochCol.write(tableDesc);
}

void HistogramTablesFormatter::addFrequencyColumn(casa::TableDesc &tableDesc)
{
	casa::ScalarColumnDesc<double> freqDesc(ColumnNameFrequency, "Central frequency of statistic bin");
	tableDesc.addColumn(freqDesc);
	
	casa::Unit hertzUnit("Hz");
	
	casa::TableQuantumDesc quantDesc(tableDesc, ColumnNameFrequency, hertzUnit);
	quantDesc.write(tableDesc);
}

unsigned HistogramTablesFormatter::StoreType(enum HistogramType type, unsigned polarizationIndex)
{
	openTypeTable(true);
	
	unsigned typeIndex = findFreeTypeIndex(*_typeTable);
	
	unsigned newRow = _typeTable->nrow();
	_typeTable->addRow();
	casa::ScalarColumn<int> typeColumn(*_typeTable, ColumnNameType);
	casa::ScalarColumn<int> polarizationColumn(*_typeTable, ColumnNamePolarization);
	casa::ScalarColumn<casa::String> nameColumn(*_typeTable, ColumnNameName);
	typeColumn.put(newRow, typeIndex);
	polarizationColumn.put(newRow, polarizationIndex);
	nameColumn.put(newRow, TypeToName(type));
	return typeIndex;
}

unsigned HistogramTablesFormatter::findFreeTypeIndex(casa::Table &typeTable)
{
	int maxIndex = 0;
	
	casa::ROScalarColumn<int> typeColumn(typeTable, ColumnNameType);
	
	const unsigned nrRow = typeTable.nrow();
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(typeColumn(i) > maxIndex)
			maxIndex = typeColumn(i);
	}
	return maxIndex + 1;
}

void HistogramTablesFormatter::openTable(enum TableType table, bool needWrite, casa::Table **tablePtr)
{
	if(*tablePtr == 0)
	{
		openMainTable(false);
		*tablePtr = new casa::Table(_measurementSet->keywordSet().asTable(TableName(table)));
		if(needWrite)
			(*tablePtr)->reopenRW();
	} else {
		if(needWrite && !(*tablePtr)->isWritable())
			(*tablePtr)->reopenRW();
	}
}

void HistogramTablesFormatter::StoreValue(unsigned typeIndex, double binStart, double binEnd, double count)
{
	openCountTable(true);
	
	unsigned newRow = _countTable->nrow();
	_countTable->addRow();
	
	// TODO maybe the columns need to be cached to avoid having to look them up for each store...
	casa::ScalarColumn<int> typeColumn(*_countTable, ColumnNameType);
	casa::ScalarColumn<double> binStartColumn(*_countTable, ColumnNameBinStart);
	casa::ScalarColumn<double> binEndColumn(*_countTable, ColumnNameBinEnd);
	casa::ScalarColumn<double> countColumn(*_countTable, ColumnNameCount);
	
	typeColumn.put(newRow, typeIndex);
	binStartColumn.put(newRow, binStart);
	binEndColumn.put(newRow, binEnd);
	countColumn.put(newRow, count);
}

void HistogramTablesFormatter::removeTypeEntry(enum HistogramType type, unsigned polarizationIndex)
{
	openTypeTable(true);
	casa::ScalarColumn<int> polarizationColumn(*_typeTable, ColumnNamePolarization);
	casa::ScalarColumn<casa::String> nameColumn(*_typeTable, ColumnNameName);
	
	const unsigned nrRow = _typeTable->nrow();
	const casa::String typeName(TypeToName(type));
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(nameColumn(i) == typeName && polarizationColumn(i) ==  polarizationIndex)
		{
			_typeTable->removeRow(i);
			break;
		}
	}
}

void HistogramTablesFormatter::removeEntries(enum TableKind table)
{
	casa::Table &casaTable = getTable(table, true);
	const unsigned nrRow = casaTable.nrow();
	for(int i=nrRow-1;i>=0;--i)
	{
		casaTable.removeRow(i);
	}
}

void HistogramTablesFormatter::QueryHistogram(unsigned typeIndex, std::vector<HistogramItem> &histogram)
{
	casa::Table &table(getTable(HistogramCountTable, false));
	const unsigned nrRow = table.nrow();
	
	casa::ROScalarColumn<int> typeColumn(table, ColumnNameType);
	casa::ROScalarColumn<double> binStartColumn(table, ColumnNameBinStart);
	casa::ROScalarColumn<double> binEndColumn(table, ColumnNameBinEnd);
	casa::ROScalarColumn<double> countColumn(table, ColumnNameCount);
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(typeColumn(i) == (int) typeIndex)
		{
			HistogramItem item;
			item.binStart = binStartColumn(i);
			item.binEnd = binEndColumn(i);
			item.count = countColumn(i);
			histogram.push_back(item);
		}
	}
}

void HistogramTablesFormatter::QueryFrequencyStatistic(unsigned kindIndex, std::vector<std::pair<FrequencyPosition, StatisticalValue> > &entries)
{
	casa::Table &table(getTable(FrequencyStatisticTable, false));
	const unsigned nrRow = table.nrow();
	
	casa::ROScalarColumn<double> frequencyColumn(table, ColumnNameFrequency);
	casa::ROScalarColumn<int> kindColumn(table, ColumnNameKind);
	casa::ROArrayColumn<casa::Complex> valueColumn(table, ColumnNameValue);
	
	int polarizationCount = valueColumn.columnDesc().shape()[0];
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == (int) kindIndex)
		{
			StatisticalValue value(polarizationCount);
			value.SetKindIndex(kindIndex);
			casa::Array<casa::Complex> valueArray = valueColumn(i);
			casa::Array<casa::Complex>::iterator iter = valueArray.begin();
			for(int p=0;p<polarizationCount;++p)
			{
				value.SetValue(p, *iter);
				++iter;
			}
			FrequencyPosition position;
			position.frequency = frequencyColumn(i);
			entries.push_back(std::pair<FrequencyPosition, StatisticalValue>(position, value));
		}
	}
}

void QualityTablesFormatter::QueryBaselineStatistic(unsigned kindIndex, std::vector<std::pair<BaselinePosition, StatisticalValue> > &entries)
{
	casa::Table &table(getTable(BaselineStatisticTable, false));
	const unsigned nrRow = table.nrow();
	
	casa::ROScalarColumn<int> antenna1Column(table, ColumnNameAntenna1);
	casa::ROScalarColumn<int> antenna2Column(table, ColumnNameAntenna2);
	casa::ROScalarColumn<double> frequencyColumn(table, ColumnNameFrequency);
	casa::ROScalarColumn<int> kindColumn(table, ColumnNameKind);
	casa::ROArrayColumn<casa::Complex> valueColumn(table, ColumnNameValue);
	
	int polarizationCount = valueColumn.columnDesc().shape()[0];
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == (int) kindIndex)
		{
			StatisticalValue value(polarizationCount);
			value.SetKindIndex(kindIndex);
			casa::Array<casa::Complex> valueArray = valueColumn(i);
			casa::Array<casa::Complex>::iterator iter = valueArray.begin();
			for(int p=0;p<polarizationCount;++p)
			{
				value.SetValue(p, *iter);
				++iter;
			}
			BaselinePosition position;
			position.antenna1 = antenna1Column(i);
			position.antenna2 = antenna2Column(i);
			position.frequency = frequencyColumn(i);
			entries.push_back(std::pair<BaselinePosition, StatisticalValue>(position, value));
		}
	}
}

void QualityTablesFormatter::openMainTable(bool needWrite)
{
	if(_measurementSet == 0)
	{
		if(needWrite)
			_measurementSet = new casa::Table(_measurementSetName, casa::Table::Update);
		else
			_measurementSet = new casa::Table(_measurementSetName);
	}
	else if(needWrite)
	{
		if(!_measurementSet->isWritable())
			_measurementSet->reopenRW();
	}
}
