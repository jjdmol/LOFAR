#include "Directive.h"
#include "scriptSelectClause.h"
#include "DirectiveDataBCA.h"
#include "SelectScriptID.h"
#include "Parser.h"

#include <DTL.h>

#include "blackboard/debug.h"

const std::string Directive::tableName("scripts");

//##ModelId=3F433C9E0159
std::vector<Directive> &Directive::getParts()
{
  TRACE t("Directive::getParts()");
  Parser p;
  // get the script text and parse it
  p.setText(getScript());
  setScript(p.getText());
  return p.getNested();
}

Directive::Directive(const std::string& initId,
		     const std::string& initText):
  id(initId),
  text(initText)
{
  TRACE t("Directive::Directive(" + id + ", " + text + ")");

  // find the record with rec.id == id

    // update the record if it is existing
    // if the record doesn't exist, create it

  if(text != "")
  {
    setScript(text);
  }
  else
  {
    text = getScript();
  }
}

std::string Directive::getScript()
{
  TRACE t("Directive::getScript()");
  dtl::DBView<DirectiveData,scriptSelectClause> view(tableName,
						     DirectiveDataBCA(),
						     "WHERE id = (?)",
						     SelectScriptID());
  DEBUG("view created");
  dtl::DBView<DirectiveData,scriptSelectClause>::select_iterator iter = view.begin();
  DEBUG("select_iterator created");
  iter.Params().id = id;
  DEBUG(std::string("Parameter set: ") + id);
  std::string dat;
  DEBUG("data record == 0");
  if(iter != view.end())
  {
    DEBUG("found something");
    dat = iter->text;
  }
  else
  {
    dat = "";
  }
  return dat;
}

void Directive::setScript(const std::string& text)
{
  TRACE t("Directive::setScript(const std::string&)");
  dtl::DBView<DirectiveData,scriptSelectClause> view(tableName,
						     DirectiveDataBCA(),
						     "WHERE id = (?)",
						     SelectScriptID());
  dtl::DBView<DirectiveData,scriptSelectClause>::select_iterator iter = view.begin();
  DirectiveData d(id,text);
  if(iter != view.end())
  {
    // we update
    dtl::DBView<DirectiveData, scriptSelectClause>::update_iterator updIter = view;
    TRACE u("update_iterator created");
    d.text = text;
    *updIter = d;
  }
  else
  {
    // we need to insert
    dtl::DBView<DirectiveData, scriptSelectClause>::insert_iterator insIter = view;
    TRACE i("insert_iterator created");
    d.id = id;
    d.text = text;
    *insIter = d;
  }
}

// eof $Id$
