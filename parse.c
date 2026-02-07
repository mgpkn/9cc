#define _XOPEN_SOURCE 700
#include "9cc.h"

typedef struct VarScope VarScope;
struct VarScope
{
  Ident *var;
  VarScope *next;
};

typedef struct TagScope TagScope;
struct TagScope
{
  char *name;
  Type *ty;
  TagScope *next;
};

typedef struct TypeDefScope TypeDefScope;
struct TypeDefScope
{
  char *name;
  Type *org_ty;
  TypeDefScope *next;
};

typedef struct Scope Scope;
struct Scope
{
  Scope *next;
  VarScope *vars;
  TagScope *tags;
  TypeDefScope *tdfs;
};

static Scope *cur_scope;
static Ident *locals;
static Ident *globals;

Ident *global(Token **rest, Token *tok);
Ident *declaration_global_var(Token **rest, Token *tok);
Ident *declaration_function(Token **rest, Token *tok);
Type *declarator(Token **rest, Token *tok, Type *base_ty);
Type *declarator_struct(Token **rest, Token *tok, Type *base_ty);
Type *declarator_prefix(Token **rest, Token *tok, Type *ty);
Type *declarator_suffix(Token **rest, Token *tok, Type *ty);
Node *statement(Token **rest, Token *tok);
Node *expr(Token **rest, Token *tok);
Node *type_def(Token **rest, Token *tok);
Node *declaration_local(Token **rest, Token *tok);
Node *declaration_param(Token **rest, Token *tok);
Type *declaration_suffix_array(Token **rest, Token *tok);
Node *assign(Token **rest, Token *tok);
Node *equality(Token **rest, Token *tok);
Node *relational(Token **rest, Token *tok);
Node *add(Token **rest, Token *tok);
Node *extra_add(Node *lhs, Node *rhs, Token *tok_dummy);
Node *extra_sub(Node *lhs, Node *rhs, Token *tok_dummy);
Node *mul(Token **rest, Token *tok);
Node *unary(Token **rest, Token *tok);
Node *postfix(Token **rest, Token *tok);
Node *primary(Token **rest, Token *tok);

extern char *code;
extern char *filename; // command parameter
int label_cnt;

