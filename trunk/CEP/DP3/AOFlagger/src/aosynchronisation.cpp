#include <iostream>
#include <string>

#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/interprocess/shared_memory_object.hpp>

using namespace std;

using namespace boost::interprocess;

const char MUTEX_NAME[] = "aosynchronisationmutex";
const char CONDITION_NAME[] = "aosynchronisationcondition";
const char SHAREDMEM_NAME[] = "aosynchronisationmem";
const size_t MEMSIZE = 1024;

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
		
		for(unsigned i=1;i<MEMSIZE;++i)
		{
			if(copy[i] != memptr[i])
			{
				if(memptr[i] != 0)
					std::cout << "Resource " << (i-1) << " is locked";
				else
					std::cout << "Resource " << (i-1) << " is released";
				copy[i] = memptr[i];
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
	while(memptr[resourceIndex+1] != 0)
	{
		condition.wait(lock);
	}
	memptr[resourceIndex+1] = 1;
}

void runReleaseUnique(int resourceIndex)
{
	named_mutex mutex(boost::interprocess::open_only, MUTEX_NAME);
	named_condition condition(boost::interprocess::open_only, CONDITION_NAME);
	shared_memory_object sharedmem(open_only, SHAREDMEM_NAME, read_write);
	mapped_region region(sharedmem, read_write, 0, MEMSIZE);
	char *memptr = static_cast<char*>(region.get_address());
	scoped_lock<named_mutex> lock(mutex);
	memptr[resourceIndex+1] = 0;
	condition.notify_all();
}

void printError(char *argv0)
{
	cerr << "Syntax: " << argv0 << " <operation> [resource number]\n"
		"Operation can be \'master\', \'shutdown\', \'lock\', \'lock-unique\', \'release\', \'release-unique\'.\n";
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
		else if(operation == "lock" && argc >= 3) runLockUnique(atoi(argv[2]));
		else if(operation == "lock-unique" && argc >= 3) runLockUnique(atoi(argv[2]));
		else if(operation == "release" && argc >= 3) runReleaseUnique(atoi(argv[2]));
		else if(operation == "release-unique" && argc >= 3) runReleaseUnique(atoi(argv[2]));
		else printError(argv[0]);
	}
}
