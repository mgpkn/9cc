#include "9cc.h"

Type *ty_char = &(Type){TY_CHAR, 1, 1};
Type *ty_int = &(Type){TY_INT, 4, 4};
Type *ty_long = &(Type){TY_LONG, 8, 8};

void init_nodetype(Node *n);

bool is_ptr_node(Node *n)
{
    if (n->ty->kind == TY_PTR)
        return true;
    if (n->ty->kind == TY_ARRAY)
        return true;
    return false;
}

Type *get_common_type(Type *lhs_ty, Type *rhs_ty)
{
    Type *ty;

    if (lhs_ty->ptr_to)
    {
        ty = calloc(1, sizeof(Type));
        ty->kind = TY_PTR;
        ty->ptr_to = lhs_ty->ptr_to;
        return ty;
    }

    if (lhs_ty->size == 8 || rhs_ty->size == 8)
        return ty_long;

    return ty_int;
}

void usual_arith_conv(Node **lhs, Node **rhs)
{
    Type *ty = get_common_type((*lhs)->ty, (*rhs)->ty);
    *lhs = new_node_cast(*lhs, ty);
    *rhs = new_node_cast(*rhs, ty);
}

// Round up `n` to the nearest multiple of `align`. For instance,
// align_to(0, 1) returns 0 and align_to(11, 4) returns 12.
int align_to(int offset, int align)
{
    // if offset is already a multiple of align, return the current offset.
    if ((offset % align) == 0)
        return offset;

    // otherwise, return the next larger multiple of align from the current offset.
    int i;
    for (i = 0; i * align < offset; i++)
        ;
    return i * align;
};

int calc_sizeof(Type *ty)
{

    switch (ty->kind)
    {
    case TY_ARRAY:
        // return multiple of array size and base type size.
        return ty->array_size * calc_sizeof(ty->ptr_to);
    case TY_PTR:
        return 8;
    case TY_VOID:
        return 1;
    case TY_CHAR:
        return 1;
    case TY_SHORT:
        return 2;
    case TY_INT:
        return 4;
    case TY_LONG:
        return 8;
    case TY_LLONG:
        return 8;
    case TY_STRUCT:
        return ty->size;
    case TY_UNION:
        return ty->size;
    default:
        error("invalid data type.");
    }

    return -1;
}

int calc_alignof(Type *ty)
{
    switch (ty->kind)
    {
    case TY_ARRAY:
        // return base type align.
        return calc_alignof(ty->ptr_to);
    case TY_PTR:
        return 8;
    case TY_VOID:
        return 1;
    case TY_CHAR:
        return 1;
    case TY_SHORT:
        return 2;
    case TY_INT:
        return 4;
    case TY_LONG:
        return 8;
    case TY_LLONG:
        return 8;
    case TY_STRUCT:
        return ty->align;
    case TY_UNION:
        return ty->align;
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
        usual_arith_conv(&n->lhs, &n->rhs);
        n->ty = n->lhs->ty;
        return;
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
        t->align = calc_alignof(t);
        n->ty = t;
        return;
    case ND_CHAR:
        t->kind = TY_CHAR;
        t->size = calc_sizeof(t);
        t->align = calc_alignof(t);
        n->ty = t;
        return;
    case ND_ADDR:
        t->kind = TY_PTR;
        t->size = calc_sizeof(t);
        t->align = calc_alignof(t);
        t->ptr_to = n->lhs->ty;
        n->ty = t;
        return;
    case ND_DEREF:
        n->ty = n->lhs->ty->ptr_to;
        return;
    case ND_COMMA:
        n->ty = n->rhs->ty;
        return;
    case ND_STMT_EXPR:
        for (Node *block_n = n->block_head; block_n; block_n = block_n->next)
        {
            init_nodetype(block_n);
            if (!block_n->next)
                n->ty = block_n->ty;
        }
        return;
    case ND_BLOCK:
        for (Node *block_n = n->block_head; block_n; block_n = block_n->next)
            init_nodetype(block_n);
        return;
    default:
        return;
    }
}

bool is_typename(Token *tok, bool include_user_defined_type)
{

    // search basic type keywords.
    if (equal(tok, "void"))
        return true;
    if (equal(tok, "char"))
        return true;
    if (equal(tok, "short"))
        return true;
    if (equal(tok, "int"))
        return true;
    if (equal(tok, "long"))
        return true;

    // search typedef keywords.
    if (include_user_defined_type && equal(tok, "struct"))
        return true;
    if (include_user_defined_type && equal(tok, "union"))
        return true;

    if (include_user_defined_type && find_typedef(tok))
        return true;

    return false;
}

Type *copy_type(Type *org)
{
    Type *rep = calloc(1, sizeof(Type));

    rep->kind = org->kind;
    rep->size = org->size;
    rep->align = org->align;
    rep->ptr_to = org->ptr_to;
    rep->array_size = org->array_size;
    rep->ident_name_tok = org->ident_name_tok;
    rep->members = org->members;

    return rep;
}