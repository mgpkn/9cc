#include "9cc.h"

void init_nodetype(Node *n);

bool is_num_node(Node *n){
    if(n->ty->kind == TY_INT) return true;
    return false;
}

bool is_ptr_node(Node *n){
    if(n->ty->kind == TY_PTR) return true;
    if(n->ty->kind == TY_ARRAY) return true;    
    return false;
}

int get_type_size(Type *ty)
{

  if (ty->kind == TY_ARRAY) return ty->array_size * get_type_size(ty->ptr_to);   
  if (ty->kind == TY_PTR) return 8;  
  if (ty->kind == TY_INT) return 4;
  if (ty->kind == TY_CHAR) return 1;

  error_at(NULL,"invalid data type.");
  return 0;
}

//各ノードの論理的な型を設定
void init_nodetype(Node *n){

    if(!n || n->ty) return;

    //再帰的に下位ノードにも型タイプをinit
    init_nodetype(n->lhs);
    init_nodetype(n->rhs);
    init_nodetype(n->init);
    init_nodetype(n->cond);
    init_nodetype(n->inc);
    init_nodetype(n->then);
    init_nodetype(n->els);
    for(Node *block_n=n->block_head;block_n;block_n=block_n->next) {
        init_nodetype(block_n);
    }

    Type *t = calloc(1,sizeof(Type));
    //ノードの種類によってtyの
    switch(n->kind){
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_MOD:
    case ND_ASSIGN:
        n->ty=n->lhs->ty;
        return;
    case ND_EQ:
    case ND_NOTEQ:
    case ND_LLESS:
    case ND_LLESSEQ:
    case ND_LVAR:
    case ND_FUNC:
    case ND_NUM:
        t->kind = TY_INT;
        n->ty = t;
        return;            
    case ND_ADDR:
        t->kind = TY_PTR;
        t->ptr_to = n->lhs->ty;
        n->ty = t;        
        return;    
    case ND_DEREF:
        n->ty = n->lhs->ty->ptr_to;    
        return;
    default:
        return;
    }     
}
