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

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/foreachmsaction.h>
#include <AOFlagger/rfi/strategy/strategy.h>
#include <AOFlagger/rfi/strategy/xmlreader.h>

#include <AOFlagger/rfi/antennaflagcountplot.h>

#include <AOFlagger/util/progresslistener.h>
#include <AOFlagger/util/stopwatch.h>

#include <LofarStMan/Register.h>

class ConsoleProgressHandler : public ProgressListener {
	private:
		boost::mutex _mutex;
		
	public:
		
		virtual void OnStartTask(size_t taskNo, size_t taskCount, const std::string &description)
		{
			boost::mutex::scoped_lock lock(_mutex);
			ProgressListener::OnStartTask(taskNo, taskCount, description);
			
			double totalProgress = TotalProgress();
			
			std::cout << round(totalProgress*1000.0)/10.0 << "% : ";
			
			for(size_t i=1;i<Depth();++i)
				std::cout << "+-";
			
			std::cout << description << "... " << std::endl;
		}
		
		virtual void OnEndTask()
		{
			boost::mutex::scoped_lock lock(_mutex);
			
			double totalProgress = TotalProgress();
			ProgressListener::OnEndTask();
			std::cout << round(totalProgress*1000.0)/10.0 << "% : ";
						
			for(size_t i=1;i<Depth();++i)
				std::cout << "+-";
			std::cout << "DONE" << std::endl;
		}

		virtual void OnProgress(size_t, size_t)
		{
		}

		virtual void OnException(std::exception &thrownException) 
		{
			std::cerr << "ERROR: " << thrownException.what() << std::endl;
		}
};

int main(int argc, char **argv)
{
	std::cout << 
			"RFI strategy console runner\n"
			"This program will execute an RFI strategy as can be created with the RFI gui\n"
			"or a console program called rfistrategy, and executes it on one or several .MS\n"
			"directories.\n\n"
			"Author: André Offringa (offringa@astro.rug.nl)\n"
			<< std::endl;

	if(argc == 1)
	{
		std::cerr << "Usage: " << argv[0] << " [-j <threadcount>] [-strategy <file.rfis>] <ms1> [<ms2> [..]]" << std::endl;
	}
	else
	{
		register_lofarstman();

		size_t threadCount = 3;
		std::string strategyFile;
		size_t parameterIndex = 1;
		while(parameterIndex < (size_t) argc && argv[parameterIndex][0]=='-')
		{
			std::string flag(argv[parameterIndex]+1);
			if(flag=="j" && parameterIndex < (size_t) (argc-1))
			{
				threadCount = atoi(argv[parameterIndex+1]);
				parameterIndex+=2;
			}
			else if(flag=="strategy")
			{
				strategyFile = argv[parameterIndex+1];
				parameterIndex+=2;
			}
			else
			{
				std::cerr << "Incorrect usage; parameter \"" << argv[parameterIndex] << "\" not understood." << std::endl;
				return 1;
			}
		}
		std::cout << "Number of threads: " << threadCount << std::endl;

		Stopwatch watch(true);

		boost::mutex ioMutex;
		
		rfiStrategy::Strategy *subStrategy;
		if(strategyFile == "")
		{
			subStrategy = new rfiStrategy::Strategy();
			subStrategy->LoadDefaultStrategy();
		} else {
			rfiStrategy::XmlReader reader;
			subStrategy = reader.CreateStrategyFromFile(strategyFile);
			std::cout << "Strategy \"" << strategyFile << "\" loaded." << std::endl;
		}
		rfiStrategy::Strategy::SetThreadCount(*subStrategy, threadCount);
			
		rfiStrategy::ForEachMSAction *fomAction = new rfiStrategy::ForEachMSAction();
		for(int i=parameterIndex;i<argc;++i)
		{
			std::cout << "Adding '" << argv[i] << "'" << std::endl;
			fomAction->Filenames().push_back(argv[i]);
		}
		fomAction->Add(subStrategy);
		
		rfiStrategy::Strategy overallStrategy;
		overallStrategy.Add(fomAction);
	
		rfiStrategy::ArtifactSet artifacts(&ioMutex);
		artifacts.SetAntennaFlagCountPlot(new AntennaFlagCountPlot());
		
		ConsoleProgressHandler progress;
		
		overallStrategy.StartPerformThread(artifacts, progress);
		rfiStrategy::ArtifactSet *set = overallStrategy.JoinThread();

		set->AntennaFlagCountPlot()->Report();

		delete set;

		std::cout << "Time: " << watch.ToString() << std::endl;
	}
}