// print error with printf() format.
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// print error with token location.
// the error message is like a bellow.
//  foo.c:10: x = y + + 5;
//                    ^ 式ではありません
void error_at(char *loc, char *msg, ...)
{

  va_list ap;
  va_start(ap, msg);

  // locが含まれている行の開始地点と終了地点を取得
  char *line = loc;
  while (code < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  // 見つかった行が全体の何行目なのかを調べる
  int line_num = 1;
  for (char *p = code; p < line; p++)
    if (*p == '\n')
      line_num++;

  // 見つかった行を、ファイル名と行番号と一緒に表示
  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // エラー箇所を"^"で指し示して、エラーメッセージを表示
  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

bool equal(Token *tok, char *op_str)
{
  return (strncmp(tok->pos, op_str, tok->len) == 0 && op_str[tok->len] == '\0');
}

bool equal_token(Token *tok1, Token *tok2)
{
  if (tok1->len == tok2->len)
    return (strncmp(tok1->pos, tok2->pos, tok1->len) == 0);
  return false;
}

// if token has expect value,read next token and return true.
bool consume(Token **rest, Token *tok, char *op)
{
  if (strlen(op) != tok->len || strncmp(tok->pos, op, strlen(op)) != 0)
    return false;
  *rest = tok->next;
  return true;
}

// if token has expect num value,read next token and return true.
bool consume_number(Token **rest, Token *tok, char *op)
{
  if (tok->kind != TK_NUM)
    return false;
  *rest = tok->next;
  return true;
}

// if token has expect value,read next token.
// else raise error.
void expect(Token **rest, Token *tok, char *op)
{
  if (tok->kind != TK_KEYWORD || strlen(op) != tok->len || strncmp(tok->pos, op, strlen(op)) != 0)
    error_at(tok->pos, "'%s'ではありません。", op);
  *rest = tok->next;
}

// if token has expect value,read next token and return value.
// else raise error.
int expect_number(Token **rest, Token *tok)
{
  if (tok->kind != TK_NUM)
    error_at(tok->pos, "数ではありません。");
  *rest = tok->next;
  return tok->val;
}

// enter-leave scope.
void *enter_scope()
{
  Scope *sc = calloc(1, sizeof(Scope));

  if (cur_scope)
    sc->next = cur_scope;
  cur_scope = sc;
}

void *leave_scope()
{
  cur_scope = cur_scope->next;
}

void add_varscope(Ident *var)
{
  VarScope *vs = calloc(1, sizeof(VarScope));
  vs->var = var;
  vs->next = cur_scope->vars;
  cur_scope->vars = vs;
}

void add_tagscope(Token *tok, Type *ty)
{
  TagScope *ts = calloc(1, sizeof(TagScope));
  ts->name = strndup(tok->pos, sizeof(char) * (tok->len));
  ts->ty = ty;
  ts->next = cur_scope->tags;
  cur_scope->tags = ts;
}

Ident *find_var(Token *tok)
{
  for (Scope *sc = cur_scope; sc; sc = sc->next)
  {
    for (VarScope *vsc = sc->vars; vsc; vsc = vsc->next)
    {
      if (equal(tok, vsc->var->name))
        return vsc->var;
    }
  }

  return NULL;
}

Type *find_tag(Token *tok)
{
  for (Scope *sc = cur_scope; sc; sc = sc->next)
  {
    for (TagScope *tsc = sc->tags; tsc; tsc = tsc->next)
    {
      if (equal(tok, tsc->name))
        return tsc->ty;
    }
  }
  return NULL;
}

Type *find_typedef(Token *tok)
{

  for (Scope *sc = cur_scope; sc; sc = sc->next)
  {
    for (TypeDefScope *tdf = sc->tdfs; tdf; tdf = tdf->next)
    {
      if (equal(tok, tdf->name))
        return tdf->org_ty;
    }
  }
  return NULL;
}

Ident *add_globals(Ident *tar)
{

  if (!globals)
  {
    globals = tar;
    return globals;
  }

  Ident *tmp = globals;
  while (tmp->next)
    tmp = tmp->next;
  tmp->next = tar;
  return tmp->next;
}

// base_type ::= ("void"|"char"|"short"|"int"|"long"|"struct"|"union","typedef")+
Type *base_type(Token **rest, Token *tok)
{

  enum
  {
    VOID = 1 << 0,
    CHAR = 1 << 2,
    SHORT = 1 << 4,
    INT = 1 << 6,
    LONG = 1 << 8,
    OTHER = 1 << 10
  };

  Type *ty = NULL;
  int ty_counter = 0;

  bool is_first = true;

  // accumulate type token
  if (consume(&tok, tok, "typedef"))
    is_first = false;

  while (is_typename(tok))
  {
    is_first = false;

    Type *def_ty = find_typedef(tok);
    ty = calloc(1, sizeof(Type));

    if (def_ty && ty_counter == 0)
    {
      ty = def_ty;
      ty_counter += OTHER;
      tok = tok->next;
      break;
    }

    if (equal(tok, "struct") && ty_counter == 0)
    {
      ty->kind = TY_STRUCT;
      ty_counter += OTHER;
      tok = tok->next;
      break;
    }

    if (equal(tok, "union") && ty_counter == 0)
    {
      ty->kind = TY_UNION;
      ty_counter += OTHER;
      tok = tok->next;
      break;
    }

    // decide base type
    if (equal(tok, "void"))
      ty_counter += VOID;

    if (equal(tok, "char"))
      ty_counter += CHAR;

    if (equal(tok, "short"))
      ty_counter += SHORT;

    if (equal(tok, "int"))
      ty_counter += INT;

    if (equal(tok, "long"))
      ty_counter += LONG;

    switch (ty_counter)
    {
    case VOID:
      ty->kind = TY_VOID;
      break;
    case CHAR:
      ty->kind = TY_CHAR;
      break;
    case SHORT:
    case SHORT + INT:
      ty->kind = TY_SHORT;
      break;
    case INT:
      ty->kind = TY_INT;
      break;
    case LONG:
    case LONG + INT:
      ty->kind = TY_LONG;
      break;
    case LONG + LONG:
    case LONG + LONG + INT:
      ty->kind = TY_LLONG;
      break;
    case OTHER: //"struct" || "union" || typedefs
      error_at(tok->pos, "unreachalble case.");
    default:
      error_at(tok->pos, "invalid type token.");
    }
    tok = tok->next;
  }

  if (is_first)
    error_at(tok->pos, "invalid type token.");

  *rest = tok;
  return ty;
}

static Ident *declaration_str(Token *tok)
{

  // literal string is treated like　global vars.

  static int str_id = 0;
  str_id++; // when this function called,incriment str_id;

  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->ptr_to = ty_char;
  ty->array_size = tok->len;
  ty->size = calc_sizeof(ty);
  ty->align = calc_alignof(ty);

  Ident *idt = calloc(1, sizeof(Ident));
  idt->is_global = true;
  idt->is_function = false;

  idt->name = calloc(1, sizeof(char) * 50);
  idt->name_len = sprintf(idt->name, ".LC%d", str_id);
  idt->str = tok->str;
  idt->ty = ty;

  add_globals(idt);

  return idt;
}

// create non num node
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *n = calloc(1, sizeof(Node));
  n->kind = kind;
  n->lhs = lhs;
  n->rhs = rhs;
  return n;
}

Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  init_nodetype(node);
  return node;
}

Node *new_node_char(Token **rest, Token *tok, int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_CHAR;
  node->val = val;

  tok = tok->next;
  *rest = tok;
  return node;
}

