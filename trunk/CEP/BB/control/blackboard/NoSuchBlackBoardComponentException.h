#ifndef NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444
#define NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444
#include <exception>

//##ModelId=3F422885004E
class NoSuchBlackBoardComponentException:public virtual std::Exception
{
  NoSuchBlackBoardComponentException(const string &txt):std:Exception(txt.c_str())
    {
    }
};



#endif /* NOSUCHBLACKBOARDCOMPONENTEXCEPTION_H_HEADER_INCLUDED_C0BDD444 */
