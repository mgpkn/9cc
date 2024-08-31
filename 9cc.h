#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_OFFSETSIZE 8
#define BASE_ALIGNMENTSIZE 16
#define FUNC_ARG_NUM 6

//CallError
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

// トークンの種類
typedef enum {
  TK_KEYWORD, //any syntax word(if,while etc..)
  TK_IDENT,  //ident,function name
  TK_NUM,      //digit value 
  TK_CHAR,      //char    
  TK_STR,      //string  
  TK_EOF
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val; //numeric value 
  char *str; //string value
  int len; //token length
  char *pos; //token word position(string) 
};

Token *tokenize(char *p);

//Node
typedef enum {
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
  ND_WHILE, //while
  ND_FOR,  //for
  ND_BLOCK,//{}
  ND_ADDR, //&（address）
  ND_DEREF, //*（dereferencer）
  ND_LVAR,//local var
  ND_GVAR,//global var
  ND_FUNC,//function  
  ND_NUM, //num value 
  ND_STR, //string value   
  ND_CHAR //char value     
} NodeKind;


//データ型
typedef struct Type Type;
struct Type {
  int kind;
  Type *ptr_to;
  size_t array_size;
  Token *core_ident_tok;
};

//データ型定義
enum TypeKind{
  TY_PTR,
  TY_ARRAY,
  TY_INT,
  TY_CHAR
};


// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  Type *ty; //ノードの論理的なデータ型
  char *ident_name; //識別子名
  int val;       // kindがND_NUMの場合のみ使う
  int offset; //kindがND_VAL系の場合のみ扱う
  Node *init; //初期化(for)
  Node *cond; //条件(if,for,while)
  Node *inc; //後処理(for)
  Node *then; //cond==Trueの制御
  Node *els;  //cond==Falseの制御
  Node *block_head;//入れ子となっている{}内のコード（先頭）
  Node *func_arg[FUNC_ARG_NUM];
  int label_num;//ラベル
  Node *next;//次のstatement
};

typedef struct Ident Ident;
struct Ident{

  bool is_function;
  bool is_global;
  
  //common
  char *name;
  int name_len;//変数名の長さ  
  Ident *next; //next ident
  Type *ty;//型

  //for variable
  int offset; //RBPからのオフセット

  //for global string.
  char *str;

  //for function
  Node *arg;
  Node *body;
  Ident *locals;   

};


//parse
Ident *parse(Token *token);

//codegen
void codegen(Ident *func_list);

//type
int get_type_size(Type *ty);
int calc_sizeof(Type *ty);
void init_nodetype(Node *n);
bool is_num_node(Node *n);
bool is_ptr_node(Node *n);

extern Type *ty_char;
extern Type *ty_int;

//read_file
char *read_file(char *path);