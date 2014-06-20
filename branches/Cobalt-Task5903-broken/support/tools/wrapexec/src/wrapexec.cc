/* wrapexec.c: run a binary under the control of some tool such as valgrind or
 *          gdb without the knowledge of the invoking program (e.g. shell or glish).
 *          This is useful to debug and profile programs that are invoked as
 *          part of some framework which doesn't support starting those binaries
 *          under gdb or valgrind control.
 *
 *  Scenario:
 *  One wants to run binary "analysis" under valgrind control. To do this
 *  you create a symbolic link from analysis.wrap_valgrind to the wrapexec binary:
 *  e.g. ln -s /usr/local/bin/wrapexec analysis. Now you change the invokation of the
 *  program from "analysis" to "analysis.wrap_valgrind". Now the correct binary analysis
 *  will be started under the control of valgrind.
 *
 *  Copyright (C) 2000,2001,2002
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 *
 *  $Log$
 *  Revision 1.2  2003/01/14 14:26:03  wierenga
 *  %[BugId: 158]%
 *
 *  Now uses $HOME/.wrapexec.cfg instead of $HOME/.wrapexec
 *
 *  wrapexec script checks for existence of system or user configuration file.
 *
 *  Fixed problem with extracting inode from 'ls -iL' output.
 *
 *  Revision 1.1  2003/01/08 10:03:56  wierenga
 *  %[BugId: 158]%
 *
 *  This is the initial implementation of the wrapexec tool.
 *
 *
 */

#include <Config.h>
#include <glob.h>
#include <ArgList.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <wrapexec_paths.h>

#define CONFIG_PATH_LEN 512
#define CONFIG_ARG_LEN  512

string substitute(const char* subst_in, int argc, char** argv)
{
  ArgList a(subst_in);
  ArgList allargs(argc, argv);
  ArgList optionargs(argc-1, &argv[1]);

#ifdef DEBUG
  fprintf(stderr, "allargs = %s\n", allargs.as_string().c_str());
#endif

  a.substitute("%%", allargs.as_string());

#ifdef DEBUG
  fprintf(stderr, "optionargs = %s\n", optionargs.as_string().c_str());
#endif

  a.substitute("%*", optionargs.as_string());

#ifdef DEBUG
  fprintf(stderr, "substitute individual args\n");
#endif

  char acpid[10];
  sprintf(acpid, "%d", getpid());
  a.substitute("%pid", string(acpid));

#ifdef DEBUG
  cerr << "%pid = " << string(acpid) << endl;
#endif

  for (int i = 0; i < argc; i++)
  {
    char pat[8];
    sprintf(pat, "%%%d", i);
    a.substitute(pat, argv[i]);
  }

  return a.as_string();
}

Config* config_read_file()
{
  string cfgname;
  Config* cfg = 0;

  //
  // Construct path to config file
  //
  if (getenv("HOME"))
  {
    // construct the config file name
    cfgname = string(getenv("HOME")) + string("/.wrapexec.cfg");

    // try to read the config file
    try { cfg = new Config(cfgname.c_str()); }
    catch (const char* s)
    {
      delete cfg;
      cfg = 0;

      // failed to read $HOME/.wrapexec.cfg
      // now read $(datadir)/wrapexec.cfg
      cfgname = string(WRAPEXEC_CFG_PATH);
      try { cfg = new Config(cfgname.c_str()); }
      catch (const char* s)
      {
	delete cfg;
	cfg = 0;
      }
    }
  }

  return cfg;
}

bool config_find_tool_section(Config* cfg, const string& name)
{
  bool result = false;
  int  i      = 1;

  if (!cfg) return false;

  //
  // Find app name in configuration
  //
  while(cfg->block(i))
  {
#ifdef DEBUG
    printf("block %d = %s\n", i, cfg->block(i));
#endif
    if (string(cfg->block(i)) == name)
    {
      result = true;
      break;
    }
    i++;
  }

  return result;
}

string extract_tool_name(const char* path)
{
  string result = string(path);
  int pos;

  //
  // Get the name of the wrapper app
  //
  pos = result.find_last_of('_');
  if (pos < 0)
  {
    result = "";
  }
  else
  {
    result = result.substr(pos+1);
  }

  return result;
}

string extract_program_name(const char* path)
{
  string result = string(path);
  
  //
  // Get position of last '/' and last _
  //
  int slash  = result.find_last_of('/');
  int uscore = result.find_last_of('_');
  if (slash < 0 || uscore < 0)
  {
    result = "";
  }
  else
  {
    result = result.substr(slash+1,uscore);
  }

  return result;
}

