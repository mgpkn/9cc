#define _XOPEN_SOURCE 700
#include <string.h>
#include "9cc.h"

Ident *function(Token **rest, Token *tok);
Node *statement(Token **rest, Token *tok, Ident **lvar_head);
Node *expr(Token **rest, Token *tok, Ident **lvar_head);
Node *assign(Token **rest, Token *tok, Ident **lvar_head);
Node *equality(Token **rest, Token *tok, Ident **lvar_head);
Node *relational(Token **rest, Token *tok, Ident **lvar_head);
Node *add(Token **rest, Token *tok, Ident **lvar_head);
Node *mul(Token **rest, Token *tok, Ident **lvar_head);
Node *unary(Token **rest, Token *tok, Ident **lvar_head);
Node *primary(Token **rest, Token *tok, Ident **lvar_head);

extern char *user_input; // main関数の引数
int label_cnt;

// エラーを報告するための関数
// printfと同じ引数を取る50
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool equal_token(Token *tok, char *op)
{
  return (strncmp(tok->str, op, tok->len) == 0 && tok->len > 0);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(Token **rest, Token *tok, char *op)
{
  if (tok->kind != TK_KEYWORD || strlen(op) != tok->len || strncmp(tok->str, op, strlen(op)) != 0)
    return false;
  *rest = tok->next;
  return true;
}

bool consume_number(Token **rest, Token *tok, char *op)
{
  if (tok->kind != TK_NUM)
    return false;
  *rest = tok->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(Token **rest, Token *tok, char *op)
{
  if (tok->kind != TK_KEYWORD || strlen(op) != tok->len || strncmp(tok->str, op, strlen(op)) != 0)
    error_at(tok->str, "'%s'ではありません", op);
  *rest = tok->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number(Token **rest, Token *tok)
{
  if (tok->kind != TK_NUM)
    error_at(tok->str, "数ではありません");
  int val = tok->val;
  *rest = tok->next;
  return val;
}

Ident *find_lvar(Token *tok, Ident **lvar_head)
{
  for (Ident *v = *lvar_head; v; v = v->next)
  {
    if (tok->len == v->name_len && strncmp(tok->str, v->name, tok->len) == 0)
      return v;
  }
  return NULL;
}

Ident *find_last_declare_val(Ident *lvar_head)
{
  if (!lvar_head)
    return lvar_head;
  for (Ident *v = lvar_head;; v = v->next)
  {
    if (!v->next)
      return v;
  }
}

// 数値以外のノードの作成
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// 数値ノードの作成
Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// 変数ノードの作成
Node *new_node_lvar(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *node = NULL;
  if (tok)
  {
    node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    // すでに宣言されたローカル変数かを確認する
    Ident *lvar = find_lvar(tok, lvar_head);
    if (!lvar)
    {
      // なければlocalsに追加
      lvar = calloc(1, sizeof(Ident));
      lvar->is_function = false;
      lvar->name = strndup(tok->str, sizeof(char) * (tok->len));
      lvar->name_len = tok->len;
      if (*lvar_head)
      {
        // ２回目以降の宣言の場合
        for (Ident *v = *lvar_head;; v = v->next)
        {
          if (!v->next)
          {
            lvar->offset = v->offset + OFFSETSIZE;
            v->next = lvar;
            break;
          }
        }
      }
      else
      {
        // １回目の宣言の場合
        lvar->offset = OFFSETSIZE;
        *lvar_head = lvar;
      }
      node->ident_name = lvar->name;
    }
    node->offset = lvar->offset;
  }
  *rest = tok->next;
  return node;
}

// 関数ノードの作成
Node *new_node_function(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *node;
  int i = 0;

  node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->ident_name = strndup(tok->str, sizeof(char) * (tok->len));
  tok = tok->next;
  expect(&tok, tok, "(");
  if (consume(&tok, tok, ")"))
  {
    *rest = tok;
    return node; // 引数がなければ即return;
  }

  while (true)
  {
    if (i >= FUNC_PRAM_NUM)
    {
      error("引数は%d個までです。", FUNC_PRAM_NUM);
    }
    node->func_param[i] = expr(&tok, tok, lvar_head);
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

// parse = function?
Ident *parse(Token *tok)
{

  label_cnt = 0;
  Ident *head_fn = NULL, *cur_fn = NULL;
  while (!at_eof(tok))
  {
    if (head_fn)
    {
      cur_fn->next = function(&tok, tok);
      cur_fn = cur_fn->next;
    }
    else
    {
      head_fn = function(&tok, tok);
      cur_fn = head_fn;
    }
  }
  return head_fn;
}

// function = ident "(" (ident("," ident)?)? ")" "{" statment? "}"

Ident *function(Token **rest, Token *tok)
{

  Ident *fn;
  Node *head_param = NULL, *cur_param = NULL, *head_body = NULL, *cur_body = NULL;

  // function_name
  if (tok->kind != TK_IDENT)
  {
    error_at(tok->str, "関数名が不正です。");
  }
  fn = calloc(1, sizeof(Ident));
  fn->is_function = true;
  fn->name = strndup(tok->str, sizeof(char) * (tok->len));
  fn->name_len = tok->len;
  tok = tok->next;

  // params
  expect(&tok, tok, "(");
  while (true)
  {
    if (equal_token(tok, ")"))
      break;
    if (head_param)
    {
      cur_param->next = expr(&tok, tok, &(fn->localvar));
      cur_param = cur_param->next;
    }
    else
    {
      head_param = expr(&tok, tok, &(fn->localvar));
      cur_param = head_param;
    }
    if (!consume(&tok, tok, ","))
      break;
  }
  consume(&tok, tok, ")");
  fn->param = head_param;

  // body
  expect(&tok, tok, "{");
  while (true)
  {
    if (equal_token(tok, "}"))
      break;
    if (head_body)
    {
      cur_body->next = statement(&tok, tok, &(fn->localvar));
      cur_body = cur_body->next;
    }
    else
    {
      head_body = statement(&tok, tok, &(fn->localvar));
      cur_body = head_body;
    }
  }
  consume(&tok, tok, "}");
  fn->body = head_body;

  *rest = tok;
  return fn;
}

/*
statement = expr? ";"
    |"{" statement "}"
    | "return " expr ";"
    | "if" "(" expr ")" statement "else" statement
    | "while" "(" expr ")"  statement
    | "for" "(" expr? ";" expr? ";" expr? ";" ")"  statement
*/
Node *statement(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *n;
  Node *n_block_current;

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
          n_block_current->next = statement(&tok, tok, lvar_head);
          n_block_current = n_block_current->next;
        }
        else
        {
          n->block_head = statement(&tok, tok, lvar_head);
          n_block_current = n->block_head;
        }
      }
      *rest = tok;
      return n;
    }
    else if (equal_token(tok, "return"))
    {
      tok = tok->next;
      n = new_node(ND_RETURN, expr(&tok, tok, lvar_head), NULL);
    }
    else if (equal_token(tok, "if"))
    {

      tok = tok->next;
      n = new_node(ND_IF, NULL, NULL);
      n->label_num = label_cnt;
      label_cnt++;
      if (consume(&tok, tok, "("))
      {
        n->cond = expr(&tok, tok, lvar_head);
        expect(&tok, tok, ")");
      }
      n->then = statement(&tok, tok, lvar_head);

      // else
      if (equal_token(tok, "else"))
      {
        tok = tok->next;
        n->els = statement(&tok, tok, lvar_head);
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
        n->cond = expr(&tok, tok, lvar_head);
        expect(&tok, tok, ")");
      }
      n->then = statement(&tok, tok, lvar_head);

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
          n->init = expr(&tok, tok, lvar_head);
        expect(&tok, tok, ";");
        // condition
        if (!equal_token(tok, ";"))
          n->cond = expr(&tok, tok, lvar_head);
        expect(&tok, tok, ";");
        // incriment
        if (!equal_token(tok, ")"))
          n->inc = expr(&tok, tok, lvar_head);
        expect(&tok, tok, ")");
      }
      n->then = statement(&tok, tok, lvar_head);
      *rest = tok;
      return n;
    }
    else
    {
      n = expr(&tok, tok, lvar_head);
    }
  }
  else
  {
    n = expr(&tok, tok, lvar_head);
  }
  expect(&tok, tok, ";");
  *rest = tok;
  return n;
}

