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
#ifndef RFISTRATEGYFOREACHBASELINEACTION_H
#define RFISTRATEGYFOREACHBASELINEACTION_H

#include "actionblock.h"
#include "artifactset.h"

#include <boost/thread/mutex.hpp>

namespace rfiStrategy {

	/**
		@author A.R. Offringa <offringa@astro.rug.nl>
	*/
	class ForEachBaselineAction : public ActionBlock {
		public:
			enum BaselineSelection {
				All, CrossCorrelations, AutoCorrelations, EqualToCurrent, AutoCorrelationsOfCurrentAntennae, Current
			};

			ForEachBaselineAction() : _threadCount(3), _selection(All), _resultSet(0), _exceptionOccured(false)
			{
			}
			virtual ~ForEachBaselineAction()
			{
			}
			virtual std::string Description()
			{
				return "For each baseline";
			}
			virtual void Initialize()
			{
			}
			virtual void Perform(ArtifactSet &artifacts, ProgressListener &progress);

			enum BaselineSelection Selection() const throw() { return _selection; }
			void SetSelection(enum BaselineSelection selection) throw() { _selection = selection; }
			size_t ThreadCount() const throw() { return _threadCount; }
			void SetThreadCount(size_t threadCount) throw() { _threadCount = threadCount; }
			virtual ActionType Type() const { return ForEachBaselineActionType; }
		private:
			bool IsBaselineSelected(ImageSetIndex &index);
			class ImageSetIndex *GetNextIndex(size_t &index);
			void SetExceptionOccured();
			void SetProgress(ProgressListener &progress, int no, int count, std::string taskName, int threadId);
			struct PerformFunction : public ProgressListener
			{
				PerformFunction(ForEachBaselineAction &action, ProgressListener &progress, size_t threadIndex)
					: _action(action), _progress(progress), _threadIndex(threadIndex)
				{
				}
				ForEachBaselineAction &_action;
				ProgressListener &_progress;
				size_t _threadIndex;
				void operator()();
				void IOLock();
				virtual void OnStartTask(size_t taskNo, size_t taskCount, const std::string &description);
				virtual void OnEndTask();
				virtual void OnProgress(size_t progres, size_t maxProgress);
				virtual void OnException(std::exception &thrownException);
			};

			size_t _baselineCount, _nextIndex;
			size_t _threadCount;
			BaselineSelection _selection;
			AntennaInfo _initAntenna1, _initAntenna2;
			size_t _initPartIndex;
			ImageSetIndex *_loopIndex;
			ArtifactSet *_artifacts, *_resultSet;
			boost::mutex _mutex;
			int *_progressTaskNo, *_progressTaskCount;
			bool _exceptionOccured;
	};
}

#endif