Node *new_node_str(Token **rest, Token *tok)
{
  Ident *idt_str = declaration_str(tok);

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_STR;
  node->ty = idt_str->ty;
  node->ident_name = idt_str->name;
  init_nodetype(node);

  tok = tok->next;
  *rest = tok;
  return node;
}

// create variable node
Node *new_node_var(Token **rest, Token *tok)
{
  Node *n = NULL;
  if (tok)
  {

    // search the variable in globals or locals
    Ident *var = find_var(tok);
    if (!var)
      error_at(tok->pos, "the variable is not declared.");

    n = calloc(1, sizeof(Node));
    n->ident_name = var->name;
    n->ty = var->ty;

    if (var->is_global)
      n->kind = ND_GVAR;
    else
    {
      n->kind = ND_LVAR;
      n->var = var;
    }
  }
  *rest = tok->next;
  return n;
}

// create lvar node
Node *new_node_declare_lvar(Type *ty)
{

  // search indent if it's already declared.
  if (find_var(ty->ident_name_tok))
    error_at(ty->ident_name_tok->pos, "the variable is already declared.");

  // if first deaclared,create locals indet.
  Ident *idt = calloc(1, sizeof(Ident));
  idt->is_global = false;
  idt->name_len = ty->ident_name_tok->len;
  idt->name = strndup(ty->ident_name_tok->pos, sizeof(char) * idt->name_len);
  idt->ty = ty;

  idt->next = locals;
  locals = idt;

  add_varscope(idt);

  return new_node_var(&(ty->ident_name_tok), ty->ident_name_tok);
}

Node *new_node_member(Token **rest, Token *tok, Node *lhs)
{
  init_nodetype(lhs);
  if (!(lhs->ty->kind == TY_STRUCT || lhs->ty->kind == TY_UNION))
    error("it's not struct type");

  if (tok->kind != TK_IDENT)
    error("token must be member ident");

  Member *mem_target;
  for (Member *mem_tmp = lhs->ty->members; mem_tmp; mem_tmp = mem_tmp->next)
  {
    if (equal_token(tok, mem_tmp->name))
    {
      mem_target = mem_tmp;
      break;
    }
  }
  if (!mem_target)
    error("not find such a member name");

  Node *n = calloc(1, sizeof(Node));
  n->kind = ND_MEMBER;
  n->lhs = lhs;
  n->ty = mem_target->ty;
  n->mem = mem_target;

  tok = tok->next;

  *rest = tok;
  return n;
}

// create function node
Node *new_node_function(Token **rest, Token *tok)
{
  Node *node;
  int i = 0;

  node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->ident_name = strndup(tok->pos, sizeof(char) * (tok->len));
  tok = tok->next;
  expect(&tok, tok, "(");
  if (consume(&tok, tok, ")"))
  {
    *rest = tok;
    return node;
  }

  while (true)
  {
    if (i >= FUNC_ARG_NUM)
      error("function aguments limit is %d", FUNC_ARG_NUM);

    node->func_arg[i] = assign(&tok, tok);
    if (!consume(&tok, tok, ","))
      break;
    i++;
  }
  expect(&tok, tok, ")");
  *rest = tok;
  return node;
}

bool at_eof(Token *tok)
{
  return tok->kind == TK_EOF;
}

/*
parse ::= global*
*/
Ident *parse(Token *tok)
{

  label_cnt = 0;
  globals = NULL;
  Ident *tmp_global;
  enter_scope(); // make global scope

  while (!at_eof(tok))
  {
    locals = NULL;
    tmp_global = global(&tok, tok);
    if (tmp_global)
    {
      tmp_global->locals = locals;
      add_globals(tmp_global);
    }
  }
  return globals;
}

/*
global ::=
  (type_def|declaration_global_var|declaration_function)
*/
Ident *global(Token **rest, Token *tok)
{

  Ident *glb = NULL;

  // typedef.
  if (equal(tok, "typedef"))
  {
    type_def(&tok, tok);
    consume(&tok,tok,";");
    *rest = tok;
    return NULL;
  }

  // attempt to create global var
  glb = declaration_global_var(&tok, tok);

  // if faild to create global var,next create function.
  if (!glb)
    glb = declaration_function(&tok, tok);

  add_varscope(glb);

  *rest = tok;
  return glb;
}

/*
declaration_global_var ::=
    base_type declarator("," declarator)* ";"
*/
Ident *declaration_global_var(Token **rest, Token *tok)
{

  Token *tmp_tok;
  Type *base_ty = base_type(&tmp_tok, tok);
  Type *ty = declarator(&tmp_tok, tmp_tok, base_ty);

  if (find_var(ty->ident_name_tok))
    error_at(ty->ident_name_tok->pos, "the variable is already declared.");

  // if it's function syntax.return NULL;
  if (consume(&tmp_tok, tmp_tok, "("))
    return NULL;

  // since it was global variable token(=not function),create it.
  tok = tmp_tok; // apply tmp_tok to tok.

  Ident *idt = calloc(1, sizeof(Ident));
  idt->is_global = true;
  idt->is_function = false;
  idt->name_len = ty->ident_name_tok->len;
  idt->name = strndup(ty->ident_name_tok->pos, sizeof(char) * idt->name_len);
  idt->ty = ty;

  // todo declaration multi variables

  consume(&tok, tok, ";");
  *rest = tok;
  return idt;
}

