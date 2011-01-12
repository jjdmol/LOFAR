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

#include <iostream>

#include <libgen.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/baselineselectionaction.h>
#include <AOFlagger/rfi/strategy/foreachmsaction.h>
#include <AOFlagger/rfi/strategy/strategy.h>
#include <AOFlagger/rfi/strategy/strategyreader.h>

#include <AOFlagger/rfi/antennaflagcountplot.h>
#include <AOFlagger/rfi/frequencyflagcountplot.h>
#include <AOFlagger/rfi/timeflagcountplot.h>

#include <AOFlagger/util/aologger.h>
#include <AOFlagger/util/parameter.h>
#include <AOFlagger/util/progresslistener.h>
#include <AOFlagger/util/stopwatch.h>

#include <AOFlagger/configuration.h>

#ifdef HAS_LOFARSTMAN
#include <LofarStMan/Register.h>
#endif // HAS_LOFARSTMAN

#include <boost/date_time/posix_time/posix_time.hpp>

class ConsoleProgressHandler : public ProgressListener {
	private:
		boost::mutex _mutex;
		
	public:
		
		virtual void OnStartTask(const rfiStrategy::Action &action, size_t taskNo, size_t taskCount, const std::string &description, size_t weight)
		{
			boost::mutex::scoped_lock lock(_mutex);
			ProgressListener::OnStartTask(action, taskNo, taskCount, description, weight);
			
			double totalProgress = TotalProgress();
			
			AOLogger::Progress << round(totalProgress*1000.0)/10.0 << "% : ";
			
			for(size_t i=1;i<Depth();++i)
				AOLogger::Progress << "+-";
			
			AOLogger::Progress << description << "... \n";
		}
		
		virtual void OnEndTask(const rfiStrategy::Action &action)
		{
			boost::mutex::scoped_lock lock(_mutex);
			
			ProgressListener::OnEndTask(action);
		}

		virtual void OnProgress(const rfiStrategy::Action &action, size_t i, size_t j)
		{
			ProgressListener::OnProgress(action, i, j);
		}

		virtual void OnException(const rfiStrategy::Action &, std::exception &thrownException) 
		{
			AOLogger::Error << thrownException.what() << '\n';
		}
};

