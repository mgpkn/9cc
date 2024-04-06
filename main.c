#include "9cc.h"

char *user_input; // main関数の引数
char *filename; // main関数の引数

int main(int argc, char **argv)
{
  
  if (argc != 2)
  {
    error("Invalid number of args.");
    return 1;
  }

  filename = calloc(1,sizeof(char)*500);
  user_input = read_file(argv[1]);

  Token *tok = tokenize(user_input);

  //ノードの集団を生成  
  Ident *prog_list = parse(tok);
  
  //アセンブリの出力
  codegen(prog_list);
  
  return 0;
}