/*
declaration_function ::=
    base_type declarator "(" (declaration_param("," declaration_param)*)? ")" "{" statment* "}"
*/
Ident *declaration_function(Token **rest, Token *tok)
{

  Type *base_ty = base_type(&tok, tok);
  Type *ty = declarator(&tok, tok, base_ty);

  if (find_var(ty->ident_name_tok))
    error_at(ty->ident_name_tok->pos, "the function is already declared.");

  // if array_type is  in declarator raise error.
  for (Type *tmp_ty = ty; tmp_ty; tmp_ty = tmp_ty->ptr_to)
  {
    if (tmp_ty->kind == TY_ARRAY)
      error("function ident can't have array type;");
  }

  Ident *idt = calloc(1, sizeof(Ident));
  idt->is_global = true;
  idt->is_function = true;
  idt->name_len = ty->ident_name_tok->len;
  idt->name = strndup(ty->ident_name_tok->pos, sizeof(char) * idt->name_len);
  idt->ty = ty;

  enter_scope();

  expect(&tok, tok, "(");

  Node head_arg, *cur_arg;
  head_arg.next = NULL;
  cur_arg = &head_arg;

  Node head_body, *cur_body;
  head_body.next = NULL;
  cur_body = &head_body;

  // agument
  while (true)
  {
    if (equal(tok, ")"))
      break;

    cur_arg->next = declaration_param(&tok, tok);
    cur_arg = cur_arg->next;
    init_nodetype(cur_arg);

    if (!consume(&tok, tok, ","))
      break;
  }
  consume(&tok, tok, ")");
  idt->arg = head_arg.next;

  // body
  expect(&tok, tok, "{");
  while (true)
  {
    if (equal(tok, "}"))
      break;

    cur_body->next = statement(&tok, tok);
    cur_body = cur_body->next;
    init_nodetype(cur_body);
  }
  consume(&tok, tok, "}");
  idt->body = head_body.next;

  leave_scope();
  *rest = tok;
  return idt;
}

// declarator ::= declarator_prefix ("(" declarator ")"|ident) declarator_suffix
Type *declarator(Token **rest, Token *tok, Type *ty)
{

  Token *tmp_ident_name_tok;

  // get prefix type
  ty = declarator_prefix(&tok, tok, ty);

  if (consume(&tok, tok, "("))
  {
    Token *nest_start_tok = tok;

    // skip tokens in nest and get suffix token
    Type *dummy_ty = calloc(1, sizeof(Type));
    declarator(&tok, nest_start_tok, dummy_ty);
    expect(&tok, tok, ")");

    ty = declarator_suffix(&tok, tok, ty);
    ty = declarator(&nest_start_tok, nest_start_tok, ty);

    *rest = tok;
    return ty;
  }

  // get ident name
  if (tok->kind != TK_IDENT)
    error_at(tok->pos, "it isn't ident.");
  tmp_ident_name_tok = tok;
  tok = tok->next;

  // get suffix type
  ty = declarator_suffix(&tok, tok, ty);

  for (Type *tmp_ty = ty; tmp_ty; tmp_ty = tmp_ty->ptr_to)
  {
    tmp_ty->size = calc_sizeof(tmp_ty);
    tmp_ty->align = calc_alignof(tmp_ty);
  }

  // set indent name token;
  ty->ident_name_tok = tmp_ident_name_tok;

  *rest = tok;
  return ty;
}