// expr = assgin
Node *expr(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *n = assign(&tok, tok, lvar_head);
  *rest = tok;
  return n;
}

// assign = equality ("=" assign )?
Node *assign(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *n = equality(&tok, tok, lvar_head);
  if (consume(&tok, tok, "="))
    n = new_node(ND_ASSIGN, n, assign(&tok, tok, lvar_head));

  *rest = tok;
  return n;
}

// equality = relational ("==" relational |"!=" relational )*
Node *equality(Token **rest, Token *tok, Ident **lvar_head)
{

  Node *n = relational(&tok, tok, lvar_head);

  for (;;)
  {
    if (consume(&tok, tok, "=="))
    {
      n = new_node(ND_EQ, n, relational(&tok, tok, lvar_head));
    }
    else if (consume(&tok, tok, "!="))
    {
      n = new_node(ND_NOTEQ, n, relational(&tok, tok, lvar_head));
    }
    else
    {
      *rest = tok;
      return n;
    }
  }
}

// relational = add (">" add | "<" add | ">=" add | "<=" add)*
Node *relational(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *n = add(&tok, tok, lvar_head);

  for (;;)
  {
    //>および>=不等号が逆の場合は左右のノード自体を逆に生成する
    if (consume(&tok, tok, "<="))
      n = new_node(ND_LLESSEQ, n, add(&tok, tok, lvar_head));
    else if (consume(&tok, tok, ">="))
      n = new_node(ND_LLESSEQ, add(&tok, tok, lvar_head), n);
    else if (consume(&tok, tok, "<"))
      n = new_node(ND_LLESS, n, add(&tok, tok, lvar_head));
    else if (consume(&tok, tok, ">"))
      n = new_node(ND_LLESS, add(&tok, tok, lvar_head), n);
    else
    {
      *rest = tok;
      return n;
    }
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add(Token **rest, Token *tok, Ident **lvar_head)
{

  Node *n = mul(&tok, tok, lvar_head);

  for (;;)
  {
    if (consume(&tok, tok, "+"))
      n = new_node(ND_ADD, n, mul(&tok, tok, lvar_head));
    else if (consume(&tok, tok, "-"))
      n = new_node(ND_SUB, n, mul(&tok, tok, lvar_head));
    else
    {
      *rest = tok;
      return n;
    }
  }
}

// mul = unary ("*" unary|"/" unary|"%" unary)*
Node *mul(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *n = unary(&tok, tok, lvar_head);

  for (;;)
  {
    if (consume(&tok, tok, "*"))
      n = new_node(ND_MUL, n, unary(&tok, tok, lvar_head));
    else if (consume(&tok, tok, "/"))
      n = new_node(ND_DIV, n, unary(&tok, tok, lvar_head));
    else if (consume(&tok, tok, "%"))
      n = new_node(ND_MOD, n, unary(&tok, tok, lvar_head));
    else
    {
      *rest = tok;
      return n;
    }
  }
}

/*
数値などの正負の項
unary = ("+"|"-")? primary
    |("&"|"*") unary

*/
Node *unary(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *n;

  if (consume(&tok, tok, "+"))
    n = primary(&tok, tok, lvar_head);
  else if (consume(&tok, tok, "-"))
    n = new_node(ND_SUB, new_node_num(0), primary(&tok, tok, lvar_head));
  else if (consume(&tok, tok, "&"))
  {
    n = new_node(ND_ADDR, unary(&tok, tok, lvar_head), NULL);
  }
  else if (consume(&tok, tok, "*"))
  {
    n = new_node(ND_DEREF, unary(&tok, tok, lvar_head), NULL);
  }
  else
    n = primary(&tok, tok, lvar_head);

  *rest = tok;
  return n;
}

/*
primary = "(" expr ")"
    |ident("(" (expr("," expr)?)? ")")?
    |num
*/
Node *primary(Token **rest, Token *tok, Ident **lvar_head)
{
  Node *n;

  if (consume(&tok, tok, "("))
  {
    Node *n = expr(&tok, tok, lvar_head);
    expect(&tok, tok, ")");
    *rest = tok;
    return n;
  }

  if (tok->kind == TK_IDENT)
  {
    if (equal_token(tok->next, "("))
      n = new_node_function(&tok, tok, lvar_head);
    else
      n = new_node_lvar(&tok, tok, lvar_head);
  }
  else
    n = new_node_num(expect_number(&tok, tok));

  *rest = tok;
  return n;
}
