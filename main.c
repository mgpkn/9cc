#include "9cc.h"

char *user_input; // main関数の引数

int main(int argc, char **argv)
{
  Ident *func_list;

  if (argc != 2)
  {
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input= argv[1];

  // トークナイズする
  Token *tok = tokenize(argv[1]);

  //ノードの集団を生成  
  func_list = parse(tok);
  
  //アセンブリの出力
  codegen(func_list);
  
  return 0;
}
