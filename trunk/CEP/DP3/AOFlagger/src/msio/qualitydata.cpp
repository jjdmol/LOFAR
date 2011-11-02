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

#include <AOFlagger/msio/qualitydata.h>

#include <stdexcept>

#include <ms/MeasurementSets/MSColumns.h>

#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>

#include <measures/TableMeasures/TableMeasDesc.h>

const std::string QualityData::_kindToNameTable[] =
{
	"RFIRatio",
	"FlaggedRatio",
	"Count",
	"Mean",
	"RFIMean",
	"RFICount",
	"SumP2",
	"SumP4",
	"Variance",
	"VarianceOfVariance",
	"Skewness",
	"Kurtosis",
	"SignalToNoise",
	"DMean",
	"DSumP2",
	"DSumP4",
	"DVariance",
	"DVarianceOfVariance",
	"DCount"
};

const std::string QualityData::_tableToNameTable[] =
{
	"QUALITY_KIND_NAME",
	"QUALITY_TIME_STATISTIC",
	"QUALITY_FREQUENCY_STATISTIC",
	"QUALITY_BASELINE_STATISTIC",
	"QUALITY_BASELINE_TIME_STATISTIC"
};

const std::string QualityData::ColumnNameFrequency = "FREQUENCY";
const std::string QualityData::ColumnNameKind      = "KIND";
const std::string QualityData::ColumnNameName      = "NAME";
const std::string QualityData::ColumnNameTime      = "TIME";
const std::string QualityData::ColumnNameValue     = "VALUE";

int QualityData::getKindIndex(enum StatisticKind kind) const
{
	casa::Table table(TableToName(KindNameTable));
	casa::ScalarColumn<int> kindColumn(table, _KindColumnName);
	casa::ScalarColumn<casa::String> nameColumn(table, _NameColumnName);
	const casa::String nameToFind(KindToName(kind));
	
	const unsigned nrRow = table.nrow();
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(nameColumn(i) == nameToFind)
			return kindColumn(i);
	}
	throw std::runtime_error("getKindIndex(): Requested statistic kind not available.");
}

bool QualityData::hasOneEntry(enum QualityTable table, int kindIndex) const
{
	casa::Table casaTable(TableToName(table));
	casa::ScalarColumn<int> kindColumn(casaTable, _KindColumnName);
	
	const unsigned nrRow = casaTable.nrow();
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == kindIndex)
			return true;
	}
	return false;
}

void QualityData::createKindNameTable()
{
	casa::TableDesc tableDesc("QUALITY_KIND_NAME_TYPE", QUALITY_DATA_VERSION_STR, casa::TableDesc::New);
	tableDesc.comment() = "Couples the KIND column in the other quality tables to the name of a statistic (e.g. Mean)";
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameKind, "Index of the statistic kind"));
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameName, "Name of the statistic"));

	SetupNewTable newTabSetup(TableToName(KindNameTable), tableDesc, Table::New);
	StandardStMan standardStMan;
	newTabSetup.bindAll(standardStMan);
	casa::Table(newTabSetup);
}

void QualityData::addTimeColumn(casa::TableDesc &tableDesc)
{
	casa::ScalarColumnDesc<double> timeDesc(ColumnNameTime, "Central time of statistic");
	casa::TableMeasRefDesc measRef(MEpoch::UTC);
	casa::TableMeasValueDesc measVal(timeDesc, "Time");
	casa::TableMeasDesc<MEpoch> mepochCol(measVal, measRef);
	// "write makes the Measure column persistent"
	mepochCol.write(timeDesc);
	tableDesc.addColumn(timeDesc);
}

void QualityData::addFrequencyColumn(casa::TableDesc &tableDesc)
{
	casa::ScalarColumnDesc<double> freqDesc(ColumnNameFrequency, "Central frequency of statistic bin");
	tableDesc.addColumn(freqDesc);
}

void QualityData::addValueColumn(casa::TableDesc &tableDesc)
{
	const casa::IPosition shape(1);
	shape[0] = 4;
	casa::ArrayColumnDesc<casa::Complex> valDesc(ColumnNameValue, "Value of statistic", shape, casa::ColumnDesc::Direct);
	tableDesc.addColumn(valDesc);
}

void QualityData::createTimeStatisticTable()
{
	casa::TableDesc tableDesc("QUALITY_KIND_NAME_TYPE", QUALITY_DATA_VERSION_STR, casa::TableDesc::New);
	tableDesc.comment() = "Statistics over time";
	
	addTimeColumn(tableDesc);
	addFrequencyColumn(tableDesc);
	addValueColumn(tableDesc);
	
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameKind, "Index of the statistic kind"));
	tableDesc.addColumn(casa::ArrayColumnDesc<casa::Complex>());
}

