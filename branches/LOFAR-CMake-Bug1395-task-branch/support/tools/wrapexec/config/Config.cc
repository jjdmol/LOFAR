/*  Config Class
 Author: adv (Andrew Marchenko), 03.08.2000  
 Modify: adv (Andrew Marchenko), 27.09.2000  
         <adv@adv.org.ua>
         <t9r@mail.ru>  */

#include <algorithm>
#include "Config.h"

int S_count(const std::string& s, char c);


Value::Value(void)
{
  next = 0;
  type = TYPE_EMPTY;
}

void
Value::set(std::string str, int bl, int key = TYPE_VARIABLE)
{
  value = str;
  type = key;
  block = bl;
}

std::string&
Value::get(void)
{
  return value;
}

const char*
Value::sget(void)
{
  return value.c_str();
}

int
Value::is_keyword(void)
{
  return (type == TYPE_KEYWORD) ? 1 : 0;
}

int
Value::is_block(void)
{
  return (type == TYPE_BLOCK) ? 1 : 0;
}

int
Value::get_block_id(void)
{
  return block;
}

int
Value::get_type(void)
{
  return type;
}

/* ----------------------------------------  */

Config::Config(void)
{
  init(DEF_CONFIG, '=', 1);
}

Config::Config(const char* name, int col = 1)
{
  init(name, '=', col);
}

Config::Config(char delim, int col = 1)
{
  init(DEF_CONFIG, delim, col);
}

Config::Config(const char* name, char delim, int col = 1)
{
  init(name, delim, col);
}

int
Config::init(const char* name, char delim, int col)
{
  delimiter = delim;
  vcol = col;
  if (vcol < 1)
      vcol = 1;
  
  svalue = 0;

  open_config_file(name);
  read_config_file();
  close_config_file();
  
  return 0;
}


Config::~Config(void)
{
  if (vstart != 0)
    {
      while (vstart->next)
        {
          svalue = vstart->next;
          delete vstart;
          vstart = svalue;
        }
    }
}

int
Config::open_config_file(const char* name)
{
  file.open(name);
  if (!file)
      throw "File Open Error";
    
  return 0;
}

int
Config::close_config_file(void)
{
  file.close();
  
  return 0;
}


const char*
Config::operator()(const char* name, int col = 1)
{
  return value(name, col);
}

const char*
Config::operator()(const char* block, const char* name, int col = 1)
{
  return value(block, name, col);
}

const char*
Config::value(const char* block, const char* name, int col = 1)
{
  if (name == NULL || col < 1)
      return NULL;

  svalue = vstart;
  
  int block_id = 0;
  block_id = search_block(block);

  while (svalue->next)
    {
      if (block_id != svalue->get_block_id())
          break;
          
      if (!(svalue->get()).compare(name) && svalue->is_keyword())
          return search_value(col);

      svalue = svalue->next;
    }
    
  return NULL;
}

const char*
Config::value(const char* name, int col = 1)
{
  if (name == NULL || col < 1)
      return NULL;

  svalue = vstart;
  
  while (svalue->next)
    {
      if (!(svalue->get()).compare(name) && svalue->is_keyword())
          return search_value(col);

      svalue = svalue->next;
    }
    
  return NULL;
}

const char*
Config::search_value(int col)
{
  for (int i = 0; i < col; i++)
    {
      svalue = svalue->next;
      if (svalue->next == NULL || svalue->is_keyword())
          return NULL;
    }

  return svalue->sget();
}


const char*
Config::_ivalue(const char* block, const char* name, int index, int col)
{
  if (index < 1 || col < 1)
      return NULL;

  svalue = vstart;

  int block_id = 0;
  if (block)
      block_id = search_block(block);
    
  int count = 0;
  while (svalue->next)
    {
      if (block)
          if (block_id != svalue->get_block_id())
              break;

      if (name)
        {
          /* If keyword's value is found  */
          if (svalue->is_keyword() && !(svalue->get()).compare(name) )
              count++;
        }
      else
        {
          /* If keyword is found  */
          if (svalue->is_keyword() )
              count++;
        }

      /* Searching specified value  */
      if (count == index)
          return search_value(col);

      svalue = svalue->next;
    }
    
  return NULL;
}

int
Config::search_block(const char* block)
{
  int block_id = 0;
  
  while (svalue->next)
    {
      if (!(svalue->get()).compare(block) && svalue->is_block())
        {
          block_id = svalue->get_block_id();
          break;
        }
      svalue = svalue->next;
    }
    
  return block_id;
}

/* 
 * Return value of variable by index
 */
const char*
Config::ivalue(int index, int col = 1)
{
  return _ivalue(NULL, NULL, index, col);
}

/* 
 * Return value of variable by index in block
 */
const char*
Config::ivalue(const char* block, int index, int col = 1)
{
  return _ivalue(block, NULL, index, col);
}

/* 
 * Return value of variable by index in block
 */
const char*
Config::ivalue(const char* block, const char* name, int index, int col = 1)
{
  return _ivalue(block, name, index, col);
}



const char*
Config::keyword(int index)
{
  if (index < 1)
      return NULL;

  svalue = vstart;
  
  int count = 0;
  while (svalue->next)
    {
      if (svalue->is_keyword())
          count++;

      if (count == index)
          return svalue->sget();

      svalue = svalue->next;
    }
    
  return NULL;
}

const char*
Config::block(int index)
{
  if (index < 1)
      return NULL;

  svalue = vstart;
  
  int count = 0;
  while (svalue->next)
    {
      if (svalue->is_block())
          count++;

      if (count == index)
          return svalue->sget();

      svalue = svalue->next;
    }
    
  return NULL;
}


int
Config::read_config_file(void)
{
  std::string str, str_buf;
  int block_no = 0;
  
  int append = 0;
  while (1)
    {
      if (svalue == 0)
        {
          svalue = new Value;
          vstart = svalue;
        }
      else if (svalue->get_type() != TYPE_EMPTY)
        {
          svalue->next = new Value;
          svalue = svalue->next;
        }
        
      if (getline(file, str_buf) == 0)
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
          svalue->set(str, block_no, TYPE_BLOCK);
          continue;
        }

      int is_delim = str.find(delimiter);
      if (is_delim == -1)
          continue;

      if (str[0] == delimiter)
          continue;

      str_buf = str.substr(0, str.find(delimiter));

      S_trunc(str_buf);
      svalue->set(str_buf, block_no, TYPE_KEYWORD);

      int col= S_count(str, delimiter);

      if (col > vcol)
          col = vcol;
	  
      pos = 0;
      for (int i = 0; i < col; i++)
        {
          svalue->next = new Value;
          svalue = svalue->next;
	  
          pos = str.find(delimiter, pos + 1);
          str_buf = str.substr(pos + 1);
          
	  
         if (i < col - 1)
           {
             str_buf += ':';
             str_buf = str_buf.substr(0, str_buf.find(delimiter));
           }
	  
          S_trunc(str_buf);
          svalue->set(str_buf, block_no);
        }
    }

  return 0;
}

int
S_count(const std::string& s, char c)
{
  int n = 0;
  std::string::const_iterator i = find(s.begin(), s.end(), c);
  
  while (i != s.end())
    {
      ++n;
      i = find(i + 1, s.end(), c);
    }
    
  return n;
}

void
Config::S_trunc(std::string& str)
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
