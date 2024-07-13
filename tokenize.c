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
  int i, j;
  int estamate_len;

  while (*p)
  {
    // skip space.
    if (isspace(*p))
    {
      p++;
      continue;
    }

    // line comment
    estamate_len = 2;
    if (strncmp(p, "//", estamate_len) == 0)
    {
      p += estamate_len;
      while (!(*p == '\n' || *p == '\0'))
        p++;
      p++;
      continue;
    }

    // block comment
    if (strncmp(p, "/*", estamate_len) == 0)
    {
      char *q = strstr(p + estamate_len, "*/");
      if (!q)
        error_at(p, "comment isn't closed");
      p = q + estamate_len;
      continue;
    }

    // 6 length token.
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
    if (*p == '"')
    {
      // allocate memory based on maximum literal string length
      for (i = 1; *(p + i) != '"'; i++)
      {
        // if can't find closed double quote,raise error.
        if (*(p + i) == '\n' || *(p + i) == '\0')
          error_at(p, "cant't find closed double quote.");
      }
      cur = new_token(TK_STR, cur, p, i);
      cur->str = calloc(1, sizeof(char) * cur->len);

      // assign string values to token.
      j = 0;
      for (i = 1; *(p + i) != '"'; i++)
      {

        // if can't find closed double quote,raise error.
        if (*(p + i) == '\n' || *(p + i) == '\0')
          error_at(p, "can't find closed double quote.");

        // escape char
        if (*(p + i) == '\\')
        {
          i++;
          switch (*(p + i))
          {
          case 'a':
            cur->str[j] = '\a';
            break;
          case 'b':
            cur->str[j] = '\b';
            break;            
          case 't':
            cur->str[j] = '\t';
            break;            
          case 'n':
            cur->str[j] = '\n';
            break;            
          case 'v':
            cur->str[j] = '\v';
            break;            
          case 'f':
            cur->str[j] = '\f';
            break;            
          case 'r':
            cur->str[j] = '\r';
            break;            
          case 'e': // [GNU] \e for the ASCII escape character is a GNU C extension.
            cur->str[j] = 27;
            break;            
          default:
            cur->str[j] = *(p + i);
            break;
          }
        }
        else
        {
          cur->str[j] = *(p + i);
        }
        j++;
      }

      cur->str[j] = '\0';

      p += i + 1;
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