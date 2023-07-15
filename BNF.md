```
parse ::= function?
function ::= declaration "(" (declaration("," declaration)?)? ")" "{" statment? "}"
statement ::= (declaration|expr)? ";"
		|"{" statement? "}"
		| "return " expr ";"
		| "if" "(" expr ")" statement "else" statement
		| "while" "(" expr ")"  statement
		| "for" "(" (declaration|expr)? ";" expr? ";" expr? ";" ")"  statement
expr ::= assgin
declaration ::= type ident ("=" assign )?
assign ::= equality ("=" assign )?
equality ::= relational ("==" relational |"!=" relational )*
relational ::= add (">" add | "<" add | ">=" add | "<=" add)*
add ::= mul ("+" mul | "-" mul)*
mul ::= unary ("*" unary|"/" unary|"%" unary)*
unary ::= ("+"|"-"|)? primary
		|("&"|"*") unary
primary ::= "(" expr ")"
		|type? ident("(" (expr("," expr)?)? ")")? 
		|num
```