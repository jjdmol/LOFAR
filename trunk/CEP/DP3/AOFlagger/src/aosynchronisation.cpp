#include <iostream>
#include <string>

#include <boost/version.hpp>

#if (BOOST_VERSION >= 103600)
#define BOOST_COMPATIBLE
#endif

#ifdef BOOST_COMPATIBLE
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#endif

using namespace std;

#ifdef BOOST_COMPATIBLE
using namespace boost::interprocess;

const char MUTEX_NAME[] = "aosynchronisationmutex";
const char CONDITION_NAME[] = "aosynchronisationcondition";
const char SHAREDMEM_NAME[] = "aosynchronisationmem";
const size_t RESOURCECOUNT = 1024;
const size_t MEMSIZE = RESOURCECOUNT*2+1;

char readLock(char *memptr, unsigned index)
{
	return memptr[index+1];
}

char writeLock(char *memptr, unsigned index)
{
	return memptr[index+1+RESOURCECOUNT];
}

void setReadLock(char *memptr, unsigned index, char value)
{
	memptr[index+1] = value;
}

void setWriteLock(char *memptr, unsigned index, char value)
{
	memptr[index+1+RESOURCECOUNT] = value;
}

void runMaster()
{
	named_mutex mutex(boost::interprocess::create_only, MUTEX_NAME);
	named_condition condition(boost::interprocess::create_only, CONDITION_NAME);
	shared_memory_object sharedmem(create_only, SHAREDMEM_NAME, read_write);
	sharedmem.truncate(MEMSIZE);
	mapped_region region(sharedmem, read_write, 0, MEMSIZE);
	char *memptr = static_cast<char*>(region.get_address());
	char *copy = new char[MEMSIZE];
	for(size_t i=0;i<MEMSIZE;++i)
	{
		memptr[i] = 0;
		copy[i] = 0;
	}

	scoped_lock<named_mutex> lock(mutex);
	while(memptr[0] == 0)
	{
		condition.wait(lock);
		
		for(unsigned i=0;i<RESOURCECOUNT;++i)
		{
			if(readLock(copy, i) != readLock(memptr, i))
			{
				if(readLock(memptr, i) != 0)
				  std::cout << "Resource " << i << " is locked" << std::endl;
				//else
				//  std::cout << "Resource " << i << " is released" << std::endl;
				setReadLock(copy, i, readLock(memptr, i));
			}
		}
	}
	mutex.remove(MUTEX_NAME);
	condition.remove(CONDITION_NAME);
	sharedmem.remove(SHAREDMEM_NAME);
	delete[] copy;
}

void runShutdown()
{
	named_mutex mutex(boost::interprocess::open_only, MUTEX_NAME);
	named_condition condition(boost::interprocess::open_only, CONDITION_NAME);
	shared_memory_object sharedmem(open_only, SHAREDMEM_NAME, read_write);
	mapped_region region(sharedmem, read_write, 0, MEMSIZE);
	char *memptr = static_cast<char*>(region.get_address());
	scoped_lock<named_mutex> lock(mutex);
	memptr[0] = 1;
	condition.notify_all();
}

void runLockUnique(int resourceIndex)
{
	named_mutex mutex(boost::interprocess::open_only, MUTEX_NAME);
	named_condition condition(boost::interprocess::open_only, CONDITION_NAME);
	shared_memory_object sharedmem(open_only, SHAREDMEM_NAME, read_write);
	mapped_region region(sharedmem, read_write, 0, MEMSIZE);
	char *memptr = static_cast<char*>(region.get_address());
	scoped_lock<named_mutex> lock(mutex);
	while(writeLock(memptr, resourceIndex) != 0)
	{
		condition.wait(lock);
	}
	setWriteLock(memptr, resourceIndex, 1);
	while(readLock(memptr, resourceIndex) != 0)
	{
		condition.wait(lock);
	}
	condition.notify_all();
}

void runReleaseUnique(int resourceIndex)
{
	named_mutex mutex(boost::interprocess::open_only, MUTEX_NAME);
	named_condition condition(boost::interprocess::open_only, CONDITION_NAME);
	shared_memory_object sharedmem(open_only, SHAREDMEM_NAME, read_write);
	mapped_region region(sharedmem, read_write, 0, MEMSIZE);
	char *memptr = static_cast<char*>(region.get_address());
	scoped_lock<named_mutex> lock(mutex);
	setWriteLock(memptr, resourceIndex, 0);
	setReadLock(memptr, resourceIndex, 0);
	condition.notify_all();
}

void runLock(int resourceIndex)
{
	named_mutex mutex(boost::interprocess::open_only, MUTEX_NAME);
	named_condition condition(boost::interprocess::open_only, CONDITION_NAME);
	shared_memory_object sharedmem(open_only, SHAREDMEM_NAME, read_write);
	mapped_region region(sharedmem, read_write, 0, MEMSIZE);
	char *memptr = static_cast<char*>(region.get_address());
	scoped_lock<named_mutex> lock(mutex);
	while(writeLock(memptr, resourceIndex) != 0)
	{
		condition.wait(lock);
	}
	setReadLock(memptr, resourceIndex, readLock(memptr, resourceIndex)+1);
	condition.notify_all();
}

void runRelease(int resourceIndex)
{
	named_mutex mutex(boost::interprocess::open_only, MUTEX_NAME);
	named_condition condition(boost::interprocess::open_only, CONDITION_NAME);
	shared_memory_object sharedmem(open_only, SHAREDMEM_NAME, read_write);
	mapped_region region(sharedmem, read_write, 0, MEMSIZE);
	char *memptr = static_cast<char*>(region.get_address());
	scoped_lock<named_mutex> lock(mutex);
	int readers = readLock(memptr, resourceIndex);
	if(readers > 0)
		setReadLock(memptr, resourceIndex, readers-1);
	condition.notify_all();
}

void runClean()
{
	named_mutex mutex(boost::interprocess::open_or_create, MUTEX_NAME);
	named_condition condition(boost::interprocess::open_or_create, CONDITION_NAME);
	shared_memory_object sharedmem(boost::interprocess::open_or_create, SHAREDMEM_NAME, read_write);
	sharedmem.truncate(MEMSIZE);
	mutex.remove(MUTEX_NAME);
	condition.remove(CONDITION_NAME);
	sharedmem.remove(SHAREDMEM_NAME);
}

void printError(char *argv0)
{
	cerr << "Syntax: " << argv0 << " <operation> [resource number]\n"
		"Operation can be \'master\', \'shutdown\', \'lock\', \'lock-unique\', \'release\', \'release-unique\', 'clean'.\n";
}

int main(int argc, char *argv[])
{
	if(argc <= 1)
		printError(argv[0]);
	else
	{
		std::string operation = argv[1];
		if(operation == "master") runMaster();
		else if(operation == "shutdown") runShutdown();
		else if(operation == "lock" && argc >= 3) runLock(atoi(argv[2]));
		else if(operation == "lock-unique" && argc >= 3) runLockUnique(atoi(argv[2]));
		else if(operation == "release" && argc >= 3) runRelease(atoi(argv[2]));
		else if(operation == "release-unique" && argc >= 3) runReleaseUnique(atoi(argv[2]));
		else if(operation == "clean") runClean();
		else printError(argv[0]);
	}
}
#else
int main(int argc, char *argv[])
{
  return 0;
}
#endif
