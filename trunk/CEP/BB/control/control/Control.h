#ifndef CONTROL_H_HEADER_INCLUDED_C0C4D9B0
#define CONTROL_H_HEADER_INCLUDED_C0C4D9B0
class BlackBoard;
#include "Directive.h"
#include "MPIProgramEntry.h"

//##ModelId=3F3A4AD902CE
class Control
{
  public:
    //##ModelId=3F3B90830399
    //##Documentation
    //## usually this method will be called by the parent controller or via
    //## user interaction.
    virtual addDirective(Directive directive) throw(NullPointerException);

    //##ModelId=3F3B4FFD01E4
    BlackBoard *blackboard;
    //##ModelId=3F3B8FF502CE
    Directive theDirective;


};



#endif /* CONTROL_H_HEADER_INCLUDED_C0C4D9B0 */
