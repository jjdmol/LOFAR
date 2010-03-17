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
#ifndef PROGRESSLISTENER_H
#define PROGRESSLISTENER_H

#include <string>
#include <vector>

class ProgressListener
{
	private:
		std::vector<size_t> _totals, _progresses;
		double _taskProgress;
	protected:
		double TotalProgress()
		{
			double part = 1.0, total = 0.0;
			for(size_t i=0;i<_totals.size();++i)
			{
				total += part * ((double) _progresses[i] / (double) _totals[i]);
				
				part /= (double) _totals[i];
			}
			total += part * _taskProgress;
			return total;
		}
		size_t Depth() const { return _totals.size(); }
	public:
		ProgressListener() { }
		virtual ~ProgressListener() { }

		virtual void OnStartTask(size_t taskNo, size_t taskCount, const std::string &/*description*/)
		{
			_totals.push_back(taskCount);
			_progresses.push_back(taskNo);
			_taskProgress = 0.0;
		}
		
		/**
		 * Signifies the end of the current task. It's not allowed to call OnProgress() after a call
		 * to OnEndTask() until a new task has been started with OnStartTask().
		 */
		virtual void OnEndTask()
		{
			_totals.pop_back();
			_progresses.pop_back();
			_taskProgress = 1.0;
		}

		virtual void OnProgress(size_t progress, size_t maxProgress)
		{
			_taskProgress = (double) progress / maxProgress;
		}

		virtual void OnException(std::exception &thrownException) = 0;
};

#endif // PROGRESSLISTENER_H
