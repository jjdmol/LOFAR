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
#ifndef AOFLAGGER_QUALITYDATATEST_H
#define AOFLAGGER_QUALITYDATATEST_H

#include <AOFlagger/test/testingtools/asserter.h>
#include <AOFlagger/test/testingtools/unittest.h>

#include <AOFlagger/quality/qualitydata.h>
#include <AOFlagger/quality/statisticalvalue.h>

class QualityDataTest : public UnitTest {
	public:
    QualityDataTest() : UnitTest("Quality data")
		{
			AddTest(TestConstructor(), "Class constructor");
			AddTest(TestTableExists(), "Query table existance");
			AddTest(TestTableInitialization(), "Initialize tables");
			AddTest(TestKindOperations(), "Statistic kind operations");
			AddTest(TestStoreStatistics(), "Storing statistics");
		}
		
	private:
		struct TestConstructor : public Asserter
		{
			void operator()();
		};
		struct TestTableExists : public Asserter
		{
			void operator()();
		};
		struct TestTableInitialization : public Asserter
		{
			void operator()();
		};
		struct TestKindOperations : public Asserter
		{
			void operator()();
		};
		struct TestStoreStatistics : public Asserter
		{
			void operator()();
		};
};

void QualityDataTest::TestConstructor::operator()()
{
	casa::Table table("QualityTest.MS");
	QualityData *qd = new QualityData(table);
	delete qd;

	qd = new QualityData("QualityTest.MS");
	delete qd;
}

void QualityDataTest::TestTableExists::operator()()
{
	QualityData qd("QualityTest.MS");
	// undefined answer, but should not crash.
	qd.TableExists(QualityData::KindNameTable);
	qd.TableExists(QualityData::TimeStatisticTable);
	qd.TableExists(QualityData::FrequencyStatisticTable);
	qd.TableExists(QualityData::BaselineStatisticTable);
	qd.TableExists(QualityData::BaselineTimeStatisticTable);
	
	qd.RemoveAllQualityTables();
	AssertFalse(qd.TableExists(QualityData::KindNameTable));
	AssertFalse(qd.TableExists(QualityData::TimeStatisticTable));
	AssertFalse(qd.TableExists(QualityData::FrequencyStatisticTable));
	AssertFalse(qd.TableExists(QualityData::BaselineStatisticTable));
	AssertFalse(qd.TableExists(QualityData::BaselineTimeStatisticTable));
}

void QualityDataTest::TestTableInitialization::operator()()
{
	QualityData qd("QualityTest.MS");
	
	qd.RemoveAllQualityTables();
	
	enum QualityData::QualityTable tables[5] =
	{
		QualityData::KindNameTable,
		QualityData::TimeStatisticTable,
		QualityData::FrequencyStatisticTable,
		QualityData::BaselineStatisticTable,
		QualityData::BaselineTimeStatisticTable
	};
	for(unsigned i=0;i<5;++i)
	{
		qd.InitializeEmptyTable(tables[i]);
		AssertTrue(qd.TableExists(tables[i]), "Table exists after initialization");
	}
	
	for(unsigned i=0;i<5;++i)
	{
		qd.RemoveTable(tables[i]);
		for(unsigned j=0;j<=i;++j)
			AssertFalse(qd.TableExists(tables[j]), "Table removed and no longer exists");
		for(unsigned j=i+1;j<5;++j)
			AssertTrue(qd.TableExists(tables[j]), "Table initialized and not yet removed, thus exists");
	}
}

