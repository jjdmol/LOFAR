#ifndef NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444
#define NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444
#include <stdexcept>

#include <iostream>

//##ModelId=3F422885004E
class NoSuchBlackBoardComponentException:public std::runtime_error
{
   public:
      NoSuchBlackBoardComponentException(const std::string &txt):std::runtime_error(txt.c_str())
      {
         DEBUG("creating my own " << typeid(NoSuchBlackBoardComponentException).name() << " what? " << what());
      }
};



#endif /* NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444 */
