
#ifdef SWIG
%module MyProtocol
%{
#include "MyProtocol.ph"
#include <string.h>
    %}
#endif

#include <string.h>

namespace MyProtocol
{

    class Event
	{
	public:
	    Event() : length(sizeof(Event)) { }

	    int signal;
	    int length;
	};

    class EventExt
	{
	public:
	    EventExt() : buffer(0), upperbound(0) { }

	    virtual ~EventExt() { if (buffer) delete [] buffer; }

	    virtual void* pack(unsigned int& packsize) = 0;
	    virtual void unpack() = 0;
	    virtual Event& getEvent() = 0;

	protected:
	    char* buffer;
	    unsigned int upperbound;
	};

    class ABSBeamAllocEvent : public Event
	{
	public:
	    ABSBeamAllocEvent() { length = sizeof(ABSBeamAllocEvent); }

	    int param1;
	    int param2;
	};

    class ABSBeamAllocEventExt : public EventExt
	{
	public:
	    ABSBeamAllocEventExt(ABSBeamAllocEvent& e, bool dounpack = false) : base(e)
		{
		    if (dounpack) unpack();
		}

	    virtual ~ABSBeamAllocEventExt() { }

#ifdef SWIG
	private:
#endif

	    ABSBeamAllocEvent& base;

#ifdef SWIG
	public:
#endif

	    unsigned int ext1_size;
	    int* ext1;

	    unsigned int ext2_size;
	    char* ext2;

	    virtual void* pack(unsigned int& packsize)
		{
		    unsigned int requiredsize = sizeof(ext1_size) + ext1_size*sizeof(int) +
			sizeof(ext2_size) + ext2_size*sizeof(char);

		    if (requiredsize > upperbound && buffer) delete [] buffer;
		    else
		    {
			buffer = new char[requiredsize];
			upperbound = requiredsize;
		    }

		    unsigned int offset = 0;

		    memcpy(buffer+offset, &ext1_size, sizeof(ext1_size));
		    offset += sizeof(ext1_size);
		    memcpy(buffer+offset, ext1, ext1_size * sizeof(int));
		    offset += ext1_size * sizeof(int);

		    memcpy(buffer+offset, &ext2_size, sizeof(ext2_size));
		    offset += sizeof(ext2_size);
		    memcpy(buffer+offset, ext2, ext2_size * sizeof(char));
		    offset += ext2_size * sizeof(char);

		    packsize = offset;
		    base.length += packsize;

		    return buffer;
		}

	    virtual void unpack()
		{
		    int offset = base.length - sizeof(ABSBeamAllocEvent);

		    if (offset > 0)
		    {
			char* data = (char*)&base;

			memcpy(&ext1_size, data+offset, sizeof(int));
			offset += sizeof(int);
			ext1 = (int*)(data + offset);
			offset += ext1_size * sizeof(int);

			memcpy(&ext2_size, data+offset, sizeof(int));
			offset += sizeof(int);
			ext2 = (char*)(data + offset);
			offset += ext2_size * sizeof(char);
		    }	
		}

	    virtual Event& getEvent() { return base; }
	};

};

using namespace MyProtocol;

