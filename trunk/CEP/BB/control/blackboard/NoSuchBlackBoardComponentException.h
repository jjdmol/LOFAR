#ifndef NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444
#define NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444
#include <stdexcept>

#include <iostream>

//##ModelId=3F422885004E
class NoSuchBlackBoardComponentException //:public std::runtime_error
{
   public:
    //##ModelId=3F4232F502CE
      NoSuchBlackBoardComponentException(const std::string &txt):messg(txt)//std::runtime_error(txt.c_str())
      {
         DEBUG(std::string("creating my own ") + typeid(NoSuchBlackBoardComponentException).name() + " what? " + what());
      }

    //##ModelId=3F4CB72C0148
      virtual ~NoSuchBlackBoardComponentException()
      {
      }
      

    //##ModelId=3F4CB72C014A
      virtual const std::string &what() const throw()
      {
         return messg;
      }
    //##ModelId=3F4CB72C0138
      std::string messg;
};



#endif /* NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444 */
