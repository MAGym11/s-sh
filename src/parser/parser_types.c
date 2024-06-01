typedef enum {
    mem, addr, null, undefined, custom
} Type_Type;

typedef struct {
    Type_Type type_type;
    String value;
} Type;

typedef enum {
    func_name, param_name, var, type, label, jmp, gnrc_name, type_name, enum_value, struct_value, lib, modu
} Id_Type;

typedef struct {
    Id_Type id_type;
    String name;
} Id;

typedef enum {
    cmp_stmt, loop, ret, jump, defer, expr, import, module_access, nop
} Stmt_Type;

typedef struct {
    Stmt_Type stmt_type;
    Array stmts;
    Array exprs;
    Id id;
} Stmt;

typedef struct {
    Array values;
    bool with_escape;
    Array stmts;
} Case;

typedef enum {
    none,
    assgn, infer_type_assgn, label_expr, imply, imply_else, match,
    equals, plusEquals, minusEquals, multEquals, divEquals, modEquals, lShiftEquals, rShiftEquals, andEquals, orEquals, xorEquals,
    ternary,
    orCmp,
    andCmp,
    bOr,
    bXor,
    bAnd,
    cmpEqual, notEquals,
    bLShift, bRShift,
    lThan, gThan, lThanEqual, gThanEqual,
    add, sub,
    mod, mult, divi,
    not, bNot, address, negate, preInc, preDec, postInc, postDec,
    dot_op, double_colon_op,
    cast,
    paren, deref, structure, structure_n_type, func_call, func_call_gnrc, id, in, re, str, cha, undrscr,
    type_gnrc, type_n_gnrc
} Expr_Type;

typedef struct {
    Expr_Type expr_type;
    Type type;
    bool mut;
    Array exprs;
    union {
        Id id_value;
        String str_value;
        uint64_t int_value;
        double real_value;
    };
    Array stmts;
    Array cases;
    Array structure_type;
    Array gnrc;
} Expr;

typedef struct {
    Id id;
    bool is_reference;
    bool is_mutable;
    Expr type_expr;
} Param;

typedef struct {
    Id id;
    bool has_generic;
    Array generic;
    Array params;
    Expr ret_type;
    Stmt stmt;
} Func;

typedef enum {
    stru, enu
} Cust_Type_Type;

typedef struct {
    Id id;
    Cust_Type_Type cust_type_type;
    bool has_generic;
    Array generic;
    Array vals;
} Cust_Type;

typedef struct {
    Id cust_type_name;
    bool has_generic;
    Array generic;
    Array methods;
} Imple;

typedef struct {
    Id id;
    bool has_generic;
    Array generic;
    bool has_self_param;
    Param self_param;
    Array params;
    Expr ret_type;
    Stmt stmt;
} Method;

void Array_increase_capacity(Array* array, size_t size) {
    if (array->capacity == 0) array->capacity++;
    array->capacity *= 2;
    void* tempPtr = malloc(array->capacity*size);
    memcpy(tempPtr, array->ptr, array->length*size);
    if (array->ptr != 0)
        free(array->ptr);
    array->ptr = tempPtr;
}

void Funcs_push(Array* array, Func value) {
    if (array->length == array->capacity)
        Array_increase_capacity(array, sizeof(Func));
    ((Func*)(array->ptr))[array->length] = value;
    array->length++;
}

void Methods_push(Array* array, Method value) {
    if (array->length == array->capacity)
        Array_increase_capacity(array, sizeof(Method));
    ((Method*)(array->ptr))[array->length] = value;
    array->length++;
}

void Cust_Types_push(Array* array, Cust_Type value) {
    if (array->length == array->capacity)
        Array_increase_capacity(array, sizeof(Cust_Type));
    ((Cust_Type*)(array->ptr))[array->length] = value;
    array->length++;
}

void Imples_push(Array* array, Imple value) {
    if (array->length == array->capacity)
        Array_increase_capacity(array, sizeof(Imple));
    ((Imple*)(array->ptr))[array->length] = value;
    array->length++;
}

void Params_push(Array* array, Param value) {
    if (array->length == array->capacity)
        Array_increase_capacity(array, sizeof(Param));
    ((Param*)(array->ptr))[array->length] = value;
    array->length++;
}

void Exprs_push(Array* array, Expr value) {
    if (array->length == array->capacity)
        Array_increase_capacity(array, sizeof(Expr));
    ((Expr*)(array->ptr))[array->length] = value;
    array->length++;
}

void Stmts_push(Array* array, Stmt value) {
    if (array->length >= array->capacity)
        Array_increase_capacity(array, sizeof(Stmt));
    ((Stmt*)(array->ptr))[array->length] = value;
    array->length++;
}

void Ids_push(Array* array, Id value) {
    if (array->length >= array->capacity)
        Array_increase_capacity(array, sizeof(Id));
    ((Id*)(array->ptr))[array->length] = value;
    array->length++;
}

void Cases_push(Array* array, Case value) {
    if (array->length >= array->capacity)
        Array_increase_capacity(array, sizeof(Case));
    ((Case*)(array->ptr))[array->length] = value;
    array->length++;
}

void uint64_t_push(Array* array, uint64_t value) {
    if (array->length >= array->capacity)
        Array_increase_capacity(array, sizeof(uint64_t));
    ((uint64_t*)(array->ptr))[array->length] = value;
    array->length++;
}
