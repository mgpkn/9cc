```
parse ::= (function|global_var)
function ::= declaration
global_var ::= declaration
statement ::= (declaration|expr)? ";"
		|"{" statement? "}"
		| "return " expr ";"
		| "if" "(" expr ")" statement "else" statement
		| "while" "(" expr ")"  statement
		| "for" "(" (declaration|expr)? ";" expr? ";" expr? ";" ")"  statement
expr ::= assgin
declaration ::= type ("*"*|"&"?)ident( declaration_suffix_array | declaration_suffix_function)
declaration_suffix_array ::=  ("[" num "]")*
declaration_suffix_function ::= "(" (declaration("," declaration)?)? ")" "{" statment? "}"
assign ::= equality ("=" assign )?
equality ::= relational ("==" relational |"!=" relational )*
relational ::= add (">" add | "<" add | ">=" add | "<=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary|"/" unary|"%" unary)*
unary ::= ("+"|"-"|"&"|"*"|"sizeof") unary
    |postfix
postfix ::= primary ("[" & expr & "]")*	
primary ::= "(" expr ")"
		|type? ident("(" (expr("," expr)?)? ")")? 
		|num
```