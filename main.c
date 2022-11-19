#include "9cc.h"

char *user_input;//main関数の引数
Token *token;// 現在着目しているトークン
Node *code[100];

int main(int argc, char **argv) {

  Node* node;//やがて消す
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
  node = statment();
  generate_assemble_code_header();    
  generate_assemble_code_body(node);
  generate_assemble_code_footer();  
  return 0;

}
