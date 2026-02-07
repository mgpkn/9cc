```
parse ::= global*
global ::= 
  (type_def|declaration_global_var|declaration_function)
declaration_function ::= 
    base_type declarator "(" (declaration_param("," declaration_param)*)? ")" "{" statment* "}"
declaration_global_var ::=
    base_type declarator("," declarator)* ";"
declaration_local ::=
    base_type (declarator_struct|declarator_union)? declarator ("=" assign )? ("," declarator("=" assign )? )* ";"
type_def ::=
    base_type? (declarator_struct|declarator_union)? (declarator ("," declarator )*)?
declaration_param ::= base_type (declarator_struct|declarator_union)? declarator
base_type ::= ("void"|"char"|"short"|"int"|"long"|"struct"|"union")+
declarator ::= declarator_prefix ("(" declarator ")"|ident) declarator_suffix
declarator_struct ::=  ident? "{" ( base_type declarator ";")* "}"
declarator_union  ::=  ident? "{" ( base_type declarator ";")* "}"
declarator_prefix := ("*" declarator_prefix)? 
declarator_suffix ::= ("[" num "]" declarator_suffix)?
statement ::=
    |";"
    |"{" statement? "}"
    |"return " expr ";"
    |"if" "(" expr ")" statement "else" statement
    |"while" "(" expr ")"  statement
    |"for" "(" (declaration_local|expr)? ";" expr? ";" expr? ";" ")"  statement
    |type_def
    |declaration_local
    |expr ";"
expr ::= assgin ("," expr )?
assign ::= equality ("=" assign )?
equality ::= relational ("==" relational |"!=" relational )*
relational ::= add (">" add | "<" add | ">=" add | "<=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary|"/" unary|"%" unary)*
unary ::= 
    ("+"|"-"|"&"|"*"|"sizeof") unary
    | "sizeof" "(" typename ")"
    |postfix
postfix ::= primary ("[" & expr & "]" | "." ident | "->" ident )*
primary ::= "(" "{" statement+ "}" ")"
    |"(" expr ")"
    |ident ("(" assign? ("," assign)* ")")?
    |num|char|str
```