#include "Parser.h"
#include "parser/selfparse.h"

#include <sstream>
#include <iostream>

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

std::vector<std::string> &parse(std::string txt)
{
   static std::vector<std::string> scripts;
   selfparseStream = new std::istringstream(txt);
   selfparseparse();
       // <todo>empty scripts</todo>
   return scripts;
}

#ifdef __cplusplus
extern "C" 
{
#endif
   int report(char * s)
   {
          // if (debug_or_verbose)
      return fprintf(stdout,s);
   }

   int selfparseerror(char * s)
   {
      extern int selfparselineno;
      std::cerr <<
         filename << ":"<<
         selfparselineno << ":" <<
         s << std::endl;
      return 0;
   }

   void saveSubScript(char * bn, char * command, char * options , char * block)
   {
          // add to nested of the present Parser-object a directive
          // containing std::string(command + " " + options + " " +
          // block) as text
   }

   void saveScript( char * command, char * block)
   {
          // replace txt of the present Parser-object by
          // std::string(command + block) both in the object and in
          // the db
   }

#ifdef __cplusplus
}
#endif
