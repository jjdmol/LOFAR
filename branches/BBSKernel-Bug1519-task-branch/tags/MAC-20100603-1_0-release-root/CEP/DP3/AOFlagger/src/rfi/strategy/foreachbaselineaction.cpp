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
#include <AOFlagger/rfi/strategy/foreachbaselineaction.h>

#include <AOFlagger/msio/antennainfo.h>

#include <iostream>
#include <sstream>

#include <boost/thread.hpp>

#include <AOFlagger/rfi/strategy/imageset.h>

namespace rfiStrategy {
	
	void ForEachBaselineAction::Perform(ArtifactSet &artifacts, ProgressListener &progress)
	{
		if(!artifacts.HasImageSet())
		{
			progress.OnStartTask(0, 1, "For each baseline (no image set)");
			progress.OnEndTask();
		} else if(_selection == Current)
		{
			ActionBlock::Perform(artifacts, progress);
		} else
		{
			ImageSet *imageSet = artifacts.ImageSet();
			ImageSetIndex *iteratorIndex = imageSet->StartIndex();

			if(artifacts.MetaData() != 0)
			{
				_initAntenna1 = artifacts.MetaData()->Antenna1();
				_initAntenna2 = artifacts.MetaData()->Antenna2();
				_hasInitAntennae = true;
			}
			_artifacts = &artifacts;
			_initPartIndex = imageSet->GetPart(*artifacts.ImageSetIndex());

			_baselineCount = 0;
			_nextIndex = 0;
			while(iteratorIndex->IsValid())
			{
				if(IsBaselineSelected(*iteratorIndex))
					++_baselineCount;
				iteratorIndex->Next();
			}
			delete iteratorIndex;
			
			_loopIndex = imageSet->StartIndex();
			_progressTaskNo = new int[_threadCount];
			_progressTaskCount = new int[_threadCount];
			progress.OnStartTask(0, 1, "Initializing");

			boost::thread_group threadGroup;
			for(unsigned i=0;i<_threadCount;++i)
			{
				PerformFunction function(*this, progress, i);
				threadGroup.create_thread(function);
			}
			
			threadGroup.join_all();
			progress.OnEndTask();

			if(_resultSet != 0)
			{
				artifacts = *_resultSet;
				delete _resultSet;
			}

			delete[] _progressTaskCount;
			delete[] _progressTaskNo;

			delete _loopIndex;

			if(_exceptionOccured)
				throw std::runtime_error("An exception occured in one of the sub tasks of the (multi-threaded) \"For-each baseline\"-action: the RFI strategy will not continue.");
		}
	}

	bool ForEachBaselineAction::IsBaselineSelected(ImageSetIndex &index)
	{
		if(_selection == All) return true;

		ImageSet *imageSet = _artifacts->ImageSet();
		size_t a1id = imageSet->GetAntenna1(index);
		size_t a2id = imageSet->GetAntenna2(index);

		switch(_selection)
		{
			case All:
				return true;
			case CrossCorrelations:
			{
				return a1id != a2id;
			}
			case AutoCorrelations:
				return a1id == a2id;
			case EqualToCurrent: {
				if(!_hasInitAntennae)
					throw BadUsageException("For each baseline over 'EqualToCurrent' with no current baseline");
				TimeFrequencyMetaDataCPtr metaData = imageSet->LoadMetaData(index);
				const AntennaInfo
					&a1 = metaData->Antenna1(),
					&a2 = metaData->Antenna2();
				Baseline b(a1, a2);
				Baseline initB(_initAntenna1, _initAntenna2);
				return (roundl(b.Distance()) == roundl(initB.Distance()) &&
					roundl(b.Angle()/5) == roundl(initB.Angle()/5));
			}
			case AutoCorrelationsOfCurrentAntennae:
				if(!_hasInitAntennae)
					throw BadUsageException("For each baseline over 'AutoCorrelationsOfCurrentAntennae' with no current baseline");
				return a1id == a2id && (_initAntenna1.id == a1id || _initAntenna2.id == a1id) && _initPartIndex == imageSet->GetPart(index);
			default:
				return false;
		}
	}

