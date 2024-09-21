#define _XOPEN_SOURCE 700
#include "9cc.h"

typedef struct VarScope VarScope;
struct VarScope
{
  VarScope *next;
  // char *name;
  Ident *var;
};

typedef struct Scope Scope;
struct Scope
{
  Scope *next;
  VarScope *vars;
};

static Scope *cur_scope;
static Ident *locals;
static Ident *globals;

Ident *global(Token **rest, Token *tok);
Ident *declaration_global_var(Token **rest, Token *tok, Type *base_ty);
Ident *declaration_function(Token **rest, Token *tok, Type *base_ty);
Type *declarator(Token **rest, Token *tok, Type *base_ty);
Type *declarator_prefix(Token **rest, Token *tok);
Type *declarator_suffix(Token **rest, Token *tok);
Node *statement(Token **rest, Token *tok);
Node *expr(Token **rest, Token *tok);
Node *declaration_local(Token **rest, Token *tok);
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

bool equal(char *tar_str, char *op_str, int tar_len)
{
  return (strncmp(tar_str, op_str, tar_len) == 0 && op_str[tar_len] == '\0');
}

bool equal_token(Token *tok, char *op)
{
  return equal(tok->pos, op, tok->len);
}

bool is_base_type_token(Token *tok)
{
  if (equal_token(tok, "int"))
    return true;
  if (equal_token(tok, "char"))
    return true;

  return false;
}

Type *base_type(Token **rest, Token *tok)
{

  Type *ty = calloc(1, sizeof(Type));
  ty->kind = -1;

  if (equal_token(tok, "int"))
    ty->kind = TY_INT;
  if (equal_token(tok, "char"))
    ty->kind = TY_CHAR;

  if (ty->kind == -1)
    error_at(tok->pos, "undefined type name;");

  *rest = tok->next;
  return ty;
}

