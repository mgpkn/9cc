
現在のタイプの正規化のルールは次の通り。
1. 比較するノード間で一方がポインタtypeであった場合、prt_toを含めそのtypeを採用。
2. 8byteのtypeのノードが存在した場合、そちらを採用。（現実装ではlongのみ）
3. それ以外は4byte、つまりはintを採用。

以下のようなexpressionがあった場合を例にする。
```c
sizeof("abc"[2]);
```
castによるtype正規化が無かった場合、このようなノードツリーとなる。
```
DERF(TY_CHAR)
    - ND_ADD(TY_PTR -> TY_CHAR)
        - ND_STR(TY_ARRAY -> TY_CHAR)
        - ND_MUL(TY_LONG)
            - ND_NUM(TY_INT) 
            - ND_NUM(TY_LONG)                 
```

一方castを考慮した場合、次のように構成される。
```
DERF(TY_CHAR)
    - ND_ADD(TY_PTR -> TY_CHAR)
        - ND_CAST(TY_PTR -> TY_CHAR)
            - ND_STR(TY_ARRAY -> TY_CHAR)
        - ND_CAST(TY_PTR -> TY_CHAR)            
            - ND_MUL(TY_LONG)
                - ND_CAST(TY_LONG)
                    - ND_NUM(TY_INT) 
                - ND_CAST(TY_LONG)
                    - ND_NUM(TY_LONG)                 
```

