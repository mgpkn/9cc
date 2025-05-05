#include "9cc.h"

static char *argreg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
// static char *argreg2[] = {"di" , "si", "dx", "cx", "r8w", "r9w"};
static char *argreg4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argreg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int stack_depth;

void gennode_expr(Node *cur_node);
void gennode_stmt(Node *cur_node);

void push()
{
  printf("  push rax\n");
  stack_depth++;
}
void pop(char *reg)
{
  printf("  pop %s\n", reg);
  stack_depth--;
}

void load_val(Type *ty)
{
  if (ty->kind == TY_ARRAY)
    return;

  switch (ty->size)
  {
  case 1:
    printf("  movsx rax, BYTE PTR [rax]\n");
    break;
  case 4:
    printf("  movsxd rax, DWORD PTR [rax]\n");
    break;
  case 8:
  default:
    printf("  mov rax,[rax]\n");
    break;
  }
}

void gennode_addr(Node *cur_node)
{
  switch (cur_node->kind)
  {
  case ND_LVAR:
    printf("  lea rax,[rbp - %d]\n", cur_node->offset);
    return;
  case ND_STR:
  case ND_GVAR:
    printf("  lea rax,%s[rip]\n", cur_node->ident_name);
    return;
  case ND_DEREF:
    gennode_expr(cur_node->lhs);
    return;
  case ND_MEMBER:
    gennode_addr(cur_node->lhs);
    printf("  add rax, %d\n", cur_node->offset);
    return;
  default:
    break;
  }

  error("代入の左辺値が変数ではありません。");
}

void gennode_expr(Node *cur_node)
{

  int argn = 0; // for ND_FUNC

  switch (cur_node->kind)
  {
  case ND_NUM:
  case ND_CHAR:
    printf("  mov rax,%d\n", cur_node->val);
    return;
  case ND_FUNC:
    for (argn = 0; cur_node->func_arg[argn]; argn++)
    {
      gennode_expr(cur_node->func_arg[argn]);
      push();
    }
    for (argn--; argn >= 0; argn--)
      pop(argreg8[argn]);

    printf("  call %s\n", cur_node->ident_name);
    return;
  case ND_MEMBER:
  case ND_LVAR:
  case ND_GVAR:
  case ND_STR:
    gennode_addr(cur_node);
    load_val(cur_node->ty);
    return;
  case ND_ADDR:
    gennode_addr(cur_node->lhs);
    return;
  case ND_DEREF:
    gennode_expr(cur_node->lhs);
    load_val(cur_node->ty);
    return;
  case ND_ASSIGN:
    gennode_expr(cur_node->rhs);
    push();
    gennode_addr(cur_node->lhs);
    pop("rdi");
    switch (cur_node->ty->size)
    {
    case 1:
      printf("  mov [rax],dil\n");
      break;
    case 4:
      printf("  mov [rax],edi\n");
      break;
    case 8:
    default:
      printf("  mov [rax],rdi\n");
      break;
    }
    return;
  case ND_COMMA:
    gennode_expr(cur_node->lhs);
    gennode_expr(cur_node->rhs);
    return;
  default:
    break;
  }

  // compare left-right node
  gennode_stmt(cur_node->rhs);
  push();
  gennode_stmt(cur_node->lhs);
  pop("rdi");

  switch (cur_node->kind)
  {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_MOD:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    printf("  mov rax,rdx\n");
    break;
  case ND_EQ:
    printf("  cmp rax,rdi\n");
    printf("  sete al\n");
    printf("  movzb rax,al\n");
    break;
  case ND_NOTEQ:
    printf("  cmp rax,rdi\n");
    printf("  setne al\n");
    printf("  movzb rax,al\n");
    break;
  case ND_LLESS:
    printf("  cmp rax,rdi\n");
    printf("  setl al\n");
    printf("  movzb rax,al\n");
    break;
  case ND_LLESSEQ:
    printf("  cmp rax,rdi\n");
    printf("  setle al\n");
    printf("  movzb rax,al\n");
    break;
  default:
    break;
  }
}

