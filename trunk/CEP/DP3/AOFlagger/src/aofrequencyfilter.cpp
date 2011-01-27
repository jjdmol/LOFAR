#include <iostream>
#include <string>
#include <deque>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSTable.h>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <tables/Tables/TableIter.h>

#include <AOFlagger/rfi/thresholdtools.h>
#include <AOFlagger/imaging/uvimager.h>
#include <AOFlagger/msio/system.h>

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

// This represents data that is constant for all threads
struct SetInfo
{
	unsigned bandCount;
	unsigned frequencyCount;
	unsigned polarizationCount;
};

// This represents the data that is specific for a single thread
struct TaskInfo
{
	TaskInfo() : length(0), convolutionSize(0.0), table(0), dataColumn(0), realData(0), imagData(0) { }
	TaskInfo(const TaskInfo &source) :
	 length(source.length),
	 convolutionSize(source.convolutionSize),
	 table(source.table),
	 dataColumn(source.dataColumn),
	 realData(source.realData),
	 imagData(source.imagData)
	{
	}
	void operator=(const TaskInfo &source)
	{
		length = source.length;
		convolutionSize = source.convolutionSize;
		table = source.table;
		dataColumn = source.dataColumn;
		realData = source.realData;
		imagData = source.imagData;
	}
	
	unsigned length;
	double convolutionSize;
	casa::Table *table;
	casa::ArrayColumn<casa::Complex> *dataColumn;
	float
		**realData,
		**imagData;
};