// declarator_struct ::=  ident? "{" ( base_type declarator ";")* "}"
Type *declarator_struct(Token **rest, Token *tok, Type *parent_ty)
{

  // not struct type.
  if (parent_ty->kind != TY_STRUCT)
    return parent_ty;

  // typedef pettern. (already has defined members.)
  if (parent_ty->members)
    return parent_ty;

  // if not set tag name,create new scope.
  Token *tag = NULL;
  if (tok->kind == TK_IDENT)
  {
    tag = tok;
    tok = tok->next;
  }

  // if not declare defs,find tag
  Member head_mem, *tmp_mem;
  int cur_offset = 0;
  head_mem.next = NULL;
  tmp_mem = &head_mem;

  if (tag)
  {

    if (find_tag(tag) && !equal(tok, "{"))
    {
      *rest = tok;
      return find_tag(tag);
    }

    if (find_tag(tag) && equal(tok, "{"))
      error_at(tag->pos, "the struct is already declared.");

    if (!find_tag(tag) && !equal(tok, "{"))
      error_at(tag->pos, "undefined struct name.");
  }

  // define struct member
  expect(&tok, tok, "{");
  while (true)
  {
    if (equal(tok, "}"))
      break;

    tmp_mem->next = calloc(1, sizeof(Member));

    Type *mem_ty = base_type(&tok, tok);
    if (!mem_ty)
      error_at(tok->pos, "undefined data type.");
    mem_ty = declarator(&tok, tok, mem_ty);

    tmp_mem->next->ty = mem_ty;
    tmp_mem->next->name = mem_ty->ident_name_tok;
    tmp_mem->next->offset = align_to(cur_offset, mem_ty->align);
    cur_offset = tmp_mem->next->offset + mem_ty->size;

    expect(&tok, tok, ";");
    tmp_mem = tmp_mem->next;
  }
  consume(&tok, tok, "}");

  parent_ty->members = head_mem.next;

  // set struct align and size.
  ////align -> largetst align in struct member.
  int largest_align = 1;
  for (Member *tmp_mem = parent_ty->members; tmp_mem; tmp_mem = tmp_mem->next)
  {
    if (tmp_mem->ty->align >= largest_align)
      largest_align = tmp_mem->ty->align;
  }
  parent_ty->align = largest_align;

  ////size -> align to total offset to members
  parent_ty->size = align_to(cur_offset, parent_ty->align);

  if (tag)
    add_tagscope(tag, parent_ty);

  *rest = tok;
  return parent_ty;
}

// declarator_union ::=  ident? "{" ( base_type declarator ";")* "}"
Type *declarator_union(Token **rest, Token *tok, Type *parent_ty)
{

  // not union type.
  if (parent_ty->kind != TY_UNION)
    return parent_ty;

  // typedef pettern. (already has defined members.)
  if (parent_ty->members)
    return parent_ty;

  // if not called tag name,create scope.
  Token *tag = NULL;
  if (tok->kind == TK_IDENT)
  {
    tag = tok;
    tok = tok->next;
  }

  // if not declare defs,find tag
  Member head_mem, *tmp_mem;
  head_mem.next = NULL;
  tmp_mem = &head_mem;

  if (tag)
  {

    if (find_tag(tag) && !equal(tok, "{"))
    {
      *rest = tok;
      return find_tag(tag);
    }

    if (find_tag(tag) && equal(tok, "{"))
      error_at(tag->pos, "the union is already declared.");

    if (!find_tag(tag) && !equal(tok, "{"))
      error_at(tag->pos, "undefined uninon name.");
  }

  // define union member
  expect(&tok, tok, "{");
  while (true)
  {
    if (equal(tok, "}"))
      break;

    tmp_mem->next = calloc(1, sizeof(Member));

    Type *mem_ty = base_type(&tok, tok);
    if (!mem_ty)
      error_at(tok->pos, "undefined data type.");
    mem_ty = declarator(&tok, tok, mem_ty);

    tmp_mem->next->ty = mem_ty;
    tmp_mem->next->name = mem_ty->ident_name_tok;
    tmp_mem->next->offset = 0;

    expect(&tok, tok, ";");
    tmp_mem = tmp_mem->next;
  }
  consume(&tok, tok, "}");

  parent_ty->members = head_mem.next;

  // set union align and size.
  ////align -> largetst align in union member.
  int largest_align = 1;
  for (Member *tmp_mem = parent_ty->members; tmp_mem; tmp_mem = tmp_mem->next)
  {
    if (tmp_mem->ty->align >= largest_align)
      largest_align = tmp_mem->ty->align;
  }
  parent_ty->align = largest_align;

  ////size -> align of union
  parent_ty->size = parent_ty->align;

  if (tag)
    add_tagscope(tag, parent_ty);

  *rest = tok;
  return parent_ty;
}

// declarator_prefix := ("*" declarator_prefix)?
Type *declarator_prefix(Token **rest, Token *tok, Type *ty)
{

  Type *tmp_ty;

  // create dereferencer
  if (consume(&tok, tok, "*"))
  {
    tmp_ty = calloc(1, sizeof(Type));
    tmp_ty->kind = TY_PTR;
    tmp_ty->ptr_to = ty;
    ty = declarator_prefix(&tok, tok, tmp_ty);
  }
  *rest = tok;
  return ty;
}

// declarator_suffix ::= ("[" num "]" declarator_suffix)?
Type *declarator_suffix(Token **rest, Token *tok, Type *ty)
{

  Type *tmp_ty = NULL;

  // create array type
  if (consume(&tok, tok, "["))
  {
    tmp_ty = calloc(1, sizeof(Type));
    tmp_ty->kind = TY_ARRAY;
    tmp_ty->array_size = expect_number(&tok, tok);
    expect(&tok, tok, "]");
    tmp_ty->ptr_to = ty;
    ty = declarator_suffix(&tok, tok, tmp_ty);
  }

  *rest = tok;
  return ty;
}

