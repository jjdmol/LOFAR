//# GTM_Config.cc:
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

#include "GTM_Config.h"
#include <Common/lofar_algorithm.h>

int S_count(const std::string& s, char c);

GTMValue::GTMValue(void)
{
  _pNext = 0;
  _type = TYPE_EMPTY;
}

void
GTMValue::set(string str, int bl, int key)
{
  _value = str;
  _type = key;
  _block = bl;
}

string&
GTMValue::get(void)
{
  return _value;
}

const char*
GTMValue::sget(void)
{
  return _value.c_str();
}

int
GTMValue::is_keyword(void)
{
  return (_type == TYPE_KEYWORD) ? 1 : 0;
}

int
GTMValue::is_block(void)
{
  return (_type == TYPE_BLOCK) ? 1 : 0;
}

int
GTMValue::get_block_id(void)
{
  return _block;
}

int
GTMValue::get_type(void)
{
  return _type;
}

/* ----------------------------------------  */

GTMConfig::GTMConfig(void)
{
  init(DEF_CONFIG, '=', 1);
}

GTMConfig::GTMConfig(const char* name, int col)
{
  init(name, '=', col);
}

GTMConfig::GTMConfig(char delim, int col)
{
  init(DEF_CONFIG, delim, col);
}

GTMConfig::GTMConfig(const char* name, char delim, int col)
{
  init(name, delim, col);
}

GTMConfig::GTMConfig(char delim, int col, const char* buf)
{
  init_from_string(buf, delim, col);
}

int
GTMConfig::init(const char* name, char delim, int col)
{
  _delimiter = delim;
  _vcol = col;
  if (_vcol < 1)
      _vcol = 1;
  
  _pSValue = 0;

  open_config_file(name);
  read_config_file();
  close_config_file();
  
  return 0;
}

int
GTMConfig::init_from_string(const char* buf, char delim, int col)
{
  _delimiter = delim;
  _vcol = col;
  if (_vcol < 1)
      _vcol = 1;
  
  _pSValue = 0;

  read_config_from_string(buf);
  
  return 0;
}


GTMConfig::~GTMConfig(void)
{
  if (_pVStart != 0)
    {
      while (_pVStart->_pNext)
        {
          _pSValue = _pVStart->_pNext;
          delete _pVStart;
          _pVStart = _pSValue;
        }
    }
}

int
GTMConfig::open_config_file(const char* name)
{
  //_file = fopen(name, "r");
  _file.open(name);
  if (_file == NULL)
      throw "File Open Error";
    
  return 0;
}

int
GTMConfig::close_config_file(void)
{
  //fclose(_file);
  _file.close();
  
  return 0;
}


const char*
GTMConfig::operator()(string& name, int col)
{
  return value(name, col);
}

const char*
GTMConfig::operator()(const string&  block, string& name, int col)
{
  return value(block, name, col);
}

const char*
GTMConfig::value(const string& block, string& name, int col)
{
  if (name == "" || col < 1)
      return NULL;

  _pSValue = _pVStart;
  
  int block_id = 0;
  block_id = search_block(block);

  while (_pSValue->_pNext)
    {
      if (block_id != _pSValue->get_block_id())
          break;
          
      if (!(_pSValue->get()).compare(name) && _pSValue->is_keyword())
          return search_value(col);

      _pSValue = _pSValue->_pNext;
    }
    
  return NULL;
}

const char*
GTMConfig::value(string& name, int col)
{
  if (name == "" || col < 1)
      return NULL;

  _pSValue = _pVStart;
  
  while (_pSValue->_pNext)
    {
      if (!(_pSValue->get()).compare(name) && _pSValue->is_keyword())
          return search_value(col);

      _pSValue = _pSValue->_pNext;
    }
    
  return NULL;
}

const char*
GTMConfig::search_value(int col)
{
  for (int i = 0; i < col; i++)
    {
      _pSValue = _pSValue->_pNext;
      if (_pSValue->_pNext == NULL || _pSValue->is_keyword())
          return NULL;
    }

  return _pSValue->sget();
}


