#include <iostream>
#include <string>
#include <deque>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <AOFlagger/strategy/algorithms/convolutions.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/msio/system.h>
#include <AOFlagger/msio/timestepaccessor.h>

using namespace std;

double uvDist(double u, double v, double firstFrequency, double lastFrequency)
{
	const double
		lowU = u * firstFrequency,
		lowV = v * firstFrequency,
		highU = u * lastFrequency,
		highV = v * lastFrequency,
		ud = lowU - highU,
		vd = lowV - highV;
	return sqrt(ud * ud + vd * vd) / UVImager::SpeedOfLight();
}

// This represents the data that is specific for a single thread
struct TaskInfo
{
	TaskInfo() : length(0), convolutionSize(0.0), index(0), data() { }
	TaskInfo(const TaskInfo &source) :
	 length(source.length),
	 convolutionSize(source.convolutionSize),
	 index(source.index),
	 data(source.data)
	{
	}
	void operator=(const TaskInfo &source)
	{
		length = source.length;
		convolutionSize = source.convolutionSize;
		index = source.index;
		data = source.data;
	}
	
	unsigned length;
	double convolutionSize;
	TimestepAccessor::TimestepIndex *index;
	TimestepAccessor::TimestepData data;
};

void performAndWriteConvolution(TimestepAccessor &accessor, TaskInfo task, boost::mutex &mutex)
{
	unsigned polarizationCount = accessor.PolarizationCount();
	if(task.convolutionSize > 1.0)
	{
		// Convolve the data
		for(unsigned p=0;p<polarizationCount;++p)
		{
			Convolutions::OneDimensionalSincConvolution(task.data.realData[p], task.length, 1.0/task.convolutionSize);
			Convolutions::OneDimensionalSincConvolution(task.data.imagData[p], task.length, 1.0/task.convolutionSize);
		}

		// Copy data back to tables
		boost::mutex::scoped_lock lock(mutex);
		accessor.Write(*task.index, task.data);
		lock.unlock();
	}

	task.data.Free(polarizationCount);
	delete task.index;
}

struct ThreadFunction
{
	void operator()();
	class ThreadControl *threadControl;
	int number;
};

class ThreadControl
{
	public:
		ThreadControl(unsigned threadCount, TimestepAccessor &accessor)
			: _accessor(accessor), _threadCount(threadCount), _isFinishing(false)
		{
			for(unsigned i=0;i<threadCount;++i)
			{
				ThreadFunction function;
				function.number = i;
				function.threadControl = this;
				_threadGroup.create_thread(function);
			}
		}
		void PushTask(const TaskInfo &taskInfo)
		{
			boost::mutex::scoped_lock lock(_mutex);
			while(_tasks.size() > _threadCount * 10)
			{
				_queueFullCondition.wait(lock);
			}

			_tasks.push_back(taskInfo);
			_dataAvailableCondition.notify_one();
		}
		bool WaitForTask(TaskInfo &taskInfo)
		{
			boost::mutex::scoped_lock lock(_mutex);
			while(_tasks.empty() && !_isFinishing)
			{
				_dataAvailableCondition.wait(lock);
			}
			if(_isFinishing && _tasks.empty())
				return false;
			else
			{
				taskInfo = _tasks.front();
				_tasks.pop_front();
				_queueFullCondition.notify_one();
				return true;
			}
		}
		void Finish()
		{
			boost::mutex::scoped_lock lock(_mutex);
			_isFinishing = true;
			lock.unlock();
			_dataAvailableCondition.notify_all();
			_threadGroup.join_all();
		}
		boost::mutex &WriteMutex() { return _writeMutex; }
		TimestepAccessor &Accessor() { return _accessor; }
		unsigned QueueSize()
		{
			boost::mutex::scoped_lock lock(_mutex);
			return _tasks.size();
		}
	private:
		TimestepAccessor &_accessor;
		boost::thread_group _threadGroup;
		unsigned _threadCount;
		bool _isFinishing;
		boost::mutex _mutex;
		boost::condition _dataAvailableCondition;
		boost::condition _queueFullCondition;
		std::deque<TaskInfo> _tasks;
		boost::mutex _writeMutex;
};