/*
statement ::=
    |";"
    |"{" statement? "}"
    |"return " expr ";"
    |"if" "(" expr ")" statement "else" statement
    |"while" "(" expr ")"  statement
    |"for" "(" (declaration_local|expr)? ";" expr? ";" expr? ";" ")"  statement
    |type_def
    |declaration_local
    |expr ";"
*/
Node *statement(Token **rest, Token *tok)
{
  Node *n;
  Node *n_block_cur = calloc(1, sizeof(Node));

  if (tok->kind == TK_KEYWORD)
  {

    if (consume(&tok, tok, ";"))
    {
      *rest = tok;
      return new_node(ND_BLOCK, NULL, NULL);
    }

    if (consume(&tok, tok, "{"))
    {
      n = new_node(ND_BLOCK, NULL, NULL);
      enter_scope();
      while (!consume(&tok, tok, "}"))
      {

        n_block_cur->next = statement(&tok, tok);
        if (!n->block_head)
          n->block_head = n_block_cur->next;
        n_block_cur = n_block_cur->next;
      }
      leave_scope();
      *rest = tok;
      return n;
    }
    else if (equal(tok, "return"))
    {
      tok = tok->next;
      n = new_node(ND_RETURN, expr(&tok, tok), NULL);
    }
    else if (equal(tok, "if"))
    {

      tok = tok->next;
      n = new_node(ND_IF, NULL, NULL);
      n->label_num = label_cnt;
      label_cnt++;
      if (consume(&tok, tok, "("))
      {
        n->cond = expr(&tok, tok);
        expect(&tok, tok, ")");
      }
      n->then = statement(&tok, tok);

      // else
      if (equal(tok, "else"))
      {
        tok = tok->next;
        n->els = statement(&tok, tok);
      }

      *rest = tok;
      return n;
    }
    else if (equal(tok, "while"))
    {
      tok = tok->next;

      n = new_node(ND_WHILE, NULL, NULL);
      n->label_num = label_cnt;
      label_cnt++;
      if (consume(&tok, tok, "("))
      {
        n->cond = expr(&tok, tok);
        expect(&tok, tok, ")");
      }
      n->then = statement(&tok, tok);

      *rest = tok;
      return n;
    }
    else if (equal(tok, "for"))
    {

      tok = tok->next;

      n = new_node(ND_FOR, NULL, NULL);
      n->label_num = label_cnt;
      label_cnt++;
      if (consume(&tok, tok, "("))
      {
        // init
        if (!equal(tok, ";"))
          n->init = expr(&tok, tok);
        expect(&tok, tok, ";");
        // condition
        if (!equal(tok, ";"))
          n->cond = expr(&tok, tok);
        expect(&tok, tok, ";");
        // incriment
        if (!equal(tok, ")"))
          n->inc = expr(&tok, tok);
        expect(&tok, tok, ")");
      }
      n->then = statement(&tok, tok);
      *rest = tok;
      return n;
    }
    else if (equal(tok, "typedef"))
      n = type_def(&tok, tok);
    else
    {
      n = expr(&tok, tok);
    }
    expect(&tok, tok, ";");
  }
  else
  {
    if (is_typename(tok))
      n = declaration_local(&tok, tok);
    else
    {
      n = expr(&tok, tok);
      expect(&tok, tok, ";");
    }
  }
  *rest = tok;
  return n;
}

// expr ::= assgin ("," expr )?
Node *expr(Token **rest, Token *tok)
{
  Node *n = assign(&tok, tok);

  if (consume(&tok, tok, ","))
  {
    n = new_node(ND_COMMA, n, expr(&tok, tok));
  }

  *rest = tok;
  return n;
}

/*
type_def ::=
    base_type?  (declarator_struct|declarator_union)? (declarator ("," declarator )*)?
*/
Node *type_def(Token **rest, Token *tok)
{

  Type *base_ty = base_type(&tok, tok);
  if (!base_ty)
  {
    // default int
    base_ty = calloc(1, sizeof(Type));
    base_ty->kind = TY_INT;
  }

  // perse strcut or union type
  base_ty = declarator_struct(&tok, tok, base_ty);
  base_ty = declarator_union(&tok, tok, base_ty);

  bool is_first = true;
  while (true)
  {

    if (equal(tok, ";"))
      break;

    if (!is_first)
      expect(&tok, tok, ",");
    is_first = false;

    Type *ty = declarator(&tok, tok, base_ty);
    TypeDefScope *tdf = calloc(1, sizeof(TypeDefScope));
    tdf->name = strndup(ty->ident_name_tok->pos, ty->ident_name_tok->len);
    tdf->org_ty = ty;
    tdf->next = cur_scope->tdfs;
    cur_scope->tdfs = tdf;
  }

  Node *n = new_node(ND_BLOCK, NULL, NULL);
  *rest = tok;
  return n;
}

