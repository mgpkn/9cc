```
parse ::= global*
global ::= core_type (declaration_global_var|declaration_function)
declaration_global_var ::= declarator("," declarator)* ";"
declaration_function ::= declarator "(" (declaration_local("," declaration_local)?)? ")" "{" statment* "}"
declarator ::= declarator_prefix ident declarator_suffix
declarator_prefix := ("*" declarator_prefix)? 
declarator_suffix ::= ("[" num "]" declarator_suffix)?
statement ::= (declaration_local|expr)? ";"
		|"{" statement? "}"
		| "return " expr ";"
		| "if" "(" expr ")" statement "else" statement
		| "while" "(" expr ")"  statement
		| "for" "(" (declaration_local|expr)? ";" expr? ";" expr? ";" ")"  statement
declaration_local ::= core_type declarator ("," declarator)*
expr ::= assgin
assign ::= equality ("=" assign )?
equality ::= relational ("==" relational |"!=" relational )*
relational ::= add (">" add | "<" add | ">=" add | "<=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary|"/" unary|"%" unary)*
unary ::= ("+"|"-"|"&"|"*"|"sizeof") unary
    |postfix
postfix ::= primary ("[" & expr & "]")*	
primary ::= "(" expr ")"
    |ident("(" (expr("," expr)?)? ")")?
    |num|char|str
```