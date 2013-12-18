/* Header file for 'Config Class'
 Author: adv (Andrew Marchenko), 03.08.2000 
         <adv@adv.org.ua>
         <t9r@mail.ru>  */
 
#include <fstream>
#include <string>

#define DEF_CONFIG       "config.cfg"

enum ValType
{
  TYPE_EMPTY,
  TYPE_VARIABLE,
  TYPE_KEYWORD,
  TYPE_BLOCK
};

class Value
{
  std::string value;
  int type;
  int block;
  
 public:
  Value* next;

  Value(void);
  
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
  std::ifstream file;
  char delimiter;
  int vcol;

  Value* svalue, *vstart;

  int open_config_file(const char* name);
  int close_config_file(void);
  int read_config_file(void);
  
  void S_trunc(std::string& str);

  int search_block(const char* block);
  const char* search_value(int col);
  
public:
  Config(void);
  Config(const char* name, int col = 1);
  Config(char delim, int col = 1);
  Config(const char* name, char delim, int col = 1);
  int init(const char* name, char delim, int col);

  ~Config(void);
  
  const char* value(const char* name, int col = 1);
  const char* value(const char* block, const char* name, int col = 1);
  const char* operator()(const char* name, int col = 1);
  const char* operator()(const char* block, const char* name, int col = 1);

  const char* _ivalue(const char* block, const char* name, int index, int col);
  const char* ivalue(int index, int col = 1);
  const char* ivalue(const char* block, int index, int col = 1);
  const char* ivalue(const char* block, const char* name, int index, int col = 1);

  const char* keyword(int index);
  const char* block(int index);
};