/*
declaration_local ::=
    base_type (declarator_struct|declarator_union)? declarator ("=" assign )? ("," declarator("=" assign )? )* ";"
*/
Node *declaration_local(Token **rest, Token *tok)
{

  Type *base_ty = base_type(&tok, tok);
  if (!base_ty)
    error_at(tok->pos, "undefined data type.");

  // perse strcut or union type
  base_ty = declarator_struct(&tok, tok, base_ty);
  base_ty = declarator_union(&tok, tok, base_ty);

  Node *n = new_node(ND_BLOCK, NULL, NULL), block_head, *cur_n;
  block_head.next = NULL;
  cur_n = &block_head;

  while (!equal(tok, ";"))
  {

    if (block_head.next)
      expect(&tok, tok, ",");

    Type *ty = declarator(&tok, tok, base_ty);

    Node *tmp_n = new_node_declare_lvar(ty);
    if (consume(&tok, tok, "="))
    {
      if (ty->kind == TY_VOID)
        error("can't assign value to void variable.");
      tmp_n = new_node(ND_ASSIGN, tmp_n, assign(&tok, tok));
    }

    cur_n->next = tmp_n;
    cur_n = cur_n->next;
  }

  consume(&tok, tok, ";");

  n->block_head = block_head.next;
  *rest = tok;
  return n;
}

/*
declaration_param ::= base_type (declarator_struct|declarator_union)? declarator
*/
Node *declaration_param(Token **rest, Token *tok)
{

  Type *base_ty = base_type(&tok, tok);
  if (!base_ty)
    error_at(tok->pos, "undefined data type");

  // perse strcut or union type
  base_ty = declarator_struct(&tok, tok, base_ty);
  base_ty = declarator_union(&tok, tok, base_ty);

  Node *n = new_node(ND_BLOCK, NULL, NULL);
  Type *ty = declarator(&tok, tok, base_ty);
  n = new_node_declare_lvar(ty);

  *rest = tok;
  return n;
}

// assign ::= equality ("=" assign )?
Node *assign(Token **rest, Token *tok)
{
  Node *n = equality(&tok, tok);
  if (consume(&tok, tok, "="))
    n = new_node(ND_ASSIGN, n, assign(&tok, tok));

  *rest = tok;
  return n;
}

// equality ::= relational ("==" relational |"!=" relational )*
Node *equality(Token **rest, Token *tok)
{
  Node *n = relational(&tok, tok);

  for (;;)
  {
    if (consume(&tok, tok, "=="))
      n = new_node(ND_EQ, n, relational(&tok, tok));
    else if (consume(&tok, tok, "!="))
      n = new_node(ND_NOTEQ, n, relational(&tok, tok));
    else
    {
      *rest = tok;
      return n;
    }
  }
}

// relational ::= add (">" add | "<" add | ">=" add | "<=" add)*
Node *relational(Token **rest, Token *tok)
{
  Node *n = add(&tok, tok);

  for (;;)
  {
    //>および>=不等号が逆の場合は左右のノード自体を逆に生成する
    if (consume(&tok, tok, "<="))
      n = new_node(ND_LLESSEQ, n, add(&tok, tok));
    else if (consume(&tok, tok, ">="))
      n = new_node(ND_LLESSEQ, add(&tok, tok), n);
    else if (consume(&tok, tok, "<"))
      n = new_node(ND_LLESS, n, add(&tok, tok));
    else if (consume(&tok, tok, ">"))
      n = new_node(ND_LLESS, add(&tok, tok), n);
    else
    {
      *rest = tok;
      return n;
    }
  }
}

// add ::= mul ("+" mul | "-" mul)*
Node *add(Token **rest, Token *tok)
{

  Token *tok_dummy = tok; // dummy token to send extra~ functions.
  Node *n = mul(&tok, tok);

  for (;;)
  {
    if (consume(&tok, tok, "+"))
    {
      n = extra_add(n, mul(&tok, tok), tok_dummy);
      continue;
    }

    if (consume(&tok, tok, "-"))
    {
      n = extra_sub(n, mul(&tok, tok), tok_dummy);
      continue;
    }

    *rest = tok;
    return n;
  }
}

// 数値とポインタの組み合わせによってノードの加算方法をよしなに変える
Node *extra_add(Node *lhs, Node *rhs, Token *tok_dummy)
{

  init_nodetype(lhs);
  init_nodetype(rhs);

  // num + numの場合は普通の演算
  if (!is_ptr_node(lhs) && !is_ptr_node(rhs))
    return new_node(ND_ADD, lhs, rhs);

  // pointer + num (=num + pointer) then calc offset
  // if reverse order,swap node.
  if (!is_ptr_node(lhs) && is_ptr_node(rhs))
  {
    Node *swp;
    swp = lhs;
    lhs = rhs;
    rhs = swp;
  }

  if (is_ptr_node(lhs) && !is_ptr_node(rhs))
  {
    Node *n = new_node(ND_MUL, rhs, new_node_num(lhs->ty->ptr_to->size));
    return new_node(ND_ADD, lhs, n);
  }

  error("invalid type addtion.");
  return NULL;
}

