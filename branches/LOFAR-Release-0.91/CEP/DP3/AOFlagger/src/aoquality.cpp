/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include <iostream>

#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableCopy.h>

#include <AOFlagger/msio/measurementset.h>

#include <AOFlagger/quality/defaultstatistics.h>
#include <AOFlagger/quality/histogramcollection.h>
#include <AOFlagger/quality/qualitytablesformatter.h>
#include <AOFlagger/quality/statisticscollection.h>
#include <AOFlagger/quality/statisticsderivator.h>

#include <AOFlagger/remote/clusteredobservation.h>
#include <AOFlagger/remote/processcommander.h>
#include <AOFlagger/util/plot.h>

#include <AOFlagger/configuration.h>

#ifdef HAS_LOFARSTMAN
#include <LofarStMan/Register.h>
#endif // HAS_LOFARSTMAN                                                       

void reportProgress(unsigned step, unsigned totalSteps)
{
	const unsigned twoPercent = (totalSteps+49)/50;
	if((step%twoPercent)==0)
	{
		if(((step/twoPercent)%5)==0)
			std::cout << (100*step/totalSteps) << std::flush;
		else
			std::cout << '.' << std::flush;
	}
}

enum CollectingMode
{
	CollectDefault,
	CollectHistograms
};

void actionCollect(const std::string &filename, enum CollectingMode mode)
{
	MeasurementSet *ms = new MeasurementSet(filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	const unsigned bandCount = ms->BandCount();
	const bool ignoreChannelZero = ms->ChannelZeroIsRubish();
	const std::string stationName = ms->GetStationName();
	BandInfo *bands = new BandInfo[bandCount];
	double **frequencies = new double*[bandCount];
	unsigned totalChannels = 0;
	for(unsigned b=0;b<bandCount;++b)
	{
		bands[b] = ms->GetBandInfo(b);
		frequencies[b] = new double[bands[b].channelCount];
		totalChannels += bands[b].channelCount;
		for(unsigned c=0;c<bands[b].channelCount;++c)
		{
			frequencies[b][c] = bands[b].channels[c].frequencyHz;
		}
	}
	delete ms;
	
	std::cout
		<< "Polarizations: " << polarizationCount << '\n'
		<< "Bands: " << bandCount << '\n'
		<< "Channels/band: " << (totalChannels / bandCount) << '\n'
		<< "Name of obseratory: " << stationName << '\n';
	if(ignoreChannelZero)
		std::cout << "Channel zero will be ignored, as this looks like a LOFAR data set with bad channel 0.\n";
	else
		std::cout << "Channel zero will be included in the statistics, as it seems that channel 0 is okay.\n";
	
	// Initialize statisticscollection
	StatisticsCollection collection(polarizationCount);
	if(mode == CollectDefault)
	{
		for(unsigned b=0;b<bandCount;++b)
		{
			if(ignoreChannelZero)
				collection.InitializeBand(b, (frequencies[b]+1), bands[b].channelCount-1);
			else
				collection.InitializeBand(b, frequencies[b], bands[b].channelCount);
		}
	}
	// Initialize Histograms collection
	HistogramCollection histogramCollection(polarizationCount);

	// get columns
	casa::Table table(filename, casa::Table::Update);
	const char *dataColumnName = "DATA";
	casa::ROArrayColumn<casa::Complex> dataColumn(table, dataColumnName);
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROScalarColumn<double> timeColumn(table, "TIME");
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");
	casa::ROScalarColumn<int> windowColumn(table, "DATA_DESC_ID");
	
	std::cout << "Collecting statistics..." << std::endl;
	
	const unsigned nrow = table.nrow();
	for(unsigned row = 0; row!=nrow; ++row)
	{
		const double time = timeColumn(row);
		const unsigned antenna1Index = antenna1Column(row);
		const unsigned antenna2Index = antenna2Column(row);
		const unsigned bandIndex = windowColumn(row);
		
		const BandInfo &band = bands[bandIndex];
		
		const casa::Array<casa::Complex> dataArray = dataColumn(row);
		const casa::Array<bool> flagArray = flagColumn(row);
		
		std::complex<float> *samples[polarizationCount];
		bool *isRFI[polarizationCount];
		for(unsigned p = 0; p < polarizationCount; ++p)
		{
			isRFI[p] = new bool[band.channelCount];
			samples[p] = new std::complex<float>[band.channelCount];
		}
		
		casa::Array<casa::Complex>::const_iterator dataIter = dataArray.begin();
		casa::Array<bool>::const_iterator flagIter = flagArray.begin();
		const unsigned startChannel = ignoreChannelZero ? 1 : 0;
		if(ignoreChannelZero)
		{
			for(unsigned p = 0; p < polarizationCount; ++p)
			{
				++dataIter;
				++flagIter;
			}
		}
		for(unsigned channel = startChannel ; channel<band.channelCount; ++channel)
		{
			for(unsigned p = 0; p < polarizationCount; ++p)
			{
				samples[p][channel - startChannel] = *dataIter;
				isRFI[p][channel - startChannel] = *flagIter;
				
				++dataIter;
				++flagIter;
			}
		}
		
		for(unsigned p = 0; p < polarizationCount; ++p)
		{
			switch(mode)
			{
				case CollectDefault:
					{
						const bool origFlags = false;
						collection.Add(antenna1Index, antenna2Index, time, bandIndex, p, &samples[p]->real(), &samples[p]->imag(), isRFI[p], &origFlags, band.channelCount - startChannel, 2, 1, 0);
					}
					break;
				case CollectHistograms:
					histogramCollection.Add(antenna1Index, antenna2Index, p, samples[p], isRFI[p], band.channelCount - startChannel);
					break;
			}
		}

		for(unsigned p = 0; p < polarizationCount; ++p)
		{
			delete[] isRFI[p];
			delete[] samples[p];
		}
		
		reportProgress(row, nrow);
	}
	
	for(unsigned b=0;b<bandCount;++b)
		delete[] frequencies[b];
	delete[] frequencies;
	delete[] bands;
	std::cout << "100\n";
	
	switch(mode)
	{
		case CollectDefault:
			{
				std::cout << "Writing quality tables..." << std::endl;
				
				QualityTablesFormatter qualityData(filename);
				collection.Save(qualityData);
			}
			break;
		case CollectHistograms:
			const std::map<HistogramCollection::AntennaPair, LogHistogram*> &map = histogramCollection.GetHistograms(0);
			Plot plotSlopes("histogram-slopes.pdf");
			plotSlopes.SetYRange(-10.0, 10.0);
			Plot plotHistograms("histograms.pdf");
			for(std::map<HistogramCollection::AntennaPair, LogHistogram*>::const_iterator i = map.begin(); i != map.end(); ++i)
			{
				if(i->first.first != i->first.second)
				{
					const LogHistogram *histogram = i->second;
					double rangeCentre = histogram->MinPositiveAmplitude();
					rangeCentre = exp2(floor(log2(rangeCentre)));
					const double maxAmplitude = histogram->MaxAmplitude();
					std::cout << "Antennae " << i->first.first << " x " << i->first.second << "\n";
					std::stringstream s;
					s << i->first.first << " x " << i->first.second;
					//plotSlopes.StartLine(s.str());
					//plotHistograms.StartLine(s.str());
					plotSlopes.StartLine();
					plotSlopes.SetLogScale(true, false);
					plotHistograms.StartLine();
					plotHistograms.SetLogScale(true, true);
					while(rangeCentre < maxAmplitude && rangeCentre > 0.0)
					{
						const double rangeStart = rangeCentre * 0.75;
						const double rangeEnd = rangeCentre * 1.5;
						const double slope = histogram->NormalizedSlope(rangeStart, rangeEnd, LogHistogram::TotalAmplitudeHistogram);
						std::cout << rangeStart << "-" << rangeEnd << ": " << slope << "\n";
						rangeCentre *= 2.0;
						plotSlopes.PushDataPoint(rangeCentre, slope);
						const double count = histogram->NormalizedCount(rangeStart, rangeEnd, LogHistogram::TotalAmplitudeHistogram);
						if(count > 0 && std::isfinite(count))
							plotHistograms.PushDataPoint(rangeCentre, count);
					}
				}
			}
			Plot plotFine("histogram-fine.pdf");
			Plot plotGlobalSlopes("histogram-gslopes.pdf");
			plotFine.SetLogScale(true, true);
			plotGlobalSlopes.SetLogScale(true, false);
			plotGlobalSlopes.SetYRange(-5.0, 5.0);
			LogHistogram intHistogram;
			histogramCollection.GetHistogramForCrossCorrelations(0, intHistogram);
			
			plotFine.StartLine("Total");
			plotGlobalSlopes.StartLine("Total");
			for(LogHistogram::iterator i=intHistogram.begin(); i!=intHistogram.end(); ++i)
			{
				plotFine.PushDataPoint(i.value(), i.normalizedCount(LogHistogram::TotalAmplitudeHistogram));
				plotGlobalSlopes.PushDataPoint(i.value(), intHistogram.NormalizedSlope(i.value()*0.5, i.value()*2.0, LogHistogram::TotalAmplitudeHistogram));
			}
			plotFine.StartLine("RFI");
			plotGlobalSlopes.StartLine("RFI");
			for(LogHistogram::iterator i=intHistogram.begin(); i!=intHistogram.end(); ++i)
			{
				plotFine.PushDataPoint(i.value(), i.normalizedCount(LogHistogram::RFIAmplitudeHistogram));
				plotGlobalSlopes.PushDataPoint(i.value(), intHistogram.NormalizedSlope(i.value()*0.5, i.value()*2.0, LogHistogram::RFIAmplitudeHistogram));
			}
			plotFine.StartLine("Data");
			plotGlobalSlopes.StartLine("Data");
			for(LogHistogram::iterator i=intHistogram.begin(); i!=intHistogram.end(); ++i)
			{
				plotFine.PushDataPoint(i.value(), i.normalizedCount(LogHistogram::DataAmplitudeHistogram));
				plotGlobalSlopes.PushDataPoint(i.value(), intHistogram.NormalizedSlope(i.value()*0.5, i.value()*2.0, LogHistogram::DataAmplitudeHistogram));
			}
			break;
	}
	
	std::cout << "Done.\n";
}

void printStatistics(std::complex<long double> *complexStat, unsigned count)
{
	if(count != 1)
		std::cout << '[';
	if(count > 0)
		std::cout << complexStat[0].real() << " + " << complexStat[0].imag() << 'i';
	for(unsigned p=1;p<count;++p)
	{
		std::cout << ", " << complexStat[p].real() << " + " << complexStat[p].imag() << 'i';
	}
	if(count != 1)
		std::cout << ']';
}

void printStatistics(unsigned long *stat, unsigned count)
{
	if(count != 1)
		std::cout << '[';
	if(count > 0)
		std::cout << stat[0];
	for(unsigned p=1;p<count;++p)
	{
		std::cout << ", " << stat[p];
	}
	if(count != 1)
		std::cout << ']';
}

void printStatistics(const DefaultStatistics &statistics)
{
	std::cout << "Count=";
	printStatistics(statistics.count, statistics.PolarizationCount());
	std::cout << "\nSum=";
	printStatistics(statistics.sum, statistics.PolarizationCount());
	std::cout << "\nSumP2=";
	printStatistics(statistics.sumP2, statistics.PolarizationCount());
	std::cout << "\nDCount=";
	printStatistics(statistics.dCount, statistics.PolarizationCount());
	std::cout << "\nDSum=";
	printStatistics(statistics.dSum, statistics.PolarizationCount());
	std::cout << "\nDSumP2=";
	printStatistics(statistics.dSumP2, statistics.PolarizationCount());
	std::cout << "\nRFICount=";
	printStatistics(statistics.rfiCount, statistics.PolarizationCount());
	std::cout << '\n';
}

void actionQueryBaselines(const std::string &kindName, const std::string &filename)
{
	MeasurementSet *ms = new MeasurementSet(filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	delete ms;
	
	const QualityTablesFormatter::StatisticKind kind = QualityTablesFormatter::NameToKind(kindName);
	
	QualityTablesFormatter formatter(filename);
	StatisticsCollection collection(polarizationCount);
	collection.Load(formatter);
	const std::vector<std::pair<unsigned, unsigned> > &baselines = collection.BaselineStatistics().BaselineList();
	StatisticsDerivator derivator(collection);

	std::cout << "ANTENNA1\tANTENNA2";
	for(unsigned p=0;p<polarizationCount;++p)
		std::cout << '\t' << kindName << "_POL" << p << "_R\t" << kindName << "_POL" << p << "_I" ;
	std::cout << '\n';
	for(std::vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
	{
		const unsigned antenna1 = i->first, antenna2 = i->second;
		std::cout << antenna1 << '\t' << antenna2;
		for(unsigned p=0;p<polarizationCount;++p)
		{
			const std::complex<long double> val = derivator.GetComplexBaselineStatistic(kind, antenna1, antenna2, p);
			std::cout << '\t' << val.real() << '\t' << val.imag();
		}
		std::cout << '\n';
	}
}

void actionQueryTime(const std::string &kindName, const std::string &filename)
{
	MeasurementSet *ms = new MeasurementSet(filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	delete ms;
	
	const QualityTablesFormatter::StatisticKind kind = QualityTablesFormatter::NameToKind(kindName);
	
	QualityTablesFormatter formatter(filename);
	StatisticsCollection collection(polarizationCount);
	collection.Load(formatter);
	const std::map<double, DefaultStatistics> &timeStats = collection.TimeStatistics();
	StatisticsDerivator derivator(collection);

	std::cout << "TIME";
	for(unsigned p=0;p<polarizationCount;++p)
		std::cout << '\t' << kindName << "_POL" << p << "_R\t" << kindName << "_POL" << p << "_I" ;
	std::cout << '\n';
	for(std::map<double, DefaultStatistics>::const_iterator i=timeStats.begin();i!=timeStats.end();++i)
	{
		const double time = i->first;
		std::cout << time;
		for(unsigned p=0;p<polarizationCount;++p)
		{
			const std::complex<long double> val = derivator.GetComplexStatistic(kind, i->second, p);
			std::cout << '\t' << val.real() << '\t' << val.imag();
		}
		std::cout << '\n';
	}
}

void actionSummarize(const std::string &filename)
{
	bool remote = aoRemote::ClusteredObservation::IsClusteredFilename(filename);
	StatisticsCollection collection;
	if(remote)
	{
		aoRemote::ClusteredObservation *observation = aoRemote::ClusteredObservation::Load(filename);
		aoRemote::ProcessCommander commander(*observation);
		commander.PushReadQualityTablesTask(&collection);
		commander.Run();
		delete observation;
	}
	else {
		MeasurementSet *ms = new MeasurementSet(filename);
		const unsigned polarizationCount = ms->GetPolarizationCount();
		delete ms;
		
		collection.SetPolarizationCount(polarizationCount);
		QualityTablesFormatter qualityData(filename);
		collection.Load(qualityData);
	}
	
	DefaultStatistics statistics(collection.PolarizationCount());
	
	collection.GetGlobalTimeStatistics(statistics);
	std::cout << "Time statistics: \n";
	printStatistics(statistics);
	
	collection.GetGlobalFrequencyStatistics(statistics);
	std::cout << "\nFrequency statistics: \n";
	printStatistics(statistics);

	collection.GetGlobalCrossBaselineStatistics(statistics);
	std::cout << "\nCross-correlated baseline statistics: \n";
	printStatistics(statistics);

	collection.GetGlobalAutoBaselineStatistics(statistics);
	std::cout << "\nAuto-correlated baseline: \n";
	printStatistics(statistics);
}

void actionCombine(const std::string outFilename, const std::vector<std::string> inFilenames)
{
	if(!inFilenames.empty())
	{
		const std::string &firstInFilename = *inFilenames.begin();
		bool remote = aoRemote::ClusteredObservation::IsClusteredFilename(firstInFilename);
		
		if(remote && inFilenames.size() != 1)
			throw std::runtime_error("Can only open one remote observation file at a time");
		
		if(!casa::Table::isReadable(outFilename))
		{
			if(remote)
			{
				throw std::runtime_error("Can't yet create a new set with clustered observations -- make output filename yourself");
			}
			casa::Table templateSet(firstInFilename);
			casa::Table templateAntennaTable = templateSet.keywordSet().asTable("ANTENNA");
			
			casa::SetupNewTable mainTableSetup(outFilename, templateSet.tableDesc(), casa::Table::New);
			casa::Table mainOutputTable(mainTableSetup);
			
			casa::SetupNewTable antennaTableSetup(outFilename + "/ANTENNA", templateAntennaTable.tableDesc(), casa::Table::New);
			casa::Table antennaOutputTable(antennaTableSetup);
			mainOutputTable.rwKeywordSet().defineTable("ANTENNA", antennaOutputTable);
			
			casa::TableCopy::copyRows(antennaOutputTable, templateAntennaTable);
		}
		
		if(remote)
		{
			aoRemote::ClusteredObservation *observation = aoRemote::ClusteredObservation::Load(firstInFilename);
			aoRemote::ProcessCommander commander(*observation);
			commander.Run();
			QualityTablesFormatter formatter(outFilename);
			commander.Statistics().Save(formatter);
			delete observation;
		}
	}
}

void actionRemove(const std::string &filename)
{
	QualityTablesFormatter formatter(filename);
	formatter.RemoveAllQualityTables();
}

void printSyntax(std::ostream &stream, char *argv[])
{
	stream << "Syntax: " << argv[0] <<
		" <action> [options]\n\n"
		"Possible actions:\n"
		"\thelp        - Get more info about an action (usage: '" << argv[0] << " help <action>')\n"
		"\tcollect     - Processes the entire measurement set, collects the statistics\n"
		"\t              and writes them in the quality tables.\n"
		"\\tcombine     - Combine several tables.\n"
		"\tquery_b     - Query baselines.\n"
		"\tquery_t     - Query time.\n"
		"\tremove      - Remove all quality tables.\n"
		"\tsummarize   - Give a summary of the statistics currently in the quality tables.\n";
}

int main(int argc, char *argv[])
{
#ifdef HAS_LOFARSTMAN
	register_lofarstman();
#endif // HAS_LOFARSTMAN

	if(argc < 2)
	{
		printSyntax(std::cerr, argv);
		return -1;
	} else {
		
		const std::string action = argv[1];
		
		if(action == "help")
		{
			if(argc != 3)
			{
				printSyntax(std::cout, argv);
			} else {
				std::string helpAction = argv[2];
				if(helpAction == "help")
				{
					printSyntax(std::cout, argv);
				}
				else if(helpAction == "collect")
				{
					std::cout << "Syntax: " << argv[0] << " collect [-a] <ms>\n\n"
						"The collect action will go over a whole measurement set and \n"
						"collect the default statistics. It will write the results in the \n"
						"quality subtables of the main measurement set.\n\n"
						"Currently, the default statistics are:\n"
						"\tRFIRatio, Count, Mean, SumP2, DCount, DMean, DSumP2.\n"
						"The subtables that will be updated are:\n"
						"\tQUALITY_KIND_NAME, QUALITY_TIME_STATISTIC,\n"
						"\tQUALITY_FREQUENCY_STATISTIC and QUALITY_BASELINE_STATISTIC.\n\n"
						"-c will use the CORRECTED_DATA column.\n";
				}
				else if(helpAction == "summarize")
				{
					std::cout << "Syntax: " << argv[0] << " summarize <ms>\n\n"
						"Gives a summary of the statistics in the measurement set.\n";
				}
				else if(helpAction == "query_b")
				{
					std::cout << "Syntax: " << argv[0] << " query_b <kind> <ms>\n\n"
						"Prints the given statistic for each baseline.\n";
				}
				else if(helpAction == "query_t")
				{
					std::cout << "Syntax: " << argv[0] << " query_t <kind> <ms>\n\n"
						"Print the given statistic for each time step.\n";
				}
				else if(helpAction == "combine")
				{
					std::cout << "Syntax: " << argv[0] << " combine <target_ms> [<in_ms> [<in_ms> ..]]\n\n"
						"This will read all given input measurement sets, combine the statistics and \n"
						"write the results to a target measurement set. The target measurement set should\n"
						"not exist beforehand.\n";
				}
				else if(helpAction == "remove")
				{
					std::cout << "Syntax: " << argv[0] << " remove [ms]\n\n"
						"This will completely remove all quality tables from the measurement set.\n";
				}
				else
				{
					std::cerr << "Unknown action specified in help.\n";
					return -1;
				}
			}
		}
		else if(action == "collect")
		{
			if(argc != 3 && !(argc == 4 && std::string(argv[2]) == "-a") )
			{
				std::cerr << "collect actions needs one or two parameters (the measurement set)\n";
				return -1;
			}
			else {
				std::string filename = (argc==3) ? argv[2] : argv[3];
				actionCollect(filename, argc==4 ? CollectHistograms : CollectDefault);
			}
		}
		else if(action == "combine")
		{
			if(argc < 3 )
			{
				std::cerr << "combine actions needs at least one parameter.\n";
				return -1;
			}
			else {
				std::string outFilename = argv[2];
				std::vector<std::string> inFilenames;
				for(int i=3;i<argc;++i)
					inFilenames.push_back(argv[i]);
				actionCombine(outFilename, inFilenames);
			}
		}
		else if(action == "summarize")
		{
			if(argc != 3)
			{
				std::cerr << "summarize actions needs one parameter (the measurement set)\n";
				return -1;
			}
			else {
				actionSummarize(argv[2]);
			}
		}
		else if(action == "query_b")
		{
			if(argc != 4)
			{
				std::cerr << "Syntax for query baselines: 'aoquality query_b <KIND> <MS>'\n";
				return -1;
			}
			else {
				actionQueryBaselines(argv[2], argv[3]);
			}
		}
		else if(action == "query_t")
		{
			if(argc != 4)
			{
				std::cerr << "Syntax for query times: 'aoquality query_t <KIND> <MS>'\n";
				return -1;
			}
			else {
				actionQueryTime(argv[2], argv[3]);
				return 0;
			}
		}
		else if(action == "remove")
		{
			if(argc != 3)
			{
				std::cerr << "Syntax for removing quality tables: 'aoquality remove <MS>'\n";
				return -1;
			}
			else {
				actionRemove(argv[2]);
				return 0;
			}
		}
		else
		{
			std::cerr << "Unknown action '" << action << "'.\n\n";
			printSyntax(std::cerr, argv);
			return -1;
		}
		
		return 0;
	}
}