int main(int argc, char* argv[])
{
  Config* cfg = 0;

  cfg = config_read_file();
  if (!cfg)
  {
    cerr << "wrapexec_wrapper: error: could not read configuration file from '"
	 << WRAPEXEC_CFG_PATH << "' or " << "'$HOME/.wrapexec.cfg'" << endl;
    exit(1);
  }

  //
  // Determine path to the binary
  //
  string apppath;
  char *cwd = getcwd(NULL, 0);
  if ('/' == *argv[0])
  {
    apppath = string(argv[0]);
  }
  else
  {
    apppath = string(cwd) + string("/") + string(argv[0]);
  }
  free(cwd);

  //
  // Find a wrapper link, e.g. binary.wrap_valgrind
  //
  glob_t globbuf;
  globbuf.gl_offs = 0;
  int globerr = 0;
  string searchstring = apppath + string(".wrap_*");

#ifdef DEBUG
  cerr << "globpattern = " << searchstring << endl;
#endif

  if ((globerr = glob(searchstring.c_str(),
		      GLOB_ERR | GLOB_MARK | GLOB_DOOFFS, NULL, &globbuf)) < 0)
  {
    switch(globerr)
    {
      case GLOB_NOSPACE:
	fprintf(stderr, "Error: out of memory while searching for wrapper link.\n");
	break;
      case GLOB_ABORTED:
	fprintf(stderr, "Error: read error while searching for wrapper link.\n");
	break;
      case GLOB_NOMATCH:
	fprintf(stderr, "Error: no wrapper links found.\n");
	break;
      default:
	fprintf(stderr, "Error: unknown error while searching for wrapper links.\n");
	break;
    }
    exit(1);
  }

  if (0 == globbuf.gl_pathv[0])
  {
    fprintf(stderr, "Error: could not find wrapper link. Searched for '%s'\n",
	    searchstring.c_str());
    exit(1);
  }

  // absolute path to the real binary is in gl_pathv[0]
  string programname = string(globbuf.gl_pathv[0]);
  if ("" == programname)
  {
    cerr << "Error: could not find program name." << endl;
    exit(1);
  }
  argv[0] = strdup(programname.c_str());

  // extract the tool name from the first matching file name
  string toolname = extract_tool_name(globbuf.gl_pathv[0]);
  if ("" == toolname)
  {
    cerr << "Error: could not find wrapper tool name." << endl;
    exit(1);
  }

  // free the glob buffer
  globfree(&globbuf);

  // check if the tool section exists
  if (!config_find_tool_section(cfg, toolname))
  {
    cerr << "Error: could not find section for tool '" << toolname
	 << "' in configuration file." << endl;
    exit(1);
  }

  // read tool definition from the selected tool section of the configuration file
  const char* init       = cfg->value(toolname.c_str(), "init");
  const char* tool       = cfg->value(toolname.c_str(), "tool");
  const char* toolstdout = cfg->value(toolname.c_str(), "stdout");
  const char* toolstderr = cfg->value(toolname.c_str(), "stderr");
  const char* toolargs   = cfg->value(toolname.c_str(), "toolargs");

  string sinit;
  string stoolargs;

  // substitute variables in the 'init' option
  if (init)
  {
    sinit = substitute(init, argc, argv);
  }

  // assign default value for 'toolargs' if needed
  if (!toolargs)
  {
    toolargs="%%";
  }

#ifdef DEBUG
  fprintf(stderr, "substitute toolargs\n");
#endif

  // substitute variables in the 'toolargs' option
  stoolargs = substitute(toolargs, argc, argv);

#ifdef DEBUG
  cerr << "toolargs = " << stoolargs << endl;
#endif

#ifdef DEBUG
  // print final values
  if (init)     cerr << "init     = " << sinit     << endl;
  if (tool)     cerr << "tool     = " << tool      << endl;
  if (toolargs) cerr << "toolargs = " << stoolargs << endl;
#endif

  if (sinit.size() > 0)
  {
    cerr << "ShellExec: " << sinit.c_str() << endl;
    system(sinit.c_str());
  }

  if (tool) stoolargs.insert(0, string(tool) + string(" "));

  ArgList toolarglist(stoolargs);
  char**  tal = (char**)toolarglist.get_argv();

  // still needed to free this memory
  free(argv[0]);

#ifdef DEBUG
  cerr << "Execing: " << stoolargs << endl;
#endif

  if (execvp(tal[0], &tal[0]) < 0)
  {
    perror("execvp");
    exit(1);
  }
  
  return 1;
}
