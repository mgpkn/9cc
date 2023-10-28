#include "9cc.h"

static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gennode(Node *current_node);

void load_val(Type *ty){
  if(ty->kind==TY_ARRAY) return; 
  printf("  mov rax,[rax]\n");
}


void gennode_addr(Node* current_node){
  
  switch(current_node->kind){
  case ND_LVAR:
    printf("  lea rax,[rbp - %d]\n",current_node->offset);
    printf("  push rax\n");  
    return;
  case ND_DEREF:
    gennode(current_node->lhs);
    return;
  default:
    break;
  }

  error("代入の左辺値が変数ではありません。");
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
    printf("  je .Lelse%d\n",current_node->label_num);
    gennode(current_node->then);
    printf(".Lelse%d:\n",current_node->label_num);
    if(current_node->els){
      gennode(current_node->els);      
    }
    printf(".Lend%d:\n",current_node->label_num);    
    return;
  case ND_WHILE:
    printf(".Lbegin%d:\n",current_node->label_num);    
    gennode(current_node->cond);
    printf("  pop rax\n");
    printf("  cmp rax,0\n");
    printf("  je .Lend%d\n",current_node->label_num);
    gennode(current_node->then);
    printf("  jmp .Lbegin%d\n",current_node->label_num);    
    printf(".Lend%d:\n",current_node->label_num);    
    return;
  case ND_FOR:
    gennode(current_node->init);    
    printf(".Lbegin%d:\n",current_node->label_num);    
    gennode(current_node->cond);
    printf("  pop rax\n");
    printf("  cmp rax,0\n");
    printf("  je .Lend%d\n",current_node->label_num);
    gennode(current_node->then);
    gennode(current_node->inc);    
    printf("  jmp .Lbegin%d\n",current_node->label_num);    
    printf(".Lend%d:\n",current_node->label_num);    
    return;    
  case ND_NUM:
    printf("  push %d\n",current_node -> val );    
    return;
  case ND_FUNC:
    for(int i=0;current_node->func_param[i];i++){
        gennode(current_node->func_param[i]);              
        printf("  pop %s\n",argreg[i]);        
    }
    printf("  call %s\n",current_node->ident_name);        
    printf("  push rax\n");            
    return;
  case ND_LVAR:
    gennode_addr(current_node);
    printf("  pop rax\n");
    
    switch(get_type_size(current_node->ty)){
    case 1:
    case 4:
      printf("  movsxd rax, DWORD PTR [rax]\n");
      break;
    case 8:      
    default:
      load_val(current_node->ty);
      break;
    }      
    
    printf("  push rax\n");        
    return;
  case ND_ADDR:
    gennode_addr(current_node->lhs);  
    return;
  case ND_DEREF:
    gennode(current_node->lhs);
    printf("  pop rax\n");
    load_val(current_node->ty);
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gennode_addr(current_node->lhs);
    gennode(current_node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    if(current_node->ty->kind == TY_INT)
      printf("  mov [rax],edi\n");        
    else 
      printf("  mov [rax],rdi\n");        
    printf("  mov rax,[rax]\n");            
    printf("  push rax\n");            
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
  
  //左右のノードを比較するコード
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

void codegen_func(Ident *func){

  Node *cur_code,*cur_arg;  
  Ident *cur_localvar=NULL;
  int cur_offset;

  //関数名のラベルを作成
  printf("%s:\n",func->name);

  //変数領域の確保。
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  cur_offset=0;
  cur_localvar = func->localvar;
  while(cur_localvar){
    cur_offset = cur_localvar->offset;
    cur_localvar=cur_localvar->next;
  }
  
  int i=0;
  while(true){      
    if(cur_offset<=i*BASE_ALIGNMENTSIZE) break;
    i++;
  }
  
  printf("  sub rsp, %d\n",i*BASE_ALIGNMENTSIZE);  

  //引数をレジスタからロード。
  cur_arg = func->arg;
  for(int i=0;cur_arg;i++){
      gennode_addr(cur_arg);

      printf("  pop rax\n");
      printf("  mov [rax],%s\n",argreg[i]);        
      printf("  push [rax]\n");            
      cur_arg = cur_arg->next;
  }

  //関数の本文を記述。
  cur_code=func->body;  
  while(cur_code)
  {
    gennode(cur_code);
    cur_code = cur_code->next;
  }

/*
  //エピローグ    
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");    //関数名のラベルを作成
*/

}

void codegen(Ident *func_list){

  Ident *cur_func=func_list;
  
  //プロローグ
  printf(".intel_syntax noprefix\n");
  printf(".globl ");
  while(cur_func){
    printf("%s",cur_func->name);
    cur_func=cur_func->next;
    if(cur_func) printf(",");
  }
  printf("\n");  

  //各関数を生成
  cur_func=func_list;
  while(cur_func){
    codegen_func(cur_func);
    cur_func=cur_func->next;    
  }

  return;

}

