#ifndef DIRECTIVE_H_HEADER_INCLUDED_C0C4E44C
#define DIRECTIVE_H_HEADER_INCLUDED_C0C4E44C

#include <vector>
#include <string>

#include "DirectiveData.h"

//##ModelId=3F3B37F70213
class Directive
{
 public:
    //##ModelId=3F3B9602002E
  //  virtual void deploy() = 0;
    //##ModelId=3F433C9E0159
  std::vector<Directive> getParts();

  Directive(const std::string &id = std::string("start"),
	    const std::string &text = std::string(""));

  Directive(const Directive & other):id(other.id),text(other.text)
  {}
  const std::string& getId() const {return id;}
 private:
  std::string getScript();
  void setScript(const std::string& text);
 private:
  static const std::string tableName;
  std::string id;
  std::string text;
};



#endif /* DIRECTIVE_H_HEADER_INCLUDED_C0C4E44C */
