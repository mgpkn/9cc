#include "9cc.h"

Type *ty_char = &(Type){TY_CHAR};
//Type *ty_int = &(Type){TY_INT};

void init_nodetype(Node *n);

bool is_ptr_node(Node *n)
{
    if (n->ty->kind == TY_PTR)
        return true;
    if (n->ty->kind == TY_ARRAY)
        return true;
    return false;
}

int get_type_size(Type *ty)
{

    switch (ty->kind){
    case TY_PTR:
        return 8;
    case TY_INT:
        return 4;
    case TY_CHAR:
        return 1;
    case TY_STRUCT:
        return ty->size;
    default:
        error("invalid data type.");
    }
    return -1;
}

int calc_sizeof(Type *ty)
{

    switch (ty->kind)
    {
    case TY_ARRAY:
        return ty->array_size * calc_sizeof(ty->ptr_to);
    case TY_STRUCT:
        return ty->size;
    case TY_PTR:
    case TY_INT:    
    case TY_CHAR:    
        return get_type_size(ty);
    default:
        error("invalid data type.");
    }

    return -1;
}



// 各ノードの論理的な型を設定
void init_nodetype(Node *n)
{

    if (!n || n->ty)
        return;

    Type *t = calloc(1, sizeof(Type));

    // 再帰的に下位ノードにも型タイプをinit
    init_nodetype(n->lhs);
    init_nodetype(n->rhs);
    init_nodetype(n->init);
    init_nodetype(n->cond);
    init_nodetype(n->inc);
    init_nodetype(n->then);
    init_nodetype(n->els);
    for (Node *block_n = n->block_head; block_n; block_n = block_n->next)
    {
        init_nodetype(block_n);
        if (!block_n->next)
        {
            t = block_n->ty;
            n->ty = t;
            return;
        }
    }

    for (int i; i < FUNC_ARG_NUM; i++)
        init_nodetype(n->func_arg[i]);

    // ノードの種類によってtyを決定する。
    switch (n->kind)
    {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_MOD:
    case ND_ASSIGN:
        n->ty = n->lhs->ty;
        return;
    case ND_EQ:
    case ND_NOTEQ:
    case ND_LLESS:
    case ND_LLESSEQ:
    case ND_LVAR:
    case ND_FUNC:
    case ND_NUM:
        t->kind = TY_INT;
        t->size = calc_sizeof(t);
        n->ty = t;
        return;
    case ND_CHAR:
        t->kind = TY_CHAR;
        t->size = calc_sizeof(t);
        n->ty = t;
        return;
    case ND_ADDR:
        t->kind = TY_PTR;
        t->size = calc_sizeof(t);
        t->ptr_to = n->lhs->ty;
        n->ty = t;
        return;
    case ND_DEREF:
        n->ty = n->lhs->ty->ptr_to;
        return;
    case ND_COMMA:
        n->ty = n->rhs->ty;
        ;
        return;
    default:
        return;
    }
}
