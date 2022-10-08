#include "9cc.h"

void generate_assemble_code(Node* current_node,bool is_first_call){

  // アセンブリの前半部分を出力
  if(is_first_call){  
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
  }    

  //内容の四則演算を入力
  if(current_node->kind == ND_NUM){
    printf("  push %d\n",current_node -> val );
  }
  else{
    generate_assemble_code(current_node->lhs,false);
    generate_assemble_code(current_node->rhs,false);    

    printf("  pop rdi\n");
    printf("  pop rax\n");
	
    switch(current_node->kind){
    case  ND_ADD:
      printf("  add rax, rdi\n");    
      break;
    case  ND_SUB:
      printf("  sub rax, rdi\n");        
      break;    
    case  ND_MUL:
      printf("  imul rax, rdi\n");        
      break;    
    case  ND_DIV:
      printf("  cqo\n");    
      printf("  idiv rdi\n");
      break;
    case  ND_MOD:
      printf("  cqo\n");    
      printf("  idiv rdi\n");
      printf("  mov rax,rdx\n");      
      break;
    case ND_EQ:
      printf("  cmp rax,rdi\n");    
      printf("  sete al\n");
      printf("  movzb rax,al\n");      
      break;      
    case ND_NOTEQ:
      printf("  cmp rax,rdi\n");    
      printf("  setne al\n");
      printf("  movzb rax,al\n");      
      break;
    case ND_LLESS:
      printf("  cmp rax,rdi\n");    
      printf("  setl al\n");
      printf("  movzb rax,al\n");            
      break;      
    case ND_LLESSEQ:
      printf("  cmp rax,rdi\n");    
      printf("  setle al\n");
      printf("  movzb rax,al\n");            
      break;            
    }
    printf("  push rax\n");
  }

  //アセンブリの終了部分を出力
  if(is_first_call){
    printf("  pop rax\n");
    printf("  ret\n");
  }
  
}





  