// 数値とポインタの組み合わせによってノードの減算方法をよしなに変える
Node *extra_sub(Node *lhs, Node *rhs, Token *tok_dummy)
{

  init_nodetype(lhs);
  init_nodetype(rhs);

  // num - numの場合は普通の演算
  if (!is_ptr_node(lhs) && !is_ptr_node(rhs))
    return new_node(ND_SUB, lhs, rhs);

  // pointer - num (=num + pointer)の場合はオフセットの計算
  if (is_ptr_node(lhs) && !is_ptr_node(rhs))
    return new_node(ND_SUB, lhs, new_node(ND_MUL, rhs, new_node_num(lhs->ty->ptr_to->size)));

  // pointer - pointerはアドレスの差
  if (is_ptr_node(lhs) && is_ptr_node(rhs))
    return new_node(ND_DIV, new_node(ND_SUB, lhs, rhs), new_node_num(lhs->ty->ptr_to->size));

  error("invalid types subtraction");
  return NULL;
}

// mul ::= unary ("*" unary|"/" unary|"%" unary)*
Node *mul(Token **rest, Token *tok)
{
  Node *n = unary(&tok, tok);

  for (;;)
  {
    if (consume(&tok, tok, "*"))
      n = new_node(ND_MUL, n, unary(&tok, tok));
    else if (consume(&tok, tok, "/"))
      n = new_node(ND_DIV, n, unary(&tok, tok));
    else if (consume(&tok, tok, "%"))
      n = new_node(ND_MOD, n, unary(&tok, tok));
    else
    {
      *rest = tok;
      return n;
    }
  }
}

/*
数値などの正負の項、ポインタデリファレンサ
unary ::= ("+"|"-"|"&"|"*"|"sizeof") unary
    |postfix
*/
Node *unary(Token **rest, Token *tok)
{
  Node *n;

  if (consume(&tok, tok, "+"))
    n = unary(&tok, tok);
  else if (consume(&tok, tok, "-"))
    n = new_node(ND_SUB, new_node_num(0), unary(&tok, tok));
  else if (consume(&tok, tok, "&"))
    n = new_node(ND_ADDR, unary(&tok, tok), NULL);
  else if (consume(&tok, tok, "*"))
    n = new_node(ND_DEREF, unary(&tok, tok), NULL);
  else if (consume(&tok, tok, "sizeof"))
  {
    n = unary(&tok, tok);
    init_nodetype(n);
    n = new_node_num(n->ty->size);
  }
  else
    n = postfix(&tok, tok);

  *rest = tok;
  return n;
}

/*
postfix ::= primary ("[" & expr & "]" | "." ident | "->" ident )*
*/
Node *postfix(Token **rest, Token *tok)
{
  Node *n, *idx;
  n = primary(&tok, tok);

  while (true)
  {

    // array parse
    // a[b] = *(a+(b*ty_size))
    if (consume(&tok, tok, "["))
    {
      idx = extra_add(n, expr(&tok, tok), NULL);
      expect(&tok, tok, "]");
      n = new_node(ND_DEREF, idx, NULL);
      continue;
    }

    // struct or union member
    if (consume(&tok, tok, "."))
    {
      n = new_node_member(&tok, tok, n);
      continue;
    }

    // struct or union member(arrow operator)
    // "a -> b" == "(*a).b"
    if (consume(&tok, tok, "->"))
    {
      n = new_node(ND_DEREF, n, NULL);
      n = new_node_member(&tok, tok, n);
      continue;
    }

    break;
  }

  *rest = tok;
  return n;
}

/*
primary ::= "(" "{" statement+ "}" ")"
    |"(" expr ")"
    |ident ("(" assign? ("," assign)* ")")?
    |num|char|str
*/
Node *primary(Token **rest, Token *tok)
{
  Node *n;
  if (consume(&tok, tok, "("))
  {
    // block statement
    if (consume(&tok, tok, "{"))
    {
      n = new_node(ND_BLOCK, NULL, NULL);
      enter_scope();
      Node *n_block_cur = calloc(1, sizeof(Node));
      while (!consume(&tok, tok, "}"))
      {
        n_block_cur->next = statement(&tok, tok);
        if (!(n->block_head))
          n->block_head = n_block_cur->next;
        n_block_cur = n_block_cur->next;
      }
      leave_scope();
    }
    else
    {
      n = expr(&tok, tok);
    }
    expect(&tok, tok, ")");
    *rest = tok;
    return n;
  }

  if (tok->kind == TK_IDENT)
  {
    // todo:()の有無ではなくident listから今後は変数判定するように変更。
    if (equal(tok->next, "("))
      n = new_node_function(&tok, tok);
    else
      n = new_node_var(&tok, tok);
  }

  if (tok->kind == TK_STR)
    n = new_node_str(&tok, tok);

  if (tok->kind == TK_CHAR)
    n = new_node_char(&tok, tok, (int)tok->str[0]);

  if (tok->kind == TK_NUM)
    n = new_node_num(expect_number(&tok, tok));

  *rest = tok;
  return n;
}
