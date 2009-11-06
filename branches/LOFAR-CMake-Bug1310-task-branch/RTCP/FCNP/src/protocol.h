#if !defined FORMAT_H
#define FORMAT_H

struct RequestPacket {
  enum {
    ZERO_COPY_READ,
    ZERO_COPY_WRITE,
    RESET
  }		 type;
  unsigned	 rank;
  unsigned short core;
  unsigned short rankInPSet; // logical; not the incomprehensible BG/P number!
  unsigned	 size;
  char		 messageHead[240];
};

#endif
