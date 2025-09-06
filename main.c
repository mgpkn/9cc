#include "9cc.h"

char *code;
char *filename;

int main(int argc, char **argv)
{
  
  if (argc != 2)
  {
    error("Invalid number of args.");
    return 1;
  }

  //read code string from file.
  filename = calloc(1,sizeof(char)*500);
  code = read_file(argv[1]);

  //create token list.
  Token *tok = tokenize(code);

  //create node list.
  Ident *prog_list = parse(tok);
  
  //emit assemble code 
  codegen(prog_list);
  
  return 0;
}
