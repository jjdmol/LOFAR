#ifndef DIRECTIVEFACTORY_H_HEADER_INCLUDED_C09168FB
#define DIRECTIVEFACTORY_H_HEADER_INCLUDED_C09168FB
#include "Directive.h"
#include "DirectiveData.h"

//##ModelId=3F6EC3780186
class DirectiveFactory
{
  public:
    //##ModelId=3F6EC39F02EE
    static Directive & makeDirective(
        //##Documentation
        //## The data from the database (or file).
        const DirectiveData & data);

};



#endif /* DIRECTIVEFACTORY_H_HEADER_INCLUDED_C09168FB */
