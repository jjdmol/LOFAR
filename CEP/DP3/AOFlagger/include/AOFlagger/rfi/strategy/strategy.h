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

#ifndef RFISTRATEGY_H
#define RFISTRATEGY_H 

#include <string>
#include <vector>

#include <boost/thread.hpp>

#include <AOFlagger/msio/timefrequencyimager.h>
#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/rfi/strategy/foreachbaselineaction.h>

#include "action.h"
#include "actionblock.h"
#include "actionfactory.h"

namespace rfiStrategy {

	class Strategy : public ActionBlock
	{
		public:
			Strategy() throw() : _thread(0) { }
			virtual ~Strategy() { JoinThread(); }

			virtual std::string Description() { return "Strategy"; }

			static Strategy *CreateDefaultSingleStrategy();

			static void SetThreadCount(Strategy &strategy, size_t threadCount);
			static void SetDataKind(Strategy &strategy, enum DataKind kind);
			static void SetPolarisations(Strategy &strategy, enum PolarisationType type);
			static void SetBaselines(Strategy &strategy, enum BaselineSelection baselineSelection);
			static void SetTransientCompatibility(Strategy &strategy);
			static void SetMultiplySensitivity(Strategy &strategy, num_t factor);
			static void SetFittingWindowSize(Strategy &strategy, size_t windowWidth, size_t windowHeight);
			static void SetFittingKernelSize(Strategy &strategy, num_t kernelWidth, num_t kernelHeight);

			void StartPerformThread(const class ArtifactSet &artifacts, class ProgressListener &progress);
			ArtifactSet *JoinThread();

			static void LoadDefaultSingleStrategy(ActionBlock &block, bool pedantic = false, bool pulsar = false);
			void LoadDefaultSingleStrategy(bool pedantic = false, bool pulsar = false);
			void LoadOldDefaultSingleStrategy();
			void LoadDefaultStrategy();

			void LoadFastStrategy(bool pedantic = false, bool pulsar = false);
			void LoadAverageStrategy(bool pedantic = false, bool pulsar = false);
			void LoadBestStrategy(bool pedantic = false, bool pulsar = false);

			virtual void Perform(class ArtifactSet &artifacts, class ProgressListener &listener)
			{
				listener.OnStartTask(0, 1, "strategy");
				try {
					ActionBlock::Perform(artifacts, listener);
				} catch(std::exception &e)
				{
					listener.OnException(e);
				}
				listener.OnEndTask();
			}
			virtual ActionType Type() const { return StrategyType; }
		protected:
		private:
			struct PerformFunc {
				PerformFunc(class Strategy *strategy, class ArtifactSet *artifacts, class ProgressListener *progress)
				: _strategy(strategy), _artifacts(artifacts), _progress(progress)
				{
				}
				class Strategy *_strategy;
				class ArtifactSet *_artifacts;
				class ProgressListener *_progress;

				void operator()();
			} *_threadFunc;

			boost::thread *_thread;
	};
}

#endif // RFISTRATEGY_H