int main(int argc, char **argv)
{
	if(argc == 1)
	{
		AOLogger::Init(basename(argv[0]), true);
		AOLogger::Error << "Usage: " << argv[0] << " [-v] [-j <threadcount>] [-strategy <file.rfis>] [-indirect-read] [-nolog] [-skip-flagged] <ms1> [<ms2> [..]]\n"
		"  -v will produce verbose output\n"
		"  -j overrides the number of threads specified in the strategy\n"
		"  -strategy specifies a possible customized strategy\n"
		"  -indirect-read will reorder the measurement set before starting, which is normally faster\n"
		"  -nolog will not use the LOFAR logger to output logging messages\n"
		"  -skip-flagged will skip an ms if it has already been processed by RFI console according\n"
		"   to its HISTORY table.\n"
		"Execute 'rfistrategy' without parameters for help on creating RFIS strategies.\n";
	}
	else
	{
#ifdef HAS_LOFARSTMAN
		register_lofarstman();
#endif // HAS_LOFARSTMAN

		Parameter<size_t> threadCount;
		Parameter<bool> indirectRead;
		Parameter<std::string> strategyFile;
		Parameter<bool> useLogger;
		Parameter<bool> logVerbose;
		Parameter<bool> skipFlagged;

		size_t parameterIndex = 1;
		while(parameterIndex < (size_t) argc && argv[parameterIndex][0]=='-')
		{
			std::string flag(argv[parameterIndex]+1);
			if(flag=="j" && parameterIndex < (size_t) (argc-1))
			{
				threadCount = atoi(argv[parameterIndex+1]);
				parameterIndex+=2;
			}
			else if(flag=="v")
			{
 				logVerbose = true;
				++parameterIndex;
			}
			else if(flag=="indirect-read")
			{
				indirectRead = true;
				++parameterIndex;
			}
			else if(flag=="strategy")
			{
				strategyFile = argv[parameterIndex+1];
				parameterIndex+=2;
			}
			else if(flag=="nolog")
			{
				useLogger = false;
				++parameterIndex;
			}
			else if(flag=="skip-flagged")
			{
				skipFlagged = true;
				++parameterIndex;
			}
			else
			{
				AOLogger::Init(basename(argv[0]), useLogger.Value(true));
				AOLogger::Error << "Incorrect usage; parameter \"" << argv[parameterIndex] << "\" not understood.\n";
				return 1;
			}
		}

		AOLogger::Init(basename(argv[0]), useLogger.Value(true), logVerbose.Value(false));
		AOLogger::Info << 
			"RFI strategy console runner\n"
			"This program will execute an RFI strategy as can be created with the RFI gui\n"
			"or a console program called rfistrategy, and executes it on one or several .MS\n"
			"directories.\n\n"
			"Author: André Offringa (offringa@astro.rug.nl)\n\n";

		if(threadCount.IsSet())
			AOLogger::Debug << "Number of threads: " << threadCount.Value() << "\n";

		Stopwatch watch(true);

		boost::mutex ioMutex;
		
		rfiStrategy::Strategy *subStrategy;
		if(!strategyFile.IsSet())
		{
			subStrategy = new rfiStrategy::Strategy();
			subStrategy->LoadDefaultStrategy();
		} else {
			rfiStrategy::StrategyReader reader;
			try {
				AOLogger::Debug << "Opening strategy file '" << strategyFile.Value() << "'\n";
				subStrategy = reader.CreateStrategyFromFile(strategyFile);
				AOLogger::Debug << "Strategy parsed succesfully.\n";
			} catch(std::exception &e)
			{
				AOLogger::Error <<
					"ERROR: Reading strategy file \"" << strategyFile.Value() << "\" failed! This\n"
					"might be caused by a change in the file format of the strategy file after you\n"
					"created the strategy file, as it is still rapidly changing.\n"
					"Try recreating the file with rfistrategy.\n"
					"\nThe thrown exception was:\n" << e.what() << "\n";
				exit(1);
			}
		}
		if(threadCount.IsSet())
			rfiStrategy::Strategy::SetThreadCount(*subStrategy, threadCount);
			
		rfiStrategy::ForEachMSAction *fomAction = new rfiStrategy::ForEachMSAction();
		if(indirectRead.IsSet())
			fomAction->SetIndirectReader(indirectRead);
		std::stringstream commandLineStr;
		commandLineStr << argv[0];
		for(int i=1;i<argc;++i)
		{
			commandLineStr << " \"" << argv[i] << '\"';
		}
		fomAction->SetCommandLineForHistory(commandLineStr.str());
		if(skipFlagged.IsSet())
			fomAction->SetSkipIfAlreadyProcessed(skipFlagged);
		for(int i=parameterIndex;i<argc;++i)
		{
			AOLogger::Debug << "Adding '" << argv[i] << "'\n";
			fomAction->Filenames().push_back(argv[i]);
		}
		fomAction->Add(subStrategy);
		
		rfiStrategy::Strategy overallStrategy;
		overallStrategy.Add(fomAction);
	
		rfiStrategy::ArtifactSet artifacts(&ioMutex);
		artifacts.SetAntennaFlagCountPlot(new AntennaFlagCountPlot());
		artifacts.SetFrequencyFlagCountPlot(new FrequencyFlagCountPlot());
		artifacts.SetTimeFlagCountPlot(new TimeFlagCountPlot());
		artifacts.SetBaselineSelectionInfo(new rfiStrategy::BaselineSelectionInfo());
		
		ConsoleProgressHandler progress;

		AOLogger::Info << "Starting strategy on " << to_simple_string(boost::posix_time::microsec_clock::local_time()) << '\n';
		
		overallStrategy.InitializeAll();
		overallStrategy.StartPerformThread(artifacts, progress);
		rfiStrategy::ArtifactSet *set = overallStrategy.JoinThread();
		overallStrategy.FinishAll();

		set->AntennaFlagCountPlot()->Report();
		set->FrequencyFlagCountPlot()->Report();

		delete set->AntennaFlagCountPlot();
		delete set->FrequencyFlagCountPlot();
		delete set->TimeFlagCountPlot();
		delete set->BaselineSelectionInfo();

		delete set;

		AOLogger::Debug << "Time: " << watch.ToString() << "\n";
	}
}
