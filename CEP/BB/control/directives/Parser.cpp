#include "Parser.h"

//##ModelId=3F4DE363005D
void Parser::setText(const std::string & text)
{
  //lock the fields txt and nested
  std::string tmptxt(text);

  // parse tmptxt;
  // generate children into nested;
  // fill txt replacing "part do .." for part "#.#"

  //release the fields
}

//##ModelId=3F4DE3900109
const std::string &Parser::getText() const
{
  return txt;
}

//##ModelId=3F4DE3CF031C
const std::vector<Directive> &Parser::getNested() const
{
  return nested;
}

