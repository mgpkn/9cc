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

  filename = calloc(1,sizeof(char)*500);
  code = read_file(argv[1]);

  Token *tok = tokenize(code);

  //ノードの集団を生成  
  Ident *prog_list = parse(tok);
  
  //アセンブリの出力
  codegen(prog_list);
  
  return 0;
}