const char*
GTMConfig::_ivalue(const char* block, const char* name, int index, int col)
{
  if (index < 1 || col < 1)
      return NULL;

  _pSValue = _pVStart;

  int block_id = 0;
  if (block)
  {      
      string sblock(block);
      block_id = search_block(sblock);
  }
    
  int count = 0;
  while (_pSValue->_pNext)
    {
      if (block)
          if (block_id != _pSValue->get_block_id())
              break;

      if (name)
        {
          /* If keyword's value is found  */
          if (_pSValue->is_keyword() && !(_pSValue->get()).compare(name) )
              count++;
        }
      else
        {
          /* If keyword is found  */
          if (_pSValue->is_keyword() )
              count++;
        }

      /* Searching specified value  */
      if (count == index)
          return search_value(col);

      _pSValue = _pSValue->_pNext;
    }
    
  return NULL;
}

int
GTMConfig::search_block(const string& block)
{
  int block_id = 0;
  
  while (_pSValue->_pNext)
    {
      if (!(_pSValue->get()).compare(block) && _pSValue->is_block())
        {
          block_id = _pSValue->get_block_id();
          break;
        }
      _pSValue = _pSValue->_pNext;
    }
    
  return block_id;
}

/* 
 * Return value of variable by index
 */
const char*
GTMConfig::ivalue(int index, int col)
{
  return _ivalue(NULL, NULL, index, col);
}

/* 
 * Return value of variable by index in block
 */
const char*
GTMConfig::ivalue(const char* block, int index, int col)
{
  return _ivalue(block, NULL, index, col);
}

/* 
 * Return value of variable by index in block
 */
const char*
GTMConfig::ivalue(const char* block, const char* name, int index, int col)
{
  return _ivalue(block, name, index, col);
}

const char*
GTMConfig::keyword(int index)
{
  if (index < 1)
      return NULL;

  _pSValue = _pVStart;
  
  int count = 0;
  while (_pSValue->_pNext)
    {
      if (_pSValue->is_keyword())
          count++;

      if (count == index)
          return _pSValue->sget();

      _pSValue = _pSValue->_pNext;
    }
    
  return NULL;
}

const char*
GTMConfig::block(int index)
{
  if (index < 1)
      return NULL;

  _pSValue = _pVStart;
  
  int count = 0;
  while (_pSValue->_pNext)
    {
      if (_pSValue->is_block())
          count++;

      if (count == index)
          return _pSValue->sget();

      _pSValue = _pSValue->_pNext;
    }
    
  return NULL;
}


int
GTMConfig::read_config_file(void)
{
  string str, str_buf;
  int block_no = 0;
  
  int append = 0;
  while (1)
    {
      if (_pSValue == 0)
        {
          _pSValue = new GTMValue;
          _pVStart = _pSValue;
        }
      else if (_pSValue->get_type() != TYPE_EMPTY)
        {
          _pSValue->_pNext = new GTMValue;
          _pSValue = _pSValue->_pNext;
        }
              
      if (getline(_file, str_buf) == 0)   
        break;

      str_buf += '#';
      str_buf = str_buf.substr(0, str_buf.find('#'));
      S_trunc(str_buf);

      if (append)
          str += str_buf;
      else
          str = str_buf;
      
      append = 0;
      
      int pos = str.length();
      if (pos <= 2)
          continue;
      S_trunc(str);

      if (str[str.length() - 1] == '\\')
        {
          int last = str.length() - 1;
          str.replace(last, 1, "");
          append = 1;
          continue;
        }

        
      if (str.length() <= 2 || str[0] == '\r')
          continue;

      if (str[0] == '[' && str[str.length() - 1] == ']')
        {
          pos = str.length() - 1;
          str.replace(pos, 1, "");
          str.replace(0, 1, "");
          
          block_no++;
          _pSValue->set(str, block_no, TYPE_BLOCK);
          continue;
        }

      int is_delim = str.find(_delimiter);
      if (is_delim == -1)
          continue;

      if (str[0] == _delimiter)
          continue;

      str_buf = str.substr(0, str.find(_delimiter));

      S_trunc(str_buf);
      _pSValue->set(str_buf, block_no, TYPE_KEYWORD);

      int col= S_count(str, _delimiter);

      if (col > _vcol)
          col = _vcol;
	  
      pos = 0;
      for (int i = 0; i < col; i++)
        {
          _pSValue->_pNext = new GTMValue;
          _pSValue = _pSValue->_pNext;
	  
          pos = str.find(_delimiter, pos + 1);
          str_buf = str.substr(pos + 1);
          
	  
         if (i < col - 1)
           {
             str_buf += ':';
             str_buf = str_buf.substr(0, str_buf.find(_delimiter));
           }
	  
          S_trunc(str_buf);
          _pSValue->set(str_buf, block_no);
        }
    }

  return 0;
}

