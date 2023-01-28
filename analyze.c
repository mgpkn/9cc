#include "9cc.h"

char *getTokenKind(int kind){
  switch(kind){
  case 0:
    return "TK_KEYWORD";
  case 1:
    return "TK_IDENT";
  case 2:
    return "TK_NUM";
  case 3:
    return "TK_EOF";
  }
}

char *getNodeKind(int kind){

  switch(kind){
  case 0:
    return "ND_ADD";
  case 1:      
    return "ND_SUB";
  case 2:    
    return "ND_MUL";
  case 3:    
    return "ND_DIV";
  case 4:    
    return "ND_MOD";
  case 5:
    return "ND_EQ";
  case 6:
    return "ND_NOTEQ";
  case 7:    
    return "ND_LLESS";
  case 8:    
    return "ND_LLESSEQ";
  case 9:
    return "ND_ASSIGN";
  case 10:
    return "ND_RETURN";
  case 11:  
    return "ND_IF";
  case 12:    
    return "ND_WHILE";
  case 13:    
    return "ND_FOR";
  case 14:
    return "ND_BLOCK";
  case 15:    
    return "ND_POINTER";
  case 16:
    return "ND_DERFER";
  case 17:  
    return "ND_LVAL";
  case 18:
    return "ND_NUM";    
  }

}

void lineToken(Token *tar){

  while(tar->kind != TK_EOF ){

    if(tar->kind == TK_NUM)
      fprintf(stderr,"kind:%s len:%d val:%d\n",getTokenKind(tar->kind),tar->len,tar->val);
    else
      fprintf(stderr,"kind:%s len:%d str:%s\n",getTokenKind(tar->kind),tar->len,tar->str); 
    tar=tar->next;

  }


}
