//# Config.h:
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef CONFIG_H
#define CONFIG_H
 
//#include <Common/lofar_fstream.h>
//#include <Common/lofar_string.h>
#include <fstream>
#include <string>

#define DEF_CONFIG "config.cfg"

enum ValType
{
  TYPE_EMPTY,
  TYPE_VARIABLE,
  TYPE_KEYWORD,
  TYPE_BLOCK
};

class ConfigValue
{
  std::string _value;
  int _type;
  int _block;
  
 public:
  ConfigValue* _pNext;

  ConfigValue(void);
  
  void set(std::string str, int bl, int key = TYPE_VARIABLE);
  std::string& get(void);
  const char* sget(void);
  
  int is_keyword(void);
  int is_block(void);
  int get_block_id(void);
  int get_type(void);
};


class Config
{
  std::ifstream _file;
  char _delimiter;
  int _vcol;

  ConfigValue* _pSValue, *_pVStart;

  int open_config_file(const char* name);
  int close_config_file(void);
  int read_config_file(void);
  int read_config_from_string(const char* buf);
  
  void S_trunc(std::string& str);

  int search_block(const std::string& block);
  const char* search_value(int col);
  
public:
  Config(void);
  Config(const char* name, int col = 1);
  Config(char delim, int col = 1);
  Config(const char* name, char delim, int col = 1);
  Config(char delim, int col, const char* buf);
  int init(const char* name, char delim, int col);
  int init_from_string(const char* buf, char delim, int col);

  ~Config(void);
  
  const char* value(const char*, int col = 1);
  const char* value(std::string& name, int col = 1);
  const char* value(const char* block, const char* name, int col = 1);
  const char* value(const std::string& block, std::string& name, int col = 1);
  const char* operator()(const char* name, int col = 1);
  const char* operator()(std::string& name, int col = 1);
  const char* operator()(const char* block, const char* name, int col = 1);
  const char* operator()(const std::string& block, std::string& name, int col = 1);

  const char* _ivalue(const char* block, const char* name, int index, int col);
  const char* ivalue(int index, int col = 1);
  const char* ivalue(const char* block, int index, int col = 1);
  const char* ivalue(const char* block, const char* name, int index, int col = 1);

  const char* keyword(int index);
  const char* block(int index);
};
 
#endif
