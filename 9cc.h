#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NODENUM  100
#define OFFSETSIZE 8
#define FUNC_PRAM_NUM 6

//CallError
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

//Token

// トークンの種類
typedef enum {
  TK_KEYWORD,//特別な構文全般
  TK_IDENT, //変数などの識別子
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

Token *tokenize(char *p);

//Node
// 抽象構文木のノードの種類
typedef enum {

  //キーワード
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_MOD, // %

  ND_EQ, // ==
  ND_NOTEQ, // !=
  ND_LLESS,// <
  ND_LLESSEQ,// <=
  ND_ASSIGN,//=
  ND_RETURN,  //return  
  ND_IF,  //if
  ND_WHILE,  //while
  ND_FOR,  //for
  ND_BLOCK,//{}
  
  ////pointer
  ND_POINTER, //&（ポインタ）
  ND_DERFER, //*（デリィファレンサ）

  //変数
  ND_LVAL,//ローカル

  //関数
  ND_FUNC,//関数  

  //実値
  ND_NUM // 整数

  
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
  char *ident_name; //識別子名
  int val;       // kindがND_NUMの場合のみ使う
  int offset; //kindがND_VAL系の場合のみ扱う
  Node *init; //初期化(for)
  Node *cond; //条件(if,for,while)
  Node *inc; //後処理(for)
  Node *then; //cond==Trueの制御
  Node *els;  //cond==Falseの制御
  Node *block_head;//入れ子となっている{}内のコード（先頭）
  Node *func_param[FUNC_PRAM_NUM];
  int label_num;//ラベル
  Node *next;//次のstatement
};


//Create Node(=parse) 
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

void parse(Token *token);

//GenerateCode
void generate_assemble_header();
void generate_assemble_footer();
void generate_assemble_statement(Node* current_node);
void generate_assemble_statement_lval(Node* current_node);

//Analyze
char *getTokenKind(int kind);
char *getNodeKind(int kind);
void lineToken(Token *tar);
