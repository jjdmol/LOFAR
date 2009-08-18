#ifndef LOFAR_LCS_STREAM_FILE_STREAM_H
#define LOFAR_LCS_STREAM_FILE_STREAM_H

#include <Stream/FileDescriptorBasedStream.h>


namespace LOFAR {

class FileStream : public FileDescriptorBasedStream
{
  public:
	    FileStream(const char *name); // read-only; existing file
	    FileStream(const char *name, int mode); // rd/wr; create file
	    FileStream(const char *name, int flags, int mode); // rd/wr; create file, use given flags
						   
    virtual ~FileStream();
};

} // namespace LOFAR

#endif