void QualityDataTest::TestKindOperations::operator()()
{
	QualityData qd("QualityTest.MS");
	
	qd.RemoveAllQualityTables();
	qd.InitializeEmptyTable(QualityData::KindNameTable);
	AssertTrue(qd.TableExists(QualityData::KindNameTable));
	
	unsigned kindIndex;
	AssertFalse(qd.QueryKindIndex(QualityData::MeanStatistic, kindIndex), "Empty table contains no index");
	
	unsigned originalKindIndex = qd.StoreKindName(QualityData::MeanStatistic);
	AssertTrue(qd.QueryKindIndex(QualityData::MeanStatistic, kindIndex), "Stored index is queried");
	AssertEquals(kindIndex, originalKindIndex, "Index returned by QueryKindIndex(Statistic, int)");
	AssertEquals(qd.QueryKindIndex(QualityData::MeanStatistic), originalKindIndex, "Index returned by QueryKindIndex(Statistic)");
	
	unsigned secondKindIndex = qd.StoreKindName(QualityData::VarianceStatistic);
	AssertNotEqual(originalKindIndex, secondKindIndex, "Store two kinds");
	AssertEquals(qd.QueryKindIndex(QualityData::MeanStatistic), originalKindIndex);
	AssertEquals(qd.QueryKindIndex(QualityData::VarianceStatistic), secondKindIndex);
	
	qd.InitializeEmptyTable(QualityData::KindNameTable);
	AssertFalse(qd.QueryKindIndex(QualityData::MeanStatistic, kindIndex), "Empty table contains no index after re-init");
}

void QualityDataTest::TestStoreStatistics::operator()()
{
	QualityData qd("QualityTest.MS");
	
	qd.RemoveAllQualityTables();
	AssertFalse(qd.IsStatisticAvailable(QualityData::TimeDimension, QualityData::MeanStatistic), "Statistic not available when table not exists");
	
	qd.InitializeEmptyTable(QualityData::KindNameTable);
	AssertFalse(qd.IsStatisticAvailable(QualityData::TimeDimension, QualityData::MeanStatistic), "Statistic not available when only kind-name table exists");
	
	qd.InitializeEmptyTable(QualityData::TimeStatisticTable);
	AssertFalse(qd.IsStatisticAvailable(QualityData::TimeDimension, QualityData::MeanStatistic), "Statistic not available when empty tables exist");
	
	unsigned meanStatIndex = qd.StoreKindName(QualityData::MeanStatistic);
	AssertFalse(qd.IsStatisticAvailable(QualityData::TimeDimension, QualityData::MeanStatistic), "Statistic not available when no entries in stat table");
	AssertEquals(qd.QueryStatisticEntryCount(QualityData::TimeDimension, meanStatIndex), 0u, "QueryStatisticEntryCount with zero entries");

	StatisticalValue value(4);
	value.SetKindIndex(meanStatIndex);
	value.SetValue(0, std::complex<float>(0.0, 1.0));
	value.SetValue(1, std::complex<float>(2.0, -2.0));
	value.SetValue(2, std::complex<float>(-3.0, 3.0));
	value.SetValue(3, std::complex<float>(-4.0, -4.0));
	qd.StoreTimeValue(60.0, 107000000.0, value);
	AssertTrue(qd.IsStatisticAvailable(QualityData::TimeDimension, QualityData::MeanStatistic), "Statistic available");
	AssertEquals(qd.QueryStatisticEntryCount(QualityData::TimeDimension, meanStatIndex), 1u, "QueryStatisticEntryCount with one entries");
	
	std::vector<std::pair<QualityData::TimePosition, StatisticalValue> > entries;
	qd.QueryTimeStatistic(meanStatIndex, entries);
	AssertEquals(entries.size(), (size_t) 1, "entries.size()");
	std::pair<QualityData::TimePosition, StatisticalValue> entry = entries[0];
	AssertEquals(entry.first.frequency, 107000000.0f, "frequency");
	AssertEquals(entry.first.time, 60.0f, "time");
	AssertEquals(entry.second.PolarizationCount(), 4u, "PolarizationCount()");
	AssertEquals(entry.second.KindIndex(), meanStatIndex, "KindIndex()");
	AssertEquals(entry.second.Value(0), std::complex<float>(0.0, 1.0), "Value(0)");
	AssertEquals(entry.second.Value(1), std::complex<float>(2.0, -2.0), "Value(1)");
	AssertEquals(entry.second.Value(2), std::complex<float>(-3.0, 3.0), "Value(2)");
	AssertEquals(entry.second.Value(3), std::complex<float>(-4.0, -4.0), "Value(3)");
	
	qd.RemoveTable(QualityData::KindNameTable);
	qd.RemoveTable(QualityData::TimeStatisticTable);
}

#endif
