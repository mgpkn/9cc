#include "9cc.h"

extern char *user_input;//main関数の引数

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
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

extern LVar *locals;

LVar *find_lvar(Token *tok){

  for(LVar *v=locals;v;v=v->next){
    if(tok->len==v->len && strncmp(tok->str,v->name,tok->len) == 0) return v;
    }
  
  return NULL;
}


extern Token *token;// 現在着目しているトークン

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED
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
  if (token->kind != TK_RESERVED
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

Token *expect_ident() {
  if (token->kind != TK_IDENT)  
    error_at(token->str,"変数ではありません");
  Token *t = token;
  token = token->next;
  return t;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str,int len) {

  Token *tok = calloc(1, sizeof(Token));
  //fprintf(stderr,"param str:%s len:%d\n",str,len);                    

  tok->kind = kind;
  tok->str = str;  
  strncpy(tok->str,str,len);
  tok->len = len;

  //fprintf(stderr,"len:%d,value:%d, str:%s\n",tok->len,tok->val,tok->str);                
  cur->next = tok;
  
  return tok;
}


//アルファベットもしくは条件付きで数値かどうか？
bool is_alnum(char c,bool allow_num){

  //[a-zA-Z_]は有効
  if(('a' <= c && c <= 'z') ||
     ('A' <= c && c <= 'Z') ||
     c =='_' ){
    return true;
    }

  //allow_numなら[0-9]も有効
  if(allow_num && ('0' <= c && c <= '9')) {
    return true;
  }

  return false;
  
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int i;
  
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    
    //2文字の演算子のトークナイズ
    if (strncmp(p,"<=",2) == 0
	|| strncmp(p,">=",2) == 0
	|| strncmp(p,"==",2) == 0
	|| strncmp(p,"!=",2) == 0
	) {
      cur = new_token(TK_RESERVED, cur,p,2);
      p += 2;      
      continue;
    }

    //1文字の演算子のトークナイズ
    if (*p == '+' || *p == '-' ||
	*p == '*' || *p == '/' || *p == '%'||
        *p == '(' || *p == ')' ||
	*p == '<' || *p == '>'||
        *p == '=' ||
	*p == ';') 
      {
      cur = new_token(TK_RESERVED,cur,p,1);
      p += 1;      
      continue;
    }

    //その他キーワードのトークナイズ
    //return
    if (strncmp(p,"return",6)==0 && !is_alnum(*(p+6),true)){
      cur = new_token(TK_RETURN,cur,p,6);
      p += 6;      
      continue;
    }
    
    //変数のトークナイズ
    //変数として有効な文字の探索
    i=0;
    while(true){

      if(is_alnum(*(p+i),i>0)){
	i++;
	continue;
	}

      //変数のルールに該当しなかった場合はブレイク
      break;
    }
    
    if(i>0) {
      cur = new_token(TK_IDENT, cur,p,i);
      p += i;      
      continue;
    }
    
   
    //整数値のトークナイズ
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p,0);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    
    error("トークナイズできません");
  }

  new_token(TK_EOF, cur, p,0);
  return head.next;
}



//数値以外のノードの作成
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

//数値ノードの作成
Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

//変数ノードの作成
Node *new_node_ident(Token *t) {
  Node *node =NULL;
  if(t){
    node = calloc(1, sizeof(Node));
    node->kind = ND_LVAL;


    //すでに宣言された変数かを確認する
    LVar *lvar = find_lvar(t);
    if (lvar){
      //宣言済ならその変数のオフセットを返却。
      node->offset = lvar->offset;
    }else{
      //なければlocalsに追加
      lvar = calloc(1,sizeof(LVar));
      lvar->next = locals;
      lvar->name = t->str;
      lvar->len = t->len;
      if(locals){
	lvar->offset = locals->offset+OFFSETVAL;
      }else{
	//1st declare
	lvar->offset = OFFSETVAL;	
      }
      locals = lvar;
      node->offset = lvar->offset;          
    }
  }
  return node;
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
void program(){
  int i=0;
  while(!at_eof()){
      code[i] =statement();
      i++;
   }
}

Node *statement(){
  Node *n;

  if(token->kind==TK_RETURN){
    //fprintf(stderr,"gen return\n");
    token = token->next;
    n = new_node(ND_RETURN,expr(),NULL);
  } else {
    n=expr();
  }

  expect(";");  
  return n;
}


Node *expr(){
  Node *n = assign();
  return n;
}


Node *assign(){
  Node *n = equality();
  if(consume("=")){
      n = new_node(ND_ASSIGN,n,assign());    
  }
  return n;
}

Node *equality(){

  Node *n = relational();
  
  for(;;){
    if (consume("=="))
      n = new_node(ND_EQ,n,relational());
    else if (consume("!="))
      n = new_node(ND_NOTEQ,n,relational());      
    else
      return n;
  }
  
}

Node *relational(){

  Node *n = add();
  
  for(;;){
    //>および>=不等号が逆の場合は左右のノード自体を逆に生成する    
    if (consume("<="))
      n = new_node(ND_LLESSEQ,n,add());
    else if (consume(">="))
      n = new_node(ND_LLESSEQ,add(),n);
    else if (consume("<"))
      n = new_node(ND_LLESS,n,add());      
    else if (consume(">"))
      n = new_node(ND_LLESS,add(),n);
    else
      return n;
  }
  
}

Node *add(){

  Node *n = mul();
  
  for(;;){
    if (consume("+"))
      n = new_node(ND_ADD,n,mul());
    else if (consume("-"))
      n = new_node(ND_SUB,n,mul());      
    else
      return n;
  }
  
}

Node *mul(){

  Node *n = unary();
	      
  for(;;){
    if (consume("*"))
      n = new_node(ND_MUL,n,unary());
    else if (consume("/"))
      n = new_node(ND_DIV,n,unary());
    else if (consume("%"))
      n = new_node(ND_MOD,n,unary());
    else
      return n;
  }
}

Node *unary(){

  Node *n;

  if (consume("+"))
    n = primary();
  else if (consume("-"))
    n = new_node(ND_SUB,new_node_num(0),primary());
  else if (consume("&")){
    //todo:ポインタのアドレス
  }
  else if (consume("*")){
    //todo:ポインタのデリファレンサ
  }
  else
    n = primary();    
  return n;

}

Node *primary(){

  if(consume("(")){
    Node *n = expr();
    expect(")");
    return n;
  }

  if (token->kind == TK_IDENT )
    return new_node_ident(expect_ident());
  else
    return new_node_num(expect_number());

}

  



