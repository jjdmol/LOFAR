#include <iostream>
#include <string>
#include <typeinfo>

#define DEBUG(x) TRACE::MESSAGE(x);

    //util vars
extern int rank;
extern bool debug;

class TRACE
{
  static unsigned long long level;
 public:
  static void MESSAGE(const std::string &msg)
  {
    if(debug)
    {
      for(unsigned long long i = 0; i< level ;++i)
      {
	std::cout << '.';
      }
      std::cout << " rank(" << ::rank << "): " << msg << std::endl;
    }
  }
  TRACE(const std::string &initMess):mess(initMess)
  {
    MESSAGE(mess);
    ++level; // maybe this leaves us with a short stack to trace
  }

  ~TRACE()
  {
    --level;
    MESSAGE("no more mr. " + mess);
  }

  std::string mess;
};
