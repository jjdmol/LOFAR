#ifndef PARSER_H_HEADER_INCLUDED_C0B1CBD5
#define PARSER_H_HEADER_INCLUDED_C0B1CBD5
#include "Directive.h"

#include <string>
#include <vector>

//##ModelId=3F4DE33A0399
class Parser
{
  public:
    //##ModelId=3F4DE363005D
    void setText(std::string text);

    //##ModelId=3F4DE3900109
    std::string getText();

    //##ModelId=3F4DE3CF031C
    std::vector<Directive> *getNested();

};



#endif /* PARSER_H_HEADER_INCLUDED_C0B1CBD5 */
