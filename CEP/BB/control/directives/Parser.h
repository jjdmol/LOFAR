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
    //##Documentation
    //## Where does this documentation go?
  void setText(const std::string & text);

  //##ModelId=3F4DE3900109
  const std::string &getText() const;

  //##ModelId=3F4DE3CF031C
  const std::vector<Directive> &getNested() const;

    //##ModelId=3F534BAA00CC
  Parser():txt(""),nested(){};
    //##ModelId=3F534BAA00DA
  virtual ~Parser(){};

 private:
  //##ModelId=3F53420E0177
  std::string txt;
  //##ModelId=3F53423103C8
  std::vector<Directive> nested;


};

#endif /* PARSER_H_HEADER_INCLUDED_C0B1CBD5 */
