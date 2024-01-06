#include "9cc.h"

char *user_input; // main関数の引数

int main(int argc, char **argv)
{
  
  if (argc != 2)
  {
    error("Invalid number of args.");
    return 1;
  }

  user_input= argv[1];

  Token *tok = tokenize(argv[1]);

  //ノードの集団を生成  
  Ident *prog_list = parse(tok);
  
  //アセンブリの出力
  codegen(prog_list);
  
  return 0;
}