void gennode_stmt(Node *cur_node)
{

  Node *n;
  if (!cur_node)
    return;

  switch (cur_node->kind)
  {
  case ND_RETURN:
    gennode_stmt(cur_node->lhs);
    printf("  mov rsp, rbp\n");
    pop("rbp");
    printf("  ret\n");
    return;
  case ND_IF:
    gennode_stmt(cur_node->cond);
    printf("  cmp rax,0\n");
    printf("  je .Lelse%d\n", cur_node->label_num);
    gennode_stmt(cur_node->then);
    printf("  jmp .Lend%d\n", cur_node->label_num);
    printf(".Lelse%d:\n", cur_node->label_num);
    if (cur_node->els)
      gennode_stmt(cur_node->els);
    printf(".Lend%d:\n", cur_node->label_num);
    return;
  case ND_WHILE:
    printf(".Lbegin%d:\n", cur_node->label_num);
    gennode_stmt(cur_node->cond);
    printf("  cmp rax,0\n");
    printf("  je .Lend%d\n", cur_node->label_num);
    gennode_stmt(cur_node->then);
    printf("  jmp .Lbegin%d\n", cur_node->label_num);
    printf(".Lend%d:\n", cur_node->label_num);
    return;
  case ND_FOR:
    gennode_stmt(cur_node->init);
    printf(".Lbegin%d:\n", cur_node->label_num);
    gennode_stmt(cur_node->cond);
    printf("  cmp rax,0\n");
    printf("  je .Lend%d\n", cur_node->label_num);
    gennode_stmt(cur_node->then);
    gennode_stmt(cur_node->inc);
    printf("  jmp .Lbegin%d\n", cur_node->label_num);
    printf(".Lend%d:\n", cur_node->label_num);
    return;
  case ND_BLOCK:
    n = cur_node->block_head;
    while (n)
    {
      gennode_stmt(n);
      n = n->next;
    }
  default:
    break;
  }

  gennode_expr(cur_node);
}

void codegen_func(Ident *func)
{

  if (!func->is_function)
    return;

  Node *cur_code, *cur_arg;
  int total_offset;

  // 関数名のラベルを作成
  printf("%s:\n", func->name);

  // 変数領域の確保。
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  total_offset = 0;
  if (func->locals)
    total_offset = func->locals->offset;

  int i = 0;
  while (true)
  {
    if (total_offset <= i * BASE_ALIGN_SIZE)
      break;
    i++;
  }

  printf("  sub rsp, %d\n", i * BASE_ALIGN_SIZE);

  // 引数をレジスタからロード。
  cur_arg = func->arg;
  for (int i = 0; cur_arg; i++)
  {
    gennode_addr(cur_arg);

    switch (cur_arg->ty->size)
    {
    case 1:
      printf("  mov [rax],%s\n", argreg1[i]);
      break;
    case 4:
      printf("  mov [rax],%s\n", argreg4[i]);
      break;
    case 8:
    default:
      printf("  mov [rax],%s\n", argreg8[i]);
      break;
    }
    cur_arg = cur_arg->next;
  }

  // 関数の本文を記述。
  cur_code = func->body;
  for (Node *cur_code = func->body; cur_code; cur_code = cur_code->next)
    gennode_stmt(cur_code);
}

void codegen(Ident *prog_list)
{

  Ident *cur_prog = prog_list;

  // プロローグ
  printf(".intel_syntax noprefix\n");

  // define global variable
  printf(".section .data\n");
  cur_prog = prog_list;
  while (cur_prog)
  {
    if ((cur_prog->is_function))
    {
      cur_prog = cur_prog->next;
      continue;
    }
    printf("%s:\n", cur_prog->name);
    if (cur_prog->str)
    {
      for (int i = 0; i < cur_prog->ty->array_size; i++)
        printf("  .byte %d\n", cur_prog->str[i]);
    }
    else
      printf("  .zero %d\n", cur_prog->ty->size);

    cur_prog = cur_prog->next;
  }
  printf("\n");

  // gen globals
  printf(".section .text\n");
  printf("  .global ");
  cur_prog = prog_list;
  bool is_first_function = true;

  while (cur_prog)
  {
    if (!(cur_prog->is_function))
    {
      cur_prog = cur_prog->next;
      continue;
    }

    if (cur_prog->is_function && !is_first_function)
      printf(",");

    printf("%s", cur_prog->name);
    cur_prog = cur_prog->next;
    is_first_function = false;
  }
  puts("");

  // gen functions
  cur_prog = prog_list;
  while (cur_prog)
  {
    codegen_func(cur_prog);
    cur_prog = cur_prog->next;
  }

  return;
}
