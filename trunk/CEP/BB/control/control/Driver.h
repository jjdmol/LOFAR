#ifndef DRIVER_H_HEADER_INCLUDED_C0C4E44C
#define DRIVER_H_HEADER_INCLUDED_C0C4E44C
#include "Control.h"
class LowLevelStrategy;

//##ModelId=3F3A4C5A03C8
class Driver : public Control
{
  public:
    //##ModelId=3F3A4CC0014A
    LowLevelStrategy *controller;

};



#endif /* DRIVER_H_HEADER_INCLUDED_C0C4E44C */
