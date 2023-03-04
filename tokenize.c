#include "9cc.h"
#include <string.h>

extern Token *token;// 現在着目しているトークン



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

Token *fetch_current_token(){
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

  tok->kind = kind;
  tok->str = str;  
  tok->len = len;

  cur->next = tok;
  
  return tok;
}

bool equal_token(Token *tk,char *op){

  return (strncmp(tk->str,op,tk->len)==0 && tk->len>0);
  //return (strncmp(tk->str,op,tk->len)==0 && op[tk->len]=='\0');
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
  int estamate_len;
  
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    
    //2文字の演算子のトークナイズ
    estamate_len=2;    
    if (strncmp(p,"<=",estamate_len) == 0
	|| strncmp(p,">=",estamate_len) == 0
	|| strncmp(p,"==",estamate_len) == 0
	|| strncmp(p,"!=",estamate_len) == 0
	) {
      cur = new_token(TK_KEYWORD, cur,p,estamate_len);
      p += estamate_len;      
      continue;
    }

    //1文字の演算子のトークナイズ
    estamate_len=1;
    if (*p == '+' || *p == '-' ||
	*p == '*' || *p == '/' || *p == '%'||
        *p == '(' || *p == ')' ||
        *p == '{' || *p == '}' ||	
	*p == '<' || *p == '>' ||
        *p == '=' ||
	*p == ';') 
      {
      cur = new_token(TK_KEYWORD,cur,p,1);
      p += estamate_len;      
      continue;
    }

    //その他キーワードのトークナイズ
    //return
    estamate_len=6;
    if (strncmp(p,"return",estamate_len)==0 && !is_alnum(*(p+estamate_len),true)){
      cur = new_token(TK_KEYWORD,cur,p,estamate_len);
      p += estamate_len;      
      continue;
    }

    //while
    estamate_len=5;
    if (strncmp(p,"while",estamate_len)==0 && !is_alnum(*(p+estamate_len),true)){
      cur = new_token(TK_KEYWORD,cur,p,estamate_len);
      p += estamate_len;      
      continue;
    }
    
    //for
    estamate_len=3;
    if (strncmp(p,"for",estamate_len)==0 && !is_alnum(*(p+estamate_len),true)){
      cur = new_token(TK_KEYWORD,cur,p,estamate_len);
      p += estamate_len;      
      continue;
    }

    //if
    estamate_len=2;
    if (strncmp(p,"if",estamate_len)==0 && !is_alnum(*(p+estamate_len),true)){
      cur = new_token(TK_KEYWORD,cur,p,estamate_len);
      p += estamate_len;      
      continue;
    }

    //else
    estamate_len=4;
    if (strncmp(p,"else",estamate_len)==0 && !is_alnum(*(p+estamate_len),true)){
      cur = new_token(TK_KEYWORD,cur,p,estamate_len);
      p += estamate_len;      
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