	class ImageSetIndex *ForEachBaselineAction::GetNextIndex(size_t &index)
	{
		boost::mutex::scoped_lock lock(_mutex);
		while(_loopIndex->IsValid())
		{
			if(IsBaselineSelected(*_loopIndex))
			{
				index = _nextIndex;

				++_nextIndex;
				ImageSetIndex *newIndex = _loopIndex->Copy();
				_loopIndex->Next();

				return newIndex;
			}
			_loopIndex->Next();
		}
		return 0;
	}

	void ForEachBaselineAction::SetExceptionOccured()
	{
		boost::mutex::scoped_lock lock(_mutex);
		_exceptionOccured = true;
	}

	void ForEachBaselineAction::PerformFunction::operator()()
	{
		ImageSet *privateImageSet = _action._artifacts->ImageSet()->Copy();

		size_t taskIndex;

		try {

			IOLock();
			ArtifactSet newArtifacts(*_action._artifacts);
			ImageSetIndex *index = _action.GetNextIndex(taskIndex);
			
			while(index != 0) {
				index->Reattach(*privateImageSet);
				
				TimeFrequencyMetaDataCPtr metaData = privateImageSet->LoadMetaData(*index);
		
				IOUnlock();
	
				_action.SetProgress(_progress, taskIndex, _action._baselineCount, "Processing baseline", _threadIndex);
	
				newArtifacts.SetImageSetIndex(index);
				newArtifacts.SetMetaData(metaData);

				_action.ActionBlock::Perform(newArtifacts, *this);
				delete index;
	
				IOLock();
				index = _action.GetNextIndex(taskIndex);
			}
	
			if(_threadIndex == 0)
				_action._resultSet = new ArtifactSet(newArtifacts);

			IOUnlock();

		} catch(std::exception &e)
		{
			_progress.OnException(e);
			_action.SetExceptionOccured();
		}

		delete privateImageSet;
	}

	void ForEachBaselineAction::PerformFunction::IOLock()
	{
	  if(_lock != 0)
	    delete _lock;
	  //_lock = new boost::mutex::scoped_lock(_action._artifacts->IOMutex(), boost::try_to_lock);
		// TODO
	  _lock = new boost::mutex::scoped_lock(_action._artifacts->IOMutex());
	  //	if(!_lock->owns_lock())
	  //	{
	  //		std::cout << "Thread " << _threadIndex << " is waiting for IO lock." << std::endl;
	  //		delete _lock;
	  //		_lock = new boost::mutex::scoped_lock(_action._artifacts->IOMutex());
	  //		std::cout << "Thread " << _threadIndex << " received IO lock." << std::endl;
	  //	}
	}
	void ForEachBaselineAction::PerformFunction::IOUnlock()
	{
	  _lock->unlock();
	}

	void ForEachBaselineAction::PerformFunction::OnStartTask(size_t /*taskNo*/, size_t /*taskCount*/, const std::string &/*description*/)
	{
	}

	void ForEachBaselineAction::PerformFunction::OnEndTask()
	{
	}

	void ForEachBaselineAction::PerformFunction::OnProgress(size_t /*progres*/, size_t /*maxProgress*/)
	{
	}

	void ForEachBaselineAction::PerformFunction::OnException(std::exception &thrownException)
	{
		_progress.OnException(thrownException);
	}

	void ForEachBaselineAction::SetProgress(ProgressListener &progress, int no, int count, std::string taskName, int threadId)
	{
	  boost::mutex::scoped_lock lock(_mutex);
		_progressTaskNo[threadId] = no;
		_progressTaskCount[threadId] = count;
		size_t totalCount = 0, totalNo = 0;
		for(size_t i=0;i<_threadCount;++i)
		{
			totalCount += _progressTaskCount[threadId];
			totalNo += _progressTaskNo[threadId];
		}
		progress.OnEndTask();
		std::stringstream str;
		str << "T" << threadId << ": " << taskName;
		progress.OnStartTask(totalNo, totalCount, str.str());
	}
}