int
GTMConfig::read_config_from_string(const char* buf)
{
  string str, str_buf;
  int block_no = 0;
  
  int append = 0;
  while (1)
    {
      if (_pSValue == 0)
        {
          _pSValue = new GTMValue;
          _pVStart = _pSValue;
        }
      else if (_pSValue->get_type() != TYPE_EMPTY)
        {
          _pSValue->_pNext = new GTMValue;
          _pSValue = _pSValue->_pNext;
        }
        
      str_buf = "";
      while (*buf != '\n' && *buf != '\0')
      {
	  str_buf += *buf;
	  buf++;
      }
      if ('\0' == *buf) break;
      if ('\n' == *buf) buf++; // skip over newline

      str_buf += '#';
      str_buf = str_buf.substr(0, str_buf.find('#'));
      S_trunc(str_buf);

      if (append)
          str += str_buf;
      else
          str = str_buf;
      
      append = 0;
      
      int pos = str.length();
      if (pos <= 2)
          continue;
      S_trunc(str);

      if (str[str.length() - 1] == '\\')
        {
          int last = str.length() - 1;
          str.replace(last, 1, "");
          append = 1;
          continue;
        }

        
      if (str.length() <= 2 || str[0] == '\r')
          continue;

      if (str[0] == '[' && str[str.length() - 1] == ']')
        {
          pos = str.length() - 1;
          str.replace(pos, 1, "");
          str.replace(0, 1, "");
          
          block_no++;
          _pSValue->set(str, block_no, TYPE_BLOCK);
          continue;
        }

      int is_delim = str.find(_delimiter);
      if (is_delim == -1)
          continue;

      if (str[0] == _delimiter)
          continue;

      str_buf = str.substr(0, str.find(_delimiter));

      S_trunc(str_buf);
      _pSValue->set(str_buf, block_no, TYPE_KEYWORD);

      int col= S_count(str, _delimiter);

      if (col > _vcol)
          col = _vcol;
	  
      pos = 0;
      for (int i = 0; i < col; i++)
        {
          _pSValue->_pNext = new GTMValue;
          _pSValue = _pSValue->_pNext;
	  
          pos = str.find(_delimiter, pos + 1);
          str_buf = str.substr(pos + 1);
          
	  
         if (i < col - 1)
           {
             str_buf += ':';
             str_buf = str_buf.substr(0, str_buf.find(_delimiter));
           }
	  
          S_trunc(str_buf);
          _pSValue->set(str_buf, block_no);
        }
    }

  return 0;
}

int
S_count(const string& s, char c)
{
  int n = 0;
  string::const_iterator i = find(s.begin(), s.end(), c);
  
  while (i != s.end())
    {
      ++n;
      i = find(i + 1, s.end(), c);
    }
    
  return n;
}

void
GTMConfig::S_trunc(string& str)
{
  int pos = str.length();
  if (pos <= 0)
      return;
      
  int count = 0;
  while (isspace(str[pos - 1]))
    {
      pos--;
      count++;
    }
  str.replace(pos, count, "");

  count = 0;
  pos = 0;
  while (isspace(str[pos]))
    {
      pos++;
      count++;
    }
  str.replace(0, count, "");
  
  if (str[0] == '"' && str[str.length() - 1] == '"')
    {
      pos = str.length() - 1;
      str.replace(pos, 1, "");
      str.replace(0, 1, "");
    }
}
