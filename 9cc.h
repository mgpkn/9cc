#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NODENUM  100
#define OFFSETVAL 8


//CallError
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

//Token

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_RETURN,      //return  
  TK_IDENT, //識別子
  TK_NUM,      // 整数
  TK_EOF,      // コードの終わり
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  int len; //トークンの長さ
  char *str;      // トークン文字列
};


bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str,int len); 
Token *tokenize(char *p);


//Node
// 抽象構文木のノードの種類
typedef enum {
  //四則演算子p
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_MOD, // %
  
  //比較演算子
  ND_EQ, // ==
  ND_NOTEQ, // !=
  ND_LLESS,// <
  ND_LLESSEQ,// <=

  //ポインタ関係
  ND_POINTER, //&（ポインタ）
  ND_DERFER, //*（デリィファレンサ）

  //代入演算子
  ND_ASSIGN,//=

  //変数
  ND_LVAL,//ローカル
  
  //実値
  ND_NUM, // 整数
  
  ND_RETURN//rertun
  
} NodeKind;

//local variable
typedef struct LVar LVar;
struct LVar {
  LVar *next; //次の変数（最後ならnull）
  char *name;//変数名
  int len;       // 変数名の長さ
  int offset; //RBPからのオフセット
};

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
  int offset; //kindがND_VAL系の場合のみ扱う
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

void program();
Node *statement();
Node *expr();
Node *equality();
Node *relational();
Node *add(); 
Node *mul(); 
Node *unary();
Node *primary(); 

//GenerateCode
void generate_assemble_header();
void generate_assemble_footer();
void generate_assemble_statement(Node* current_node);
void generate_assemble_statement_lval(Node* current_node);


