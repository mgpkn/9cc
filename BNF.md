```
parse ::= global*
global ::= base_type (declaration_global_var|declaration_function)
declaration_function ::= declarator "(" (declaration_local("," declaration_local)?)? ")" "{" statment* "}"
declaration_global_var ::= declarator("," declarator)* ";"
declaration_local ::= base_type declarator_struct? (declarator ("=" expr )? ("," declarator("=" expr )? )*)? ";"
base_type ::= "int"|"char"|"struct"
declarator ::= declarator_prefix ident declarator_suffix
declarator_struct ::= "struct" ident "{" ( base_type declarator ";")* "}"
declarator_prefix := ("*" declarator_prefix)? 
declarator_suffix ::= ("[" num "]" declarator_suffix)?
statement ::=
    |";"
    |"{" statement? "}"
    | "return" expr ";"
    | "if" "(" expr ")" statement "else" statement
    | "while" "(" expr ")"  statement
    | "for" "(" (declaration_local|expr)? ";" expr? ";" expr? ";" ")"  statement
    |declaration_local    
    |expr ";"
declaration_local ::= base_type declarator ("=" expr )? ("," declarator("=" expr )? )*
expr ::= assgin ("," expr )?
assign ::= equality ("=" assign )?
equality ::= relational ("==" relational |"!=" relational )*
relational ::= add (">" add | "<" add | ">=" add | "<=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary|"/" unary|"%" unary)*
unary ::= ("+"|"-"|"&"|"*"|"sizeof") unary
    |postfix
postfix ::= primary ("[" & expr & "]")*	
primary ::= "(" "{" statement+ "}" ")"
    |"(" expr ")"
    |ident ("(" assign? ("," assign)* ")")?
    |num|char|str
```