// if token has expect value,read next token and return true.
bool consume(Token **rest, Token *tok, char *op)
{
  if (tok->kind != TK_KEYWORD || strlen(op) != tok->len || strncmp(tok->pos, op, strlen(op)) != 0)
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

void add_varscope(Ident *var){

  VarScope *vs = calloc(1, sizeof(VarScope));
  vs->var = var;
  vs->next = cur_scope->vars;
  cur_scope -> vars = vs;

}

// check if the variable is already declared
Ident *find_var(Token *tok)
{
  Token *target, *dummy;
  Type *decl = declarator(&dummy, tok, calloc(1, sizeof(Type)));
  target = decl->ident_name_tok;

  for(Scope *sc = cur_scope;sc;sc = sc->next){
    for(VarScope *vsc = sc->vars;vsc;vsc = vsc->next){
      if (equal(target->pos, vsc->var->name, target->len))
        return vsc->var;
    }
  }

  /*

  // search locals
  for (Ident *v = locals; v; v = v->next)
  {
    if (equal(target->pos, v->name, target->len))
      return v;
  }

  // search globals
  for (Ident *v = globals; v; v = v->next)
  {
    if (equal(target->pos, v->name, target->len))
      return v;
  }

  */

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

static Ident *declaration_str(Token *tok)
{

  // literal string is treated like　global vars.

  static int str_id = 0;
  str_id++; // when this function called,incriment str_id;

  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->ptr_to = ty_char;
  ty->array_size = tok->len;

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
  init_nodetype(node);

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

// create lvar node
Node *new_node_declare_lvar(Type *ty)
{
  Node *node = calloc(1, sizeof(Node));

  // find indent if it's already declared.
  if (find_var(ty->ident_name_tok))
    error_at(ty->ident_name_tok->pos, "the variable is already declared.");

  if (ty->ident_name_tok)
  {
    Ident *idt = calloc(1, sizeof(Ident));
    idt->is_global = false;
    idt->name_len = ty->ident_name_tok->len;
    idt->name = strndup(ty->ident_name_tok->pos, sizeof(char) * idt->name_len);
    idt->ty = ty;

    if (locals)
    {
      // after 2nd locals declare.
      for (Ident *v = locals;; v = v->next)
      {
        if (!v->next)
        {
          idt->offset = v->offset + calc_sizeof(idt->ty);
          v->next = idt;
          break;
        }
      }
    }
    else
    {
      // 1st locals declare.
      idt->offset = calc_sizeof(idt->ty);
      locals = idt;
    }
    add_varscope(idt);
    node->kind = ND_LVAR;
    node->ident_name = idt->name;
    node->offset = idt->offset;
    node->ty = idt->ty;
  }
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
    {
      n->kind = ND_GVAR;
    }
    else
    {
      n->kind = ND_LVAR;
      n->offset = var->offset;
    }
  }
  *rest = tok->next;
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
    {
      error("function aguments limit is %d", FUNC_ARG_NUM);
    }
    node->func_arg[i] = expr(&tok, tok);
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
  enter_scope();

  while (!at_eof(tok))
  {

    locals = NULL;
    tmp_global = global(&tok, tok);
    tmp_global->locals = locals;
    add_globals(tmp_global);
  }
  return globals;
}

/*
global ::= base_type (declaration_global_var|declaration_function)
*/
Ident *global(Token **rest, Token *tok)
{

  Type *base_ty = base_type(&tok, tok);

  // first,try to create variable ident from current token.
  // if coundn't,next try to function ident.

  // create global variable ident.
  Ident *glb = declaration_global_var(&tok, tok, base_ty);

  // create function ident.
  if (!glb)
    glb = declaration_function(&tok, tok, base_ty);
  
  add_varscope(glb);

  *rest = tok;
  return glb;
}

// declaration_global_var ::= declarator("," declarator)* ";"
Ident *declaration_global_var(Token **rest, Token *tok, Type *base_ty)
{

  Token *tmp_tok;
  Type *ty = declarator(&tmp_tok, tok, base_ty);

  if (find_var(ty->ident_name_tok))
    error_at(ty->ident_name_tok->pos, "the variable is already declared.");


  // if it's function syntax.return NULL;
  if (consume(&tmp_tok, tmp_tok, "("))
    return NULL;

  // since it was not function.create variable ident.
  // apply tmp_tok to tok.
  tok = tmp_tok;

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

// declaration_function ::= declarator "(" (declaration_local("," declaration_local)?)? ")" "{" statment* "}"
Ident *declaration_function(Token **rest, Token *tok, Type *base_ty)
{

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

  Node *head_arg = NULL, *cur_arg = NULL, *head_body = NULL, *cur_body = NULL;

  // agument
  while (true)
  {
    if (equal_token(tok, ")"))
      break;
    if (head_arg)
    {
      cur_arg->next = declaration_local(&tok, tok);
      cur_arg = cur_arg->next;
    }
    else
    {
      head_arg = declaration_local(&tok, tok);
      cur_arg = head_arg;
    }
    init_nodetype(cur_arg);
    if (!consume(&tok, tok, ","))
      break;
  }
  consume(&tok, tok, ")");
  idt->arg = head_arg;

  // body
  expect(&tok, tok, "{");
  while (true)
  {
    if (equal_token(tok, "}"))
      break;
    if (head_body)
    {
      cur_body->next = statement(&tok, tok);
      cur_body = cur_body->next;
    }
    else
    {
      head_body = statement(&tok, tok);
      cur_body = head_body;
    }
    init_nodetype(cur_body);
  }
  consume(&tok, tok, "}");
  idt->body = head_body;

  leave_scope();
  *rest = tok;
  return idt;
}

// declarator ::= declarator_prefix ident declarator_suffix
Type *declarator(Token **rest, Token *tok, Type *base_ty)
{

  Type *ty_prefix, *ty_suffix, *tmp_ty;
  Token *tmp_ident_name_tok;

  // get prefix type
  ty_prefix = declarator_prefix(&tok, tok);

  // get core ident.
  if (tok->kind != TK_IDENT)
    error_at(tok->pos, "it isn't ident.");
  tmp_ident_name_tok = tok;
  tok = tok->next;

  // get suffix type
  ty_suffix = declarator_suffix(&tok, tok);

  // join prefix
  tmp_ty = NULL;
  tmp_ty = ty_prefix;
  while (tmp_ty)
  {
    if (tmp_ty->ptr_to)
    {
      tmp_ty = tmp_ty->ptr_to;
    }
    else
    {
      tmp_ty->ptr_to = base_ty;
      base_ty = ty_prefix;
      break;
    }
  }

  // join suffix
  tmp_ty = NULL;
  tmp_ty = ty_suffix;
  while (tmp_ty)
  {
    if (tmp_ty->ptr_to)
    {
      tmp_ty = tmp_ty->ptr_to;
    }
    else
    {
      tmp_ty->ptr_to = base_ty;
      base_ty = ty_suffix;
      break;
    }
  }

  base_ty->ident_name_tok = tmp_ident_name_tok;
  *rest = tok;
  return base_ty;
}

// declarator_prefix := ("*" declarator_prefix)?
Type *declarator_prefix(Token **rest, Token *tok)
{

  Type *ty = NULL;

  // create dereferencer
  if (consume(&tok, tok, "*"))
  {
    ty = calloc(1, sizeof(Type));
    ty->kind = TY_PTR;
    ty->ptr_to = declarator_prefix(&tok, tok);
  }
  *rest = tok;
  return ty;
}

// declarator_suffix ::= ("[" num "]" declarator_suffix)?
Type *declarator_suffix(Token **rest, Token *tok)
{

  Type *ty = NULL;

  // create array type
  if (consume(&tok, tok, "["))
  {
    ty = calloc(1, sizeof(Type));
    ty->kind = TY_ARRAY;
    ty->array_size = expect_number(&tok, tok);
    expect(&tok, tok, "]");
    ty->ptr_to = declarator_suffix(&tok, tok);
  }

  *rest = tok;
  return ty;
}

/*
statement ::= (declaration_local|expr)? ";"
    |"{" statement? "}"
    | "return " expr ";"
    | "if" "(" expr ")" statement "else" statement
    | "while" "(" expr ")"  statement
    | "for" "(" (declaration_local|expr)? ";" expr? ";" expr? ";" ")"  statement
*/
Node *statement(Token **rest, Token *tok)
{
  Node *n;
  Node *n_block_cur;

  if (tok->kind == TK_KEYWORD)
  {

    if (equal_token(tok, ";"))
    {
      consume(&tok, tok, ";");
      *rest = tok;
      return new_node(ND_BLOCK, NULL, NULL);
    }

    if (equal_token(tok, "{"))
    {
      consume(&tok, tok, "{");
      n = new_node(ND_BLOCK, NULL, NULL);

      while (!consume(&tok, tok, "}"))
      {
        if (n->block_head)
        {
          n_block_cur->next = statement(&tok, tok);
          n_block_cur = n_block_cur->next;
        }
        else
        {
          n->block_head = statement(&tok, tok);
          n_block_cur = n->block_head;
        }
      }
      *rest = tok;
      return n;
    }
    else if (equal_token(tok, "return"))
    {
      tok = tok->next;
      n = new_node(ND_RETURN, expr(&tok, tok), NULL);
    }
    else if (equal_token(tok, "if"))
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
      if (equal_token(tok, "else"))
      {
        tok = tok->next;
        n->els = statement(&tok, tok);
      }

      *rest = tok;
      return n;
    }
    else if (equal_token(tok, "while"))
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
    else if (equal_token(tok, "for"))
    {

      tok = tok->next;

      n = new_node(ND_FOR, NULL, NULL);
      n->label_num = label_cnt;
      label_cnt++;
      if (consume(&tok, tok, "("))
      {
        // init
        if (!equal_token(tok, ";"))
          n->init = expr(&tok, tok);
        expect(&tok, tok, ";");
        // condition
        if (!equal_token(tok, ";"))
          n->cond = expr(&tok, tok);
        expect(&tok, tok, ";");
        // incriment
        if (!equal_token(tok, ")"))
          n->inc = expr(&tok, tok);
        expect(&tok, tok, ")");
      }
      n->then = statement(&tok, tok);
      *rest = tok;
      return n;
    }
    else
    {
      n = expr(&tok, tok);
    }
  }
  else
  {
    if (is_base_type_token(tok))
      n = declaration_local(&tok, tok);
    else
      n = expr(&tok, tok);
  }
  expect(&tok, tok, ";");
  *rest = tok;
  return n;
}

// expr ::= assgin
Node *expr(Token **rest, Token *tok)
{
  Node *n = assign(&tok, tok);
  *rest = tok;
  return n;
}

/*
declaration_local ::= base_type declarator ("," declarator)*
*/
Node *declaration_local(Token **rest, Token *tok)
{

  Type *ty = base_type(&tok, tok);
  ty = declarator(&tok, tok, ty);

  // todo declaration multi declaration

  *rest = tok;
  return new_node_declare_lvar(ty);
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

  Token *tok_dummy = tok; // extra_に渡すダミーのトークン。
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

  // pointer + num (=num + pointer)の場合はオフセットの計算
  // 逆だった場合正規化を行う。
  if (!is_ptr_node(lhs) && is_ptr_node(rhs))
  {
    Node *swp;
    swp = lhs;
    lhs = rhs;
    rhs = swp;
  }

  if (is_ptr_node(lhs) && !is_ptr_node(rhs))
  {
    Node *n = new_node(ND_MUL, rhs, new_node_num(calc_sizeof(lhs->ty->ptr_to)));
    return new_node(ND_ADD, lhs, n);
  }

  error("invalid types addtion.");
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
    return new_node(ND_SUB, lhs, new_node(ND_MUL, rhs, new_node_num(calc_sizeof(rhs->ty))));

  // pointer - pointerはアドレスの差
  if (is_ptr_node(lhs) && is_ptr_node(rhs))
    return new_node(ND_DIV, new_node(ND_SUB, lhs, rhs), new_node_num(calc_sizeof(lhs->ty->ptr_to)));

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
    n = new_node_num(calc_sizeof(n->ty));
  }
  else
    n = postfix(&tok, tok);

  *rest = tok;
  return n;
}

/*
postfix ::= primary ("[" & expr & "]")*
*/
Node *postfix(Token **rest, Token *tok)
{
  Node *n, *idx;
  n = primary(&tok, tok);

  for (;;)
  {
    // a[b] = *(a+(b*ty_size))
    if (consume(&tok, tok, "["))
    {
      idx = extra_add(n, expr(&tok, tok), NULL);
      expect(&tok, tok, "]");
      n = new_node(ND_DEREF, idx, NULL);
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
    |ident("(" expr? ("," expr)? ")")?
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
      Node *n_block_cur;
      while (!consume(&tok, tok, "}"))
      {
        if (n->block_head)
        {
          n_block_cur->next = statement(&tok, tok);
          n_block_cur = n_block_cur->next;
        }
        else
        {
          n->block_head = statement(&tok, tok);
          n_block_cur = n->block_head;
        }
      }
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
    if (equal_token(tok->next, "("))
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
