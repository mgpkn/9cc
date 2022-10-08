#include "9cc.h"

char *user_input;//main関数の引数
Token *token;// 現在着目しているトークン

int main(int argc, char **argv) {

  Node* node;
  Token head;
  
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズする
  user_input = argv[1];
  //fprintf(stderr,"begin takenize\n");                          
  token = tokenize(user_input);

  //list_token(&head);

  //fprintf(stderr,"begin create node\n");                          
  node = expr();
  generate_assemble_code(node,true);

  return 0;

}

  
