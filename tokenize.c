#define _XOPEN_SOURCE 700
#include "9cc.h"

// create new token and join cur.
Token *new_token(TokenKind kind, Token *cur, char *pos, int len)
{

  Token *tok = calloc(1, sizeof(Token));

  tok->kind = kind;
  tok->len = len;
  tok->pos = pos;
  cur->next = tok;

  return tok;
}

// アルファベットもしくは条件付きで数値かどうか？
bool is_alnum(char c, bool allow_num)
{

  //[a-zA-Z_]は有効
  if (('a' <= c && c <= 'z') ||
      ('A' <= c && c <= 'Z') ||
      c == '_')
  {
    return true;
  }

  // allow_numなら[0-9]も有効
  if (allow_num && ('0' <= c && c <= '9'))
  {
    return true;
  }

  return false;
}

Token *tokenize(char *p)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;
  int i;
  int estamate_len;

  while (*p)
  {
    // skip space.
    if (isspace(*p))
    {
      p++;
      continue;
    }

    estamate_len = 6;
    if (strncmp(p, "sizeof", estamate_len) == 0)
    {
      cur = new_token(TK_KEYWORD, cur, p, estamate_len);
      p += estamate_len;
      continue;
    }

    // 2 length token.
    estamate_len = 2;
    if (strncmp(p, "<=", estamate_len) == 0 || strncmp(p, ">=", estamate_len) == 0 || strncmp(p, "==", estamate_len) == 0 || strncmp(p, "!=", estamate_len) == 0)
    {
      cur = new_token(TK_KEYWORD, cur, p, estamate_len);
      p += estamate_len;
      continue;
    }

    // 1 length token.
    estamate_len = 1;
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '%' ||
        *p == '(' || *p == ')' || *p == '[' || *p == ']' ||
        *p == '{' || *p == '}' || *p == '&' ||
        *p == '<' || *p == '>' || *p == '=' ||
        *p == ';' || *p == ',')
    {
      cur = new_token(TK_KEYWORD, cur, p, 1);
      p += estamate_len;
      continue;
    }

    // string(")
    if(*p == '"'){
        //serach close double quote.
        for(i=1;*(p+i) != '"';i++){
            if(*(p+i)=='\n' || *(p+i)=='\0')
              error_at(p,"coludn't find closed double quote.");
        }
        cur = new_token(TK_STR, cur, p,i-1);
        cur->str = strndup(cur->pos+1, sizeof(char) * cur->len);
        p+= i+1;
        continue;
    }

    // keyword tokens.
    // return
    estamate_len = 6;
    if (strncmp(p, "return", estamate_len) == 0 && !is_alnum(*(p + estamate_len), true))
    {
      cur = new_token(TK_KEYWORD, cur, p, estamate_len);
      p += estamate_len;
      continue;
    }

    // while
    estamate_len = 5;
    if (strncmp(p, "while", estamate_len) == 0 && !is_alnum(*(p + estamate_len), true))
    {
      cur = new_token(TK_KEYWORD, cur, p, estamate_len);
      p += estamate_len;
      continue;
    }

    // for
    estamate_len = 3;
    if (strncmp(p, "for", estamate_len) == 0 && !is_alnum(*(p + estamate_len), true))
    {
      cur = new_token(TK_KEYWORD, cur, p, estamate_len);
      p += estamate_len;
      continue;
    }

    // if
    estamate_len = 2;
    if (strncmp(p, "if", estamate_len) == 0 && !is_alnum(*(p + estamate_len), true))
    {
      cur = new_token(TK_KEYWORD, cur, p, estamate_len);
      p += estamate_len;
      continue;
    }

    // else
    estamate_len = 4;
    if (strncmp(p, "else", estamate_len) == 0 && !is_alnum(*(p + estamate_len), true))
    {
      cur = new_token(TK_KEYWORD, cur, p, estamate_len);
      p += estamate_len;
      continue;
    }

    // type,ident
    // find possible ident token.
    i = 0;
    while (true)
    {
      if (is_alnum(*(p + i), i > 0))
      {
        i++;
        continue;
      }
      // if don't find,break.
      break;
    }

    if (i > 0)
    {
      cur = new_token(TK_IDENT, cur, p, i);
      p += i;
      continue;
    }

    // digit token.
    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("tokenize is failed.");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}