#include "Parser.h"
#include "parser/selfparse.h"
#include "blackboard/debug.h"

#include <sstream>
#include <iostream>

#include <map>

//##ModelId=3F4DE363005D
void Parser::setText(const std::string & text)
{
  TRACE t("Parser::setText(const std::string &)");
  //lock the fields txt and nested
  std::string tmptxt(text);

  parse(tmptxt);
  //release the fields
}

//##ModelId=3F4DE3900109
const std::string &Parser::getText() const
{
  TRACE t("Parser::getText()");
  return txt;
}

//##ModelId=3F4DE3CF031C
std::vector<Directive> &Parser::getNested()
{
  TRACE t("Parser::getNested()");
  return nested;
}

static std::string *scriptText = 0;
static std::vector<Directive> *directives = 0;
pthread_mutex_t parser_mutex = PTHREAD_MUTEX_INITIALIZER;

void Parser::parse(const std::string& text)
{
  TRACE t("Parser::parse(std::string)");
  //start mutex
  pthread_mutex_lock(&parser_mutex);
  if(directives)
  {
    delete directives;
  }
  directives = new std::vector<Directive>;
  DEBUG("creating stream to parse");
  selfparseStream = new std::istringstream(text);
  DEBUG("start to parse");
  selfparseparse();
  DEBUG("retrieving result");
  this->txt = *scriptText;
  this->nested = *directives;
  // <todo>empty scripts</todo>
  pthread_mutex_unlock(&parser_mutex);
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
      std::ostringstream os;
      os <<  selfparselineno << ":" << s;
      DEBUG(os.str());
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
    TRACE t(std::string("saveSubScript(") + bn + ",...)");
    std::string newScript(std::string(bn) + " " +
			  command + " " +
			  (options ? options : "") +
			  " " + //the only optional parameter (would have to be last to give it a default value)
			  block);
    Directive *d = new Directive(std::string(bn),
				 newScript);
    DEBUG("new directive created");
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