void ThreadFunction::operator()()
{
	cout << "Thread " << number << " started\n";
	TaskInfo task;
	bool hasTask = threadControl->WaitForTask(task);
	while(hasTask)
	{
		performAndWriteConvolution(threadControl->Accessor(), task, threadControl->WriteMutex());
		hasTask = threadControl->WaitForTask(task);
	}
	cout << "Thread " << number << " finished\n";
}

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		cerr << "Syntax: " << argv[0] << " <fringe size> <taskIndex> <taskCount> <column-name> <locking> <MS1> [<MS2> [..]]\n";
		cerr << " fringe size is a double and should be given in units of wavelength / fringe.\n";
	} else {
		const double fringeSize = atof(argv[1]);
		cout << "Fringe size: " << fringeSize << '\n';

		const int taskIndex = atoi(argv[2]), taskCount = atoi(argv[3]);
		cout << "Task index " << taskIndex << " out of " << taskCount << '\n';
		const std::string columnName = argv[4];
		const bool performLocking(atoi(argv[5])!=0);
		if(performLocking)
			cout << "Locking WILL be performed.\n";
		else
			cout << "NO locking will be performend.\n";

		TimestepAccessor accessor(performLocking);
		for(int i=6;i<argc;++i)
			accessor.AddMS(argv[i]);

		accessor.SetColumnName(columnName);
		accessor.Open();

		unsigned long rows = accessor.TotalRowCount();
		unsigned long start = rows * taskIndex / taskCount;
		unsigned long end = rows * (taskIndex+1) / taskCount;
		cout << "Filtering rows " << start << '-' << end << ".\n";
		accessor.SetStartRow(start);
		accessor.SetEndRow(end);

		cout << "Number of polarizations: " << accessor.PolarizationCount() << '\n';
		cout
			<< "Number of channels: " << accessor.TotalChannelCount()
			<< " (" << round(accessor.LowestFrequency()/1e6) << " MHz - "
			<< round(accessor.HighestFrequency()/1e6) << " MHz)\n";

		const unsigned long totalIterations = end - start;
		cout << "Total iterations: " << totalIterations << '\n';
		
		const unsigned processorCount = System::ProcessorCount();
		cout << "CPUs: " << processorCount << '\n';
		ThreadControl threads(processorCount, accessor);

		double maxFringeChannels = 0.0, minFringeChannels = 1e100;
		const unsigned totalChannels = accessor.TotalChannelCount();

		TimestepAccessor::TimestepIndex *index = new TimestepAccessor::TimestepIndex();
		TimestepAccessor::TimestepData data;

		data.Allocate(accessor.PolarizationCount(), totalChannels);
		
		unsigned iterSteps = 0;

		boost::mutex::scoped_lock lock(threads.WriteMutex());
		while(accessor.ReadNext(*index, data)) {
			lock.unlock();

			TaskInfo task;
			task.index = index;
			task.data = data;
			
			// Skip autocorrelations
			if(data.antenna1 == data.antenna2)
			{
				data.Free(accessor.PolarizationCount());
				delete index;
			} else
			{
				// Calculate the convolution size
				double u = data.u, v = data.v;
				task.length = totalChannels;
				task.convolutionSize = fringeSize * (double) totalChannels / uvDist(u, v, accessor.LowestFrequency(), accessor.HighestFrequency());
				if(task.convolutionSize > maxFringeChannels) maxFringeChannels = task.convolutionSize;
				if(task.convolutionSize < minFringeChannels) minFringeChannels = task.convolutionSize;
				task.data = data;

				// Add task
				threads.PushTask(task);
			}

			index = new TimestepAccessor::TimestepIndex();
			data.Allocate(accessor.PolarizationCount(), totalChannels);

			lock.lock();
			++iterSteps;

			if(iterSteps%100==0)
			{
				cout << threads.QueueSize();
				cout << '.' << flush;
			}
		}
		lock.unlock();

		data.Free(accessor.PolarizationCount());
		delete index;

		cout << "\nWaiting for threads to finish..." << endl;
		threads.Finish();
		cout << "Closing time accessor..." << endl;
		accessor.Close();
		cout
			<< "Done. " << iterSteps << " steps taken.\n"
			<< "Write action count: " << accessor.WriteActionCount() << '\n'
			<< "Maximum filtering fringe size = " << maxFringeChannels << " channels, "
			   "minimum = " << minFringeChannels << " channels. \n";
	}
}
