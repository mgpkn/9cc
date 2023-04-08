#include "9cc.h"

char *user_input; // main関数の引数

int main(int argc, char **argv)
{
  Node **code;

  if (argc != 2)
  {
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input= argv[1];

  // トークナイズする
  Token *tok = tokenize(argv[1]);

  //ノードの集団を生成  
  code = parse(tok);
  
  //アセンブリの出力
  codegen(code);
  
  return 0;
}
