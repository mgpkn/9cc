```
parse = function?
function = ident "(" (ident("," ident)?)? ")" "{" statment? "}"
statement = expr? ";"
		|"{" statement "}"
		| "return " expr ";"
		| "if" "(" expr ")" statement "else" statement
		| "while" "(" expr ")"  statement
		| "for" "(" expr? ";" expr? ";" expr? ";" ")"  statement
expr = assgin
assign = equality ("=" assign )?
equality = relational ("==" relational |"!=" relational )*
relational = add (">" add | "<" add | ">=" add | "<=" add)*
add = mul ("+" mul | "-" mul)*
mul = unary ("*" unary|"/" unary|"%" unary)*
unary = ("+"|"-"|)? primary
		|("&"|"*") unary
primary = "(" expr ")"
		|ident("(" (expr("," expr)?)? ")")?
		|num
```