#include "9cc.h"

char *user_input;//main関数の引数
Token *token;// 現在着目しているトークン

Node *code[NODENUM];
LVar *locals;//ローカル変数のセット


int main(int argc, char **argv) {

  Token head;
  int i;
  
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズする
  user_input = argv[1];
  //fprintf(stderr,"begin takenize\n");                          
  token = tokenize(user_input);

  program();//ノードの集団を生成

  //アセンブリの出力
  generate_assemble_header();  
  for(i=0;code[i];i++){
    generate_assemble_statement(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");  
    
  }
  generate_assemble_footer();  

  return 0;

}