void performAndWriteConvolution(const SetInfo &set, TaskInfo task, boost::mutex &mutex)
{
	if(task.convolutionSize > 1.0)
	{
		// Convolve the data
		for(unsigned p=0;p<set.polarizationCount;++p)
		{
			ThresholdTools::OneDimensionalSincConvolution(task.realData[p], task.length, task.convolutionSize);
			ThresholdTools::OneDimensionalSincConvolution(task.imagData[p], task.length, task.convolutionSize);
		}

		boost::mutex::scoped_lock lock(mutex);
		// Copy data back to tables
		for(unsigned i=0;i<set.bandCount;++i)
		{
			casa::Array<casa::Complex> dataArray = (*task.dataColumn)(i);
			casa::Array<casa::Complex>::iterator dataIterator = dataArray.begin();
			unsigned index = i * set.frequencyCount;
			for(unsigned f=0;f<set.frequencyCount;++f)
			{
				for(unsigned p=0;p<set.polarizationCount;++p)
				{
					*dataIterator = casa::Complex(task.realData[p][index], task.imagData[p][index]);
					++dataIterator;
				}
				++index;
			}
			task.dataColumn->basePut(i, dataArray);
		}
		lock.unlock();
	}

	// Free memory
	boost::mutex::scoped_lock lock(mutex);
	for(unsigned p=0;p<set.polarizationCount;++p)
	{
		delete[] task.realData[p];
		delete[] task.imagData[p];
	}
	delete[] task.realData;
	delete[] task.imagData;
	
	delete task.dataColumn;
	delete task.table;
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
		ThreadControl(unsigned threadCount, const struct SetInfo &setInfo)
			: _setInfo(setInfo), _threadCount(threadCount), _isFinishing(false)
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
			if(_isFinishing)
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
		const struct SetInfo &SetInfo() const { return _setInfo; }
		boost::mutex &WriteMutex() { return _writeMutex; }
	private:
		const struct SetInfo _setInfo;
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
		performAndWriteConvolution(threadControl->SetInfo(), task, threadControl->WriteMutex());
		hasTask = threadControl->WaitForTask(task);
	}
	cout << "Thread " << number << " finished\n";
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		cerr << "Syntax: " << argv[0] << " <fringe size> <MS>\n";
		cerr << " fringe size is a double and should be given in units of wavelength / fringe.\n";
	} else {
		const double fringeSize = atof(argv[1]);
		const string msFilename = argv[2];
		cout << "Fringe size: " << fringeSize << '\n';

		casa::MeasurementSet ms(msFilename);
		casa::Table mainTable(msFilename, casa::Table::Update);
		class SetInfo setInfo;

		//count number of polarizations
		casa::Table polTable = ms.polarization();
		casa::ROArrayColumn<int> corTypeColumn(polTable, "CORR_TYPE"); 
		setInfo.polarizationCount = corTypeColumn(0).shape()[0];
		cout << "Number of polarizations: " << setInfo.polarizationCount << '\n';

		// Find lowest and highest frequency and check order
		double lowestFrequency = 0.0, highestFrequency = 0.0;
		casa::Table spectralWindowTable = ms.spectralWindow();
		casa::ROArrayColumn<double> frequencyCol(spectralWindowTable, "CHAN_FREQ");
		for(unsigned b=0;b<spectralWindowTable.nrow();++b)
		{
			casa::Array<double> frequencyArray = frequencyCol(b);
			casa::Array<double>::const_iterator frequencyIterator = frequencyArray.begin();
			while(frequencyIterator != frequencyArray.end())
			{
				double frequency = *frequencyIterator;
				if(lowestFrequency == 0.0) lowestFrequency = frequency;
				if(frequency < lowestFrequency || frequency <= highestFrequency)
				{
					cerr << "ERROR: Channels are not ordered in increasing frequency!\n";
					abort();
				}
				highestFrequency = frequency;
				++frequencyIterator;
			}
		}
		setInfo.bandCount = spectralWindowTable.nrow();
		cout
			<< "Number of bands: " << setInfo.bandCount
			<< " (" << round(lowestFrequency/1e6) << " MHz - "
			<< round(highestFrequency/1e6) << " MHz)\n";

		setInfo.frequencyCount =
			casa::ROArrayColumn<casa::Complex>(mainTable, "DATA")(0).shape()[1];
		cout << "Channels per band: " << setInfo.frequencyCount << '\n';

		const unsigned long totalIterations =
			mainTable.nrow() / spectralWindowTable.nrow();
		cout << "Total iterations: " << totalIterations << '\n';
		
		unsigned processorCount = System::ProcessorCount();
		cout << "CPUs: " << processorCount << '\n';
		ThreadControl threads(processorCount, setInfo);

		// Create the sorted table and iterate over it
		casa::Block<casa::String> names(4);
		names[0] = "TIME";
		names[1] = "ANTENNA1";
		names[2] = "ANTENNA2";
		names[3] = "DATA_DESC_ID";
		cout << "Sorting...\n";
		casa::Table sortab = mainTable.sort(names);
		cout << "Iterating...\n";
		unsigned long iterSteps = 0;
		names.resize(3, true, true);
		casa::TableIterator iter (sortab, names, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
		double maxFringeChannels = 0.0, minFringeChannels = 1e100;
		while (! iter.pastEnd()) {
			TaskInfo task;
			task.table = new casa::Table(iter.table());
			
			int antenna1, antenna2;
			// we start a new block to let the columns go out of scope (table might be freed
			// before end of parent block)
			{
				casa::ROScalarColumn<int> antenna1Column =
						casa::ROScalarColumn<int>(*task.table, "ANTENNA1");
				casa::ROScalarColumn<int> antenna2Column =
						casa::ROScalarColumn<int>(*task.table, "ANTENNA2");
				antenna1 = antenna1Column(0);
				antenna2 = antenna2Column(0);
			}
			
			// Skip autocorrelations
			if(antenna1 == antenna2)
			{
				delete task.table;
			} else
			{
				task.dataColumn = new casa::ArrayColumn<casa::Complex>(*task.table, "DATA");

				// Check number of channels & bands
				const casa::IPosition &dataShape = task.dataColumn->shape(0);
				if(dataShape[1] != setInfo.frequencyCount) {
					std::cerr << "ERROR: bands do not have equal number of channels!\n";
					abort();
				}
				if(task.table->nrow() != setInfo.bandCount) {
					std::cerr << "ERROR: inconsistent band information in specific correlation/time step\n"
						" (rows in table iterator's table: " << task.table->nrow() <<
						", in set: " << setInfo.bandCount << ")\n";
					abort();
				}
	
				// Retrieve uv info and calculate the convolution size
				double u, v;
				{
					casa::ROArrayColumn<double> uvwColumn =
						casa::ROArrayColumn<double>(*task.table, "UVW");
					casa::Array<double> uvwArray = uvwColumn(0);
					casa::Array<double>::const_iterator uvwIterator = uvwArray.begin();
					u = *uvwIterator;
					++uvwIterator;
					v = *uvwIterator;
				}
				task.length = setInfo.bandCount * setInfo.frequencyCount;
				task.convolutionSize = fringeSize * (double) task.length / uvDist(u, v, lowestFrequency, highestFrequency);
				if(task.convolutionSize > maxFringeChannels) maxFringeChannels = task.convolutionSize;
				if(task.convolutionSize < minFringeChannels) minFringeChannels = task.convolutionSize;

				// Allocate memory for putting the data of all channels in an array, for each polarization
				task.realData = new float*[setInfo.polarizationCount];
				task.imagData = new float*[setInfo.polarizationCount];
				for(unsigned p=0;p<setInfo.polarizationCount;++p)
				{
					task.realData[p] = new float[task.length];
					task.imagData[p] = new float[task.length];
				}
				
				boost::mutex::scoped_lock lock(threads.WriteMutex());
				// Copy data from tables in arrays
				for(unsigned i=0;i<setInfo.bandCount;++i)
				{
					casa::Array<casa::Complex> dataArray = (*task.dataColumn)(i);
					casa::Array<casa::Complex>::const_iterator dataIterator = dataArray.begin();
					unsigned index = i * setInfo.frequencyCount;
					for(unsigned f=0;f<setInfo.frequencyCount;++f)
					{
						for(unsigned p=0;p<setInfo.polarizationCount;++p)
						{
							task.realData[p][index] = (*dataIterator).real();
							task.imagData[p][index] = (*dataIterator).imag();
							++dataIterator;
						}
						++index;
					}
				}
				lock.unlock();

				threads.PushTask(task);
			}

			iter.next();
			++iterSteps;
			if((iterSteps * 100UL) % totalIterations < 100UL)
				cout << '.' << flush;
			if((iterSteps * 10UL) % totalIterations < 10UL)
				cout << (iterSteps*100/totalIterations) << '%' << flush;
		}
		cout << '\n';
		threads.Finish();
		cout
			<< "Done. " << iterSteps << " steps taken.\n"
			<< "Maximum filtering fringe size = " << maxFringeChannels << " channels, "
			   "minimum = " << minFringeChannels << " channels. \n";
	}
}
