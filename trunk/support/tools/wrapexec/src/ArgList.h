

#ifndef ARGLIST_H
#define ARGLIST_H

#include <vector>
#include <string>

class ArgList
{
 public:
  ArgList(const char* s);
  ArgList(string s);
  ArgList(int argc, char** argv);
  ~ArgList();

  void substitute(const char* pattern, const string& replacement);

  string       as_string();
  int          get_argc();
  const char** get_argv();
  
 private:
  string m_args;

  string m_argv_string;
  char** m_argv;
};

#endif
