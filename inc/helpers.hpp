#ifndef HELPERS_HPP
#define HELPERS_HPP

//#include <stdio.h>

extern class instr *global_instr;

class arg {
  public:
  char    *name;
  int      tip_arg;
  struct arg *next;
};

class instr {
  public:
  char    *label;
  char    *name;
  struct arg *args;
  struct instr *next;
};

char* copy_str(const char*);

class arg* mk_argument(char*, int, class arg*);
class instr* mk_instruction(char*,char*, class arg*, class instr*);

void print_instrs(class instr *);
void free_instrs(class instr*);


#endif