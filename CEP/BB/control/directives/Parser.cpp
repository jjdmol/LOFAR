#include "Parser.h"
#include "parser/selfparse.h"

#include <sstream>
#include <iostream>

#include <map>

//##ModelId=3F4DE363005D
void Parser::setText(const std::string & text)
{
  //lock the fields txt and nested
  std::string tmptxt(text);

  parse(tmptxt);
  //release the fields
}

//##ModelId=3F4DE3900109
const std::string &Parser::getText() const
{
  return txt;
}

//##ModelId=3F4DE3CF031C
std::vector<Directive> &Parser::getNested()
{
  return nested;
}

static std::string *scriptText = 0;
static std::vector<Directive> *directives = 0;

void Parser::parse(std::string txt)
{
  //start mutex

  scriptText = &txt;

  selfparseStream = new std::istringstream(txt);
  selfparseparse();
  // <todo>empty scripts</todo>
  //end mutex
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
	//         scriptname << ":"<<
         selfparselineno << ":" <<
         s << std::endl;
      return 0;
   }

  void saveSubScript(char * bn, char * command, char * options , char * block)
  {
    // generate children into nested;
    // add to nested of the present Parser-object a directive
    // containing std::string(bn + " " + command + " " + options
    // + " " + block) as text, and std::string(bn) as id.

    // we have to decide here what kind of directive to create:
    // "command" holds information on this and maybe also "block".
    // for now:
    Directive *d = new Directive(std::string(bn),
				 std::string(std::string(bn) + " " +
					     command + " " +
					     options + " " +
					     block)
			);
    directives->insert(directives->end(),
		       *d
		       );
  }

  void saveScript( char * command, char * block)
  {
    // replace txt of the present Parser-object by
    // std::string(command + block) both in the object and in
    // the db
    if (scriptText)
    {
      delete scriptText;
    }
    scriptText = new std::string(std::string(command) + " "
				 + block);
  }

#ifdef __cplusplus
}
#endif
