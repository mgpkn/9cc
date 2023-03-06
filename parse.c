#include "9cc.h"
#include <string.h>

extern Token *token; // 現在着目しているトークン

extern char *user_input; // main関数の引数
extern int label_cnt;

// エラーを報告するための関数
// printfと同じ引数を取る
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


bool equal_token(Token *tk,char *op){
  return (strncmp(tk->str,op,tk->len)==0 && tk->len>0);
}


// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_KEYWORD
      || strlen(op) != token->len
      ||  strncmp(token->str,op,strlen(op)) != 0)    
    return false;
  token = token->next;
  return true;
}

bool consume_number(char *op) {
  if (token->kind != TK_NUM)  
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_KEYWORD
      || strlen(op) != token->len
      ||  strncmp(token->str,op,strlen(op)) != 0)    
    error_at(token->str,"'%s'ではありません", op);    
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str,"数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

extern LVar *locals;

LVar *find_lvar(Token *tok)
{

  for (LVar *v = locals; v; v = v->next)
  {
    if (tok->len == v->len && strncmp(tok->str, v->name, tok->len) == 0)
      return v;
  }

  return NULL;
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
Node *new_node_lval(Token *t)
{
  Node *node = NULL;
  if (t)
  {
    node = calloc(1, sizeof(Node));
    node->kind = ND_LVAL;

    // すでに宣言された変数かを確認する
    LVar *lvar = find_lvar(t);
    if (lvar)
    {
      // 宣言済ならその変数のオフセットを返却。
      node->offset = lvar->offset;
    }
    else
    {
      // なければlocalsに追加
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = t->str;
      lvar->len = t->len;
      if (locals)
      {
        // frm 2nd onwards call
        lvar->offset = locals->offset + OFFSETSIZE;
      }
      else
      {
        // 1st call
        lvar->offset = OFFSETSIZE;
      }
      locals = lvar;

      node->ident_name = strndup(t->str, sizeof(char) * (t->len));
      node->offset = lvar->offset;
    }
  }
  return node;
}


// 関数ノードの作成
Node *new_node_function(Token *t)
{
  Node *node;
  int i=0;

  node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->ident_name = strndup(t->str, sizeof(char) * (t->len));

  if(consume(")")) return node; //引数がなければ即return;

  while(true){
    if(i>=FUNC_PRAM_NUM){
      error("引数は%d個までです。",FUNC_PRAM_NUM);
    }
    node->func_param[i] = expr();
    if(!consume(",")) break;
    i++;
  }
  expect(")");
  return node;
}


Token *fetch_current_token(){
  Token *t = token;  
  if (token->kind != TK_IDENT)  
    error_at(token->str,"変数ではありません");
  token = token->next;
  return t;
}


void program();
Node *statement();
Node *assign();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

extern Node *code[NODENUM];

bool at_eof() {
  return token->kind == TK_EOF;
}

void program()
{
  int i = 0;
  while (!at_eof())
  {
    code[i] = statement();
    i++;
  }
}

Node *statement()
{
  Node *n;
  Node *n_block_current;

  if (token->kind == TK_KEYWORD)
  {

    if (equal_token(token, "{"))
    {
      consume("{");
      n = new_node(ND_BLOCK, NULL, NULL);

      while (!consume("}"))
      {
        if (n->block_head)
        {
          n_block_current->next = statement();
          n_block_current = n_block_current->next;
        }
        else
        {
          n->block_head = statement();
          n_block_current = n->block_head;
        }
      }
      return n;
    }
    else if (equal_token(token, "return"))
    {

      token = token->next;
      n = new_node(ND_RETURN, expr(), NULL);
    }
    else if (equal_token(token, "if"))
    {

      token = token->next;
      n = new_node(ND_IF, NULL, NULL);
      n->label_num = label_cnt;
      label_cnt++;
      if (consume("("))
      {
        n->cond = expr();
        expect(")");
      }
      n->then = statement();

      // else
      if (equal_token(token, "else"))
      {
        token = token->next;
        n->els = statement();
      }
      return n;
    }
    else if (equal_token(token, "while"))
    {

      token = token->next;

      n = new_node(ND_WHILE, NULL, NULL);
      n->label_num = label_cnt;
      label_cnt++;
      if (consume("("))
      {
        n->cond = expr();
        expect(")");
      }
      n->then = statement();
      return n;
    }
    else if (equal_token(token, "for"))
    {

      token = token->next;

      n = new_node(ND_FOR, NULL, NULL);
      n->label_num = label_cnt;
      label_cnt++;
      if (consume("("))
      {
        // init
        if (!equal_token(token, ";"))
          n->init = expr();
        expect(";");
        // condition
        if (!equal_token(token, ";"))
          n->cond = expr();
        expect(";");
        // incriment
        if (!equal_token(token, ")"))
          n->inc = expr();
        expect(")");
      }
      n->then = statement();
      return n;
    }
    else
    {

      n = expr();
    }
  }
  else
  {
    n = expr();
  }
  expect(";");
  return n;
}

Node *expr()
{
  Node *n = assign();
  return n;
}

Node *assign()
{
  Node *n = equality();
  if (consume("="))
  {
    n = new_node(ND_ASSIGN, n, assign());
  }
  return n;
}

Node *equality()
{

  Node *n = relational();

  for (;;)
  {
    if (consume("=="))
      n = new_node(ND_EQ, n, relational());
    else if (consume("!="))
      n = new_node(ND_NOTEQ, n, relational());
    else
      return n;
  }
}

Node *relational()
{

  Node *n = add();

  for (;;)
  {
    //>および>=不等号が逆の場合は左右のノード自体を逆に生成する
    if (consume("<="))
      n = new_node(ND_LLESSEQ, n, add());
    else if (consume(">="))
      n = new_node(ND_LLESSEQ, add(), n);
    else if (consume("<"))
      n = new_node(ND_LLESS, n, add());
    else if (consume(">"))
      n = new_node(ND_LLESS, add(), n);
    else
      return n;
  }
}

Node *add()
{

  Node *n = mul();

  for (;;)
  {
    if (consume("+"))
      n = new_node(ND_ADD, n, mul());
    else if (consume("-"))
      n = new_node(ND_SUB, n, mul());
    else
      return n;
  }
}

Node *mul()
{

  Node *n = unary();

  for (;;)
  {
    if (consume("*"))
      n = new_node(ND_MUL, n, unary());
    else if (consume("/"))
      n = new_node(ND_DIV, n, unary());
    else if (consume("%"))
      n = new_node(ND_MOD, n, unary());
    else
      return n;
  }
}

// 数値などの正負の項
Node *unary()
{

  Node *n;

  if (consume("+"))
    n = primary();
  else if (consume("-"))
    n = new_node(ND_SUB, new_node_num(0), primary());
  else if (consume("&"))
  {
    // todo:ポインタのアドレス
  }
  else if (consume("*"))
  {
    // todo:ポインタのデリファレンサ
  }
  else
    n = primary();
  return n;
}

Node *primary()
{

  Token *t;
  Node *n;

  if (consume("("))
  {
    Node *n = expr();
    expect(")");
    return n;
  }

  if (token->kind == TK_IDENT)
  {

    t = fetch_current_token();
    if (consume("("))
    {
      n = new_node_function(t);
    }
    else
    {
      return new_node_lval(t);
    }
  }
  else
    n = new_node_num(expect_number());

  return n;
}
