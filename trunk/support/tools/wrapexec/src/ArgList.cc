#include <ArgList.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

ArgList::ArgList(const char* s) : m_args(s), m_argv_string(""), m_argv(0)
{
}

ArgList::ArgList(string s) : m_args(s), m_argv_string(""), m_argv(0)
{
}

ArgList::ArgList(int argc, char** argv) : m_argv_string(""), m_argv(0)
{
#ifdef DEBUG
  fprintf(stderr, "ArgList ctor: argc = %d\n", argc);
#endif

  m_args.erase();

  for (int i = 0; i < argc; i++)
  {
#ifdef DEBUG
    fprintf(stderr, "argv[%d] = %s\n", i, argv[i]);
#endif

    if (i < argc - 1) m_args += string(argv[i]) + string(" ");
    else              m_args += string(argv[i]);
  }
}

ArgList::~ArgList()
{
  if (m_argv) free(m_argv);
}

string ArgList::as_string()
{
  return m_args;
}

int ArgList::get_argc()
{
  int begin = 0;
  int end   = -1;

  // count number of arguments
  int argc = 0;
  while (((end = m_args.find_first_of(' ', begin)) >= 0) && (begin != end))
  {
    argc++;
    begin = end + 1;
  }
  if (end != m_args.length()) argc++;

  return argc;
}

const char** ArgList::get_argv()
{
  int begin     = 0;
  int end       = -1;
  m_argv_string = m_args;

  // count number of arguments
  int argc = 0;
  while (((end = m_argv_string.find(' ', begin)) >= 0) && (begin != end))
  {
    argc++;
    begin = end + 1;
  }
  if (end != m_argv_string.length()) argc++;

#ifdef DEBUG
  cerr << "m_argv_string = " << m_argv_string << "; get_argv; argc = " << argc << endl;
#endif

  // reallocate argv array
  m_argv = (char**)realloc(m_argv, (argc + 1) * sizeof(char*));
  m_argv[argc] = 0;

  // assign to argv arguments
  begin = 0;
  end   = -1;
  int i=0;
#ifdef DEBUG
  cerr << "m_argv_string.length() = " << m_argv_string.length() << endl;
#endif
  while (((end = m_argv_string.find_first_of(' ', begin)) >= 0) && (begin != end))
  {
    m_argv_string[end] = '\0';
#ifdef DEBUG
    cerr << "begin = " << begin << endl;
#endif
    m_argv[i] = &m_argv_string[begin];
    begin = end + 1;
    i++;
  }
  if (end != m_args.length()) m_argv[argc - 1] = &m_argv_string[begin];

  return (const char**)m_argv;
}

void ArgList::substitute(const char* pattern, const string& replacement)
{
  int begin = 0;
  int end   = 0;

  if (m_args.length() == 0) return;

#ifdef DEBUG
  cerr << "m_args      = " << m_args      << endl;
  cerr << "pattern     = " << pattern     << endl;
  cerr << "replacement = " << replacement << endl;
#endif

  while ((begin = m_args.find(pattern)) >= 0)
  {
    end = begin + strlen(pattern);
    if (end > m_args.length()) end = m_args.length();

#ifdef DEBUG
    fprintf(stderr, "begin = %d\n", begin);
    fprintf(stderr, "end   = %d\n", end);
    fprintf(stderr, "replacementlength = %d\n", replacement.length());
#endif
    
    m_args.replace(begin, end - begin, replacement);
  }

#ifdef DEBUG
  cerr << "result m_args = " << m_args << endl;
  cerr << "m_args length = " << m_args.length() << endl;
#endif
}
