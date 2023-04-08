#include "9cc.h"

void gennode_lval(Node* current_node){
  
  if(current_node->kind !=ND_LVAL)
    error("代入の左辺値が変数ではありません。");
  printf("  mov rax,rbp\n");
  printf("  sub rax,%d\n",current_node->offset);
  printf("  push rax\n");  
}


void gennode(Node* current_node){

  Node *n;
  
  if(!current_node) return;
  
  switch(current_node->kind){
  case ND_RETURN:
    gennode(current_node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF:
    gennode(current_node->cond);
    printf("  pop rax\n");
    printf("  cmp rax,0\n");
    printf("  je .lelse%d\n",current_node->label_num);
    gennode(current_node->then);
    printf(".lelse%d:\n",current_node->label_num);
    if(current_node->els){
      gennode(current_node->els);      
    }
    printf(".lend%d:\n",current_node->label_num);    
    return;
  case ND_WHILE:
    printf(".lbegin%d:\n",current_node->label_num);    
    gennode(current_node->cond);
    printf("  pop rax\n");
    printf("  cmp rax,0\n");
    printf("  je .lend%d\n",current_node->label_num);
    gennode(current_node->then);
    printf("  jmp .lbegin%d\n",current_node->label_num);    
    printf(".lend%d:\n",current_node->label_num);    
    return;
  case ND_FOR:
    gennode(current_node->init);    
    printf(".lbegin%d:\n",current_node->label_num);    
    gennode(current_node->cond);
    printf("  pop rax\n");
    printf("  cmp rax,0\n");
    printf("  je .lend%d\n",current_node->label_num);
    gennode(current_node->then);
    gennode(current_node->inc);    
    printf("  jmp .lbegin%d\n",current_node->label_num);    
    printf(".lend%d:\n",current_node->label_num);    
    return;    
  case ND_NUM:
    printf("  push %d\n",current_node -> val );    
    return;
  case ND_FUNC:
    printf("  call %s\n",current_node->ident_name);        
    printf("  push rax\n");            
    return;
  case ND_LVAL:
    gennode_lval(current_node);
    printf("  pop rax\n");
    printf("  mov rax,[rax]\n");
    printf("  push rax\n");        
    return;
  case ND_ASSIGN:
    gennode_lval(current_node->lhs);
    gennode(current_node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax],rdi\n");        
    printf("  push [rax]\n");            
    return;
  case ND_BLOCK:
    n = current_node->block_head;
    while(n){
      gennode(n);
      n=n->next;
    }
  default:
    break;
  }
  
  gennode(current_node->lhs);
  gennode(current_node->rhs);    

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
  default:
    break;
    
  }
  printf("  push rax\n");
}

void codegen(Node **code){

  int i =0;

  //プロローグ
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  
  //変数領域の確保。（今は26個で固定）
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");  

  for (i = 0; code[i]; i++)
  {
    gennode(code[i]);
  }

  //エピローグ
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return;

}

