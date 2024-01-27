#define _XOPEN_SOURCE 700
#include "9cc.h"

static Ident *locals;
static Ident *globals;

Ident *function(Token **rest, Token *tok);
Node *statement(Token **rest, Token *tok);
Node *expr(Token **rest, Token *tok);
Node *declaration(Token **rest, Token *tok);
Type *declaration_suffix_array(Token **rest,Token *tok);
Node *assign(Token **rest, Token *tok);
Node *equality(Token **rest, Token *tok);
Node *relational(Token **rest, Token *tok);
Node *add(Token **rest, Token *tok);
Node *extra_add(Node *lhs,Node *rhs,Token *tok_dummy);
Node *extra_sub(Node *lhs,Node *rhs,Token *tok_dummy);
Node *mul(Token **rest, Token *tok);
Node *unary(Token **rest, Token *tok);
Node *postfix(Token **rest, Token *tok);
Node *primary(Token **rest, Token *tok);

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

bool equal(char *tar_str, char *op_str,int tar_len)
{
  return (strncmp(tar_str,op_str,tar_len) == 0 && op_str[tar_len] == '\0');
}

bool equal_token(Token *tok, char *op)
{
  return equal(tok->pos,op,tok->len);
}

bool is_identtype(char *str)
{
    char *s = str;

    //skip pointer keyword
    while (*s == '*') s++;

    if (equal(s,"int", 3)) return true;
    if (equal(s,"char", 4)) return true;

    return false;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(Token **rest, Token *tok, char *op)
{
  if (tok->kind != TK_KEYWORD || strlen(op) != tok->len || strncmp(tok->pos, op, strlen(op)) != 0)
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
  if (tok->kind != TK_KEYWORD || strlen(op) != tok->len || strncmp(tok->pos, op, strlen(op)) != 0)
    error_at(tok->pos, "'%s'ではありません。", op);
  *rest = tok->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number(Token **rest, Token *tok)
{
  if (tok->kind != TK_NUM)
    error_at(tok->pos, "数ではありません。");
  *rest = tok->next;
  return tok->val;
}

Type *get_type(Token **rest,Token *tok,char *ty_name,Type *ty_array)
{
  Type *ty;
  ty=calloc(1,sizeof(Type));
  ty->kind=-1;

  //create array type
  if(ty_array){
    ty = ty_array;
    ty->ptr_to = get_type(&tok,tok,ty_name,NULL);    
    *rest = tok;
    return ty;
  }

  //create pointer type
  if(equal_token(tok,"*")){
    ty->kind = TY_PTR;
    tok=tok->next;
    ty->ptr_to = get_type(&tok,tok,ty_name,NULL);
    *rest = tok;
    return ty;
  }

  //create else type
  if (equal(ty_name,"int", 3)) ty->kind = TY_INT;
  if (equal(ty_name,"char", 4)) ty->kind = TY_CHAR;

  if(ty->kind <0)
    error_at(NULL,"invalid data type.");

  *rest = tok;
  return ty;

}

//プレフィックスなしの識別子名を取得
Token *get_core_ident_token(Token *tok)
{
  //デリファレンサ、ポインタを除去
  while (equal(tok->pos,"*",1) || equal(tok->pos,"&",1) )
    tok = tok->next;
  
  if (tok->kind != TK_IDENT)
    error_at(tok->pos, "it isn't ident.");

  return tok;

}

//check if the variable is already declared
Ident *find_var(Token *tok)
{
  Token *core_ident_tok = get_core_ident_token(tok);

  //search locals
  for (Ident *v = locals; v; v = v->next)
  {
    if (equal(core_ident_tok->pos,v->name,core_ident_tok->len))
      return v;
  }

  //search globals  
  for (Ident *v = globals; v; v = v->next)
  {
    if (equal(core_ident_tok->pos,v->name,core_ident_tok->len))
      return v;
  }

  return NULL;
}

// 数値以外のノードの作成
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{  
  Node *n = calloc(1, sizeof(Node));  
  n->kind = kind;
  n->lhs = lhs;
  n->rhs = rhs;
  return n;
}

// 数値ノードの作成
Node *new_node_num(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;  
  node->val = val;
  init_nodetype(node);    
  return node;
}

Ident *declarator(Token **rest,Token *tok,char *ty_name,Type *ty_array,bool is_global){

  //remove prefix in ident
  Token *core_ident_tok = get_core_ident_token(tok);

  // find indent if it's already declared.
  if(find_var(core_ident_tok))
    error_at(tok->pos, "the variable is already declared.");
    
  // if couldn'd find ident in locals. add it to locals.
  Ident *idt = calloc(1, sizeof(Ident));
  idt->is_global = is_global;
  idt->name = strndup(core_ident_tok->pos, sizeof(char) * (core_ident_tok->len));
  idt->name_len = core_ident_tok->len;
  idt->ty = get_type(&tok,tok,ty_name,ty_array);

  tok = tok->next;

  //if this ident is function,set argments.
  idt->is_function = equal_token(tok, "(");
  Node *head_arg = NULL, *cur_arg = NULL, *head_body = NULL, *cur_body = NULL;  
  if(idt->is_function){
    consume(&tok, tok, "(");
    while (true)
    {
      if (equal_token(tok, ")"))
        break;
      if (head_arg)
      {
        cur_arg->next = declaration(&tok, tok);
        cur_arg = cur_arg->next;
      }
      else
      {
        head_arg = declaration(&tok, tok);
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
  }

  *rest = tok;
  return idt;
}

//変数ノードの作成
Node *new_node_declare_lvar(Token *tok,char *type_name,Type *ty_array)
{
  Node *node = calloc(1, sizeof(Node));    

  if(!is_identtype(type_name))
    error_at(tok->pos, "invalid data type.");    

  // find indent if it's already declared.
  if(find_var(tok))
    error_at(tok->pos, "the variable is already declared.");

  
  if (tok)
  {
    Ident *var = declarator(&tok,tok,type_name,ty_array,false);
    if (locals)
    {
      //after 2nd declare.
      for (Ident *v = locals;; v = v->next)
      {
        if (!v->next)
        {
          var->offset = v->offset + get_type_size(var->ty);
          v->next = var;
          break;
        }
      }
    }
    else
    {
      //1st declare.
      var->offset = get_type_size(var->ty);
      locals = var;
    }

    node->kind = ND_LVAR;    
    node->ident_name = var->name;
    node->offset = var->offset;
    node->ty = var->ty;    
  }
  return node;
}

//create variable node
Node *new_node_var(Token **rest, Token *tok)
{
  Node *n = NULL;
  if (tok)
  {

    //search the variable in globals or locals
    Ident *var = find_var(tok);
    if (!var){
      error_at(tok->pos, "the variable is not declared.");
    }

    n = calloc(1, sizeof(Node));
    n->ident_name = var->name;
    n->ty = var->ty;

    if(var->is_global){
      n->kind = ND_GVAR;    
    }
    else {
      n->kind = ND_LVAR;
      n->offset = var->offset;      
    }
  }
  *rest = tok->next;
  return n;
}

//create function node
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
    if (i >= FUNC_PRAM_NUM)
    {
      error("function aguments limit is %d", FUNC_PRAM_NUM);
    }
    node->func_param[i] = expr(&tok, tok);
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

// parse = (function)?
Ident *parse(Token *tok)
{

  label_cnt = 0;
  globals = NULL;
  Ident  *cur_globals = NULL,*tmp_globals=NULL;

  while (!at_eof(tok))
  {

    locals = NULL;
    tmp_globals = function(&tok, tok);
    tmp_globals->locals = locals;

    if (globals)
    {
      cur_globals->next = tmp_globals;
      cur_globals = cur_globals->next;
    }
    else
    {
      globals = tmp_globals;      
      cur_globals = globals;
    }
  }
  return globals;
}

/*
function ::= declaration "(" (declaration("," declaration)?)? ")" "{" statment? "}"
*/
Ident *function(Token **rest, Token *tok)
{
  char *ty_name;

  if (!is_identtype(tok->pos))
    error_at(tok->pos, "invalid data type.");    
  ty_name = tok->pos;

  tok=tok->next;

  Ident *fn = declarator(&tok,tok,ty_name,NULL,true);
  
  consume(&tok,tok,";");
  *rest = tok;
  return fn;
}

/*
statement ::= (declaration|expr)? ";"
    |"{" statement? "}"
    | "return " expr ";"
    | "if" "(" expr ")" statement "else" statement
    | "while" "(" expr ")"  statement
    | "for" "(" (declaration|expr)? ";" expr? ";" expr? ";" ")"  statement
*/
Node *statement(Token **rest, Token *tok)
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
          n_block_current->next = statement(&tok, tok);
          n_block_current = n_block_current->next;
        }
        else
        {
          n->block_head = statement(&tok, tok);
          n_block_current = n->block_head;
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
      if (is_identtype(tok->pos))
        n = declaration(&tok, tok);      
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
declaration ::= type ("*"*|"&"?)ident( declaration_suffix_array | declaration_suffix_function)
*/
Node *declaration(Token **rest, Token *tok)
{
  //ident_tok -> has prefix(&,*)
  //core_ident_tok -> remove prefix
  Token *ident_tok,*core_ident_tok;

  //get core type name
  char *ty_name=tok->pos;
  tok=tok->next;
  ident_tok = tok;

  //get core type name(remove prefix and suffix keyword.)
  core_ident_tok = get_core_ident_token(ident_tok);

  //function   
  if(equal_token(core_ident_tok->next,"(") )
    ;//todo

  //array
  Type *ty_array;  
  ty_array = declaration_suffix_array(&tok,core_ident_tok->next);  
  if(equal_token(core_ident_tok->next,"[") ){
    ;//todo
  }

  *rest = tok;
  
  return new_node_declare_lvar(ident_tok,ty_name,ty_array);
}

/*
declaration_suffix_array ::=  ("[" num "]")*
*/
Type *declaration_suffix_array(Token **rest, Token *tok){

  Type *ty=NULL;

  //create array type
  if (equal_token(tok, "[")){
    ty = calloc(1,sizeof(Type));
    consume(&tok, tok, "[");
    ty->kind = TY_ARRAY;
    ty->array_size = expect_number(&tok,tok);
    expect(&tok, tok, "]");    
    ty->ptr_to = declaration_suffix_array(&tok,tok);
  }

  *rest = tok;
  return ty;

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
      n = new_node(ND_LLESSEQ, n, add(&tok,tok));
    else if (consume(&tok, tok, ">="))
      n = new_node(ND_LLESSEQ, add(&tok,tok), n);
    else if (consume(&tok, tok, "<"))
      n = new_node(ND_LLESS, n, add(&tok,tok));
    else if (consume(&tok, tok, ">"))
      n = new_node(ND_LLESS, add(&tok,tok), n);
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

  Token *tok_dummy = tok;//extra_に渡すダミーのトークン。
  Node *n = mul(&tok, tok);

  for (;;)
  {
    if (consume(&tok, tok, "+")){
      n = extra_add(n,mul(&tok, tok),tok_dummy);
      continue;
    }

    if (consume(&tok, tok, "-")){
      n = extra_sub(n,mul(&tok, tok),tok_dummy);
      continue;
    }      

    *rest = tok;
    return n;
  }
}

//数値とポインタの組み合わせによってノードの加算方法をよしなに変える
Node *extra_add(Node *lhs,Node *rhs,Token *tok_dummy)
{

  init_nodetype(lhs);
  init_nodetype(rhs);

  //num + numの場合は普通の演算
  if(!is_ptr_node(lhs) && !is_ptr_node(rhs)) 
    return new_node(ND_ADD,lhs,rhs);

  //pointer + num (=num + pointer)の場合はオフセットの計算 
  //逆だった場合正規化を行う。
  if(!is_ptr_node(lhs) && is_ptr_node(rhs)) {
    Node *swp;
    swp = lhs;
    lhs = rhs;    
    rhs = swp;        
  }    

  if(is_ptr_node(lhs) && !is_ptr_node(rhs)) {
    Node *n = new_node(ND_MUL,rhs,new_node_num(get_type_size(lhs->ty->ptr_to)));
    return new_node(ND_ADD,lhs,n);
  }

  error("invalid types addtion.");
  return NULL;

}

//数値とポインタの組み合わせによってノードの減算方法をよしなに変える
Node *extra_sub(Node *lhs,Node *rhs,Token *tok_dummy)
{

  init_nodetype(lhs);
  init_nodetype(rhs);

  //num - numの場合は普通の演算
  if(!is_ptr_node(lhs) && !is_ptr_node(rhs)) 
    return new_node(ND_SUB,lhs,rhs);

  //pointer - num (=num + pointer)の場合はオフセットの計算 
  if(is_ptr_node(lhs) && !is_ptr_node(rhs)) 
    return new_node(ND_SUB,lhs,new_node(ND_MUL,rhs,new_node_num(get_type_size(rhs->ty))));

  //pointer - pointerはアドレスの差
  if(is_ptr_node(lhs) && is_ptr_node(rhs))
    return new_node(ND_DIV,new_node(ND_SUB,lhs,rhs),new_node_num(get_type_size(lhs->ty->ptr_to)));

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

  if(consume(&tok, tok, "+"))
    n = unary(&tok, tok);
  else if(consume(&tok, tok, "-"))
    n = new_node(ND_SUB, new_node_num(0), unary(&tok, tok));
  else if(consume(&tok, tok, "&"))
    n = new_node(ND_ADDR, unary(&tok, tok), NULL);
  else if(consume(&tok, tok, "*"))
    n = new_node(ND_DEREF, unary(&tok, tok), NULL);
  else if(consume(&tok, tok, "sizeof")){
    n = unary(&tok, tok);
    init_nodetype(n);
    n = new_node_num(get_type_size(n->ty));  
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
  Node *n;
  n = primary(&tok, tok);


  for (;;)
  {
    //a[b] = *(a+b)
    if (consume(&tok, tok, "[")){
      n = extra_add(n,expr(&tok, tok),NULL);
      n = new_node(ND_DEREF, n,NULL);
      expect(&tok,tok,"]");
      continue;
    }
    break;
  }

  *rest = tok;
  return n;

}

/*
primary ::= "(" expr ")"
    |ident("(" (expr("," expr)?)? ")")?
    |num
*/
Node *primary(Token **rest, Token *tok)
{
  Node *n;

  if (consume(&tok, tok, "("))
  {
    Node *n = expr(&tok, tok);
    expect(&tok, tok, ")");
    *rest = tok;
    return n;
  }

  if (tok->kind == TK_IDENT)
  {
    //todo:()の有無ではなくident listから今後は変数判定するように変更。
    if (equal_token(tok->next, "("))
      n = new_node_function(&tok, tok);
    else
      n = new_node_var(&tok, tok);
  }
  else
    n = new_node_num(expect_number(&tok, tok));

  *rest = tok;
  return n;
}